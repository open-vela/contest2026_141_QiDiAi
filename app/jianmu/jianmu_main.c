/****************************************************************************
 * QiDiAi 建木 — on-device semantic indexing demo (openvela)
 *
 * Vertical slice proving the end-to-end offline pipeline:
 *     embed text  ->  build local vector index  ->  semantic search
 *
 * Powered by the V10 Pure Source Pool embedding engine:
 * 1024-d output, 2.58M params, FP16 weights 4.92MB, libc+libm only.
 * Anti-collapse trained (hyperspherical repulsion + contraction loss).
 * Runs fully offline — no network, no cloud.
 *
 * Usage:
 *   jianmu /path/to/v10_weights.baize   — run with real weights
 *   jianmu --selftest                    — run with random weights (pipeline test)
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "v10.h"
#include "semantic_index.h"

#ifndef V10_MODEL_PATH
#define V10_MODEL_PATH "/data/v10_weights.baize"
#endif

static const char *k_corpus[] =
{
  "上次聊装修的朋友 小王 微信",
  "周五的团队周会纪要 关于 openvela 移植",
  "妈妈生日提醒 下周二 买蛋糕",
  "健身计划 每周三跑步五公里",
  "装修预算表格 水电改造两万元",
  "围棋课报名 周末上午",
};

/* Forward declarations from v10_infer.c (for selftest) */
typedef struct {
    int vocab_size, embed_dim, num_layers, max_seq_len, output_dim, sp_dim;
    float r1, r2, r3;
    const float *w_embed;
    const float *skip_gates;
    const float *l_write_gw[6], *l_write_gb[6];
    const float *l_fast_rw[6], *l_fast_rb[6];
    const float *l_med_rw[6], *l_med_rb[6];
    const float *l_slow_rw[6], *l_slow_rb[6];
    const float *l_fusion_w[6], *l_fusion_b[6];
    const float *bottleneck_w, *bottleneck_b;
    const float *out_proj_w, *out_proj_b;
    const float *out_norm_w, *out_norm_b;
    float *tok_emb, *hidden, *pool_f, *pool_m, *pool_s;
    float *write_vals, *fast_r, *med_r, *slow_r;
    float *concat, *mod, *bottleneck_out, *output;
    float *weights;
    long n_floats;
} v10_model_t;

extern int  model_init(v10_model_t *m, const char *path);
extern void model_free(v10_model_t *m);
extern void model_forward(v10_model_t *m, const char *text, float *out_vec);

static v10_model_t g_selftest_model;

static int selftest_init(void)
{
  /* Allocate random weights to test the pipeline end-to-end.
     This does NOT produce meaningful embeddings — it only verifies
     that the C engine loads, runs, and produces valid (non-NaN) output. */
  memset(&g_selftest_model, 0, sizeof(g_selftest_model));

  long n_floats = 2580168;
  float *w = (float *)malloc(n_floats * sizeof(float));
  if (!w) return -1;

  /* Fill with small random values (seeded for reproducibility) */
  srand(42);
  for (long i = 0; i < n_floats; i++)
    w[i] = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;

  g_selftest_model.weights = w;
  g_selftest_model.n_floats = n_floats;
  g_selftest_model.vocab_size = 4958;
  g_selftest_model.embed_dim = 384;
  g_selftest_model.num_layers = 6;
  g_selftest_model.max_seq_len = 96;
  g_selftest_model.output_dim = 1024;
  g_selftest_model.sp_dim = 48;
  g_selftest_model.r1 = 0.85f;
  g_selftest_model.r2 = 0.95f;
  g_selftest_model.r3 = 0.995f;

  /* Set weight pointers (same layout as model_init in v10_infer.c) */
  int off = 0;
  g_selftest_model.w_embed = w + off; off += 4958 * 384;
  g_selftest_model.skip_gates = w + off; off += 6;
  for (int i = 0; i < 6; i++) {
    g_selftest_model.l_write_gw[i] = w + off; off += 48 * 384;
    g_selftest_model.l_write_gb[i] = w + off; off += 48;
    g_selftest_model.l_fast_rw[i] = w + off; off += 384 * 48;
    g_selftest_model.l_fast_rb[i] = w + off; off += 384;
    g_selftest_model.l_med_rw[i] = w + off; off += 384 * 48;
    g_selftest_model.l_med_rb[i] = w + off; off += 384;
    g_selftest_model.l_slow_rw[i] = w + off; off += 384 * 48;
    g_selftest_model.l_slow_rb[i] = w + off; off += 384;
    g_selftest_model.l_fusion_w[i] = w + off; off += 3 * 1152;
    g_selftest_model.l_fusion_b[i] = w + off; off += 3;
  }
  g_selftest_model.bottleneck_w = w + off; off += 144 * 384;
  g_selftest_model.bottleneck_b = w + off; off += 144;
  g_selftest_model.out_proj_w = w + off; off += 1024 * 144;
  g_selftest_model.out_proj_b = w + off; off += 1024;
  g_selftest_model.out_norm_w = w + off; off += 1024;
  g_selftest_model.out_norm_b = w + off; off += 1024;

  /* Initialize output_norm to identity (weight=1, bias=0) for stability */
  for (int i = 0; i < 1024; i++) {
    ((float *)g_selftest_model.out_norm_w)[i] = 1.0f;
    ((float *)g_selftest_model.out_norm_b)[i] = 0.0f;
  }

  /* Allocate runtime buffers */
  g_selftest_model.tok_emb = (float *)calloc(96 * 384, sizeof(float));
  g_selftest_model.hidden = (float *)calloc(96 * 384, sizeof(float));
  g_selftest_model.pool_f = (float *)calloc(48, sizeof(float));
  g_selftest_model.pool_m = (float *)calloc(48, sizeof(float));
  g_selftest_model.pool_s = (float *)calloc(48, sizeof(float));
  g_selftest_model.write_vals = (float *)calloc(96 * 48, sizeof(float));
  g_selftest_model.fast_r = (float *)calloc(384, sizeof(float));
  g_selftest_model.med_r = (float *)calloc(384, sizeof(float));
  g_selftest_model.slow_r = (float *)calloc(384, sizeof(float));
  g_selftest_model.concat = (float *)calloc(1152, sizeof(float));
  g_selftest_model.mod = (float *)calloc(384, sizeof(float));
  g_selftest_model.bottleneck_out = (float *)calloc(144, sizeof(float));
  g_selftest_model.output = (float *)calloc(1024, sizeof(float));

  if (!g_selftest_model.tok_emb || !g_selftest_model.output) {
    fprintf(stderr, "selftest: buffer alloc failed\n");
    return -1;
  }

  printf("[selftest] Random weights initialized (%ld floats, %.1f MB)\n",
         n_floats, n_floats * 4.0 / 1024 / 1024);
  return 0;
}

/* Selftest embed: calls model_forward directly */
static int selftest_embed(const char *text, float *out, int dim)
{
  if (!text || text[0] == '\0') {
    memset(out, 0, dim * sizeof(float));
    return dim;
  }
  float tmp[1024];
  model_forward(&g_selftest_model, text, tmp);
  int copy = dim < 1024 ? dim : 1024;
  memcpy(out, tmp, copy * sizeof(float));
  return copy;
}

int main(int argc, char *argv[])
{
  int selftest = 0;
  const char *model_path = V10_MODEL_PATH;

  if (argc > 1 && strcmp(argv[1], "--selftest") == 0)
    selftest = 1;
  else if (argc > 1)
    model_path = argv[1];

  printf("=== QiDiAi 建木 (openvela on-device semantic index) ===\n");

  if (selftest)
    {
      if (selftest_init() != 0)
        {
          fprintf(stderr, "建木: selftest init failed\n");
          return 1;
        }

      /* Test pipeline: embed → check output validity */
      const char *test_texts[] = {"测试文本", "hello world", ""};
      int all_ok = 1;
      for (int i = 0; i < 3; i++)
        {
          float emb[1024];
          selftest_embed(test_texts[i], emb, 1024);

          int has_nan = 0;
          float norm = 0;
          for (int j = 0; j < 1024; j++)
            {
              if (isnan(emb[j]) || isinf(emb[j])) has_nan = 1;
              norm += emb[j] * emb[j];
            }
          norm = sqrtf(norm);

          printf("  [%d] \"%s\": norm=%.4f nan=%d %s\n",
                 i, test_texts[i], norm, has_nan,
                 has_nan ? "FAIL" : "OK");
          if (has_nan) all_ok = 0;
        }

      printf("\n=== selftest %s (pipeline OK, weights are random) ===\n",
             all_ok ? "PASSED" : "FAILED");

      free(g_selftest_model.weights);
      free(g_selftest_model.tok_emb); free(g_selftest_model.hidden);
      free(g_selftest_model.pool_f); free(g_selftest_model.pool_m);
      free(g_selftest_model.pool_s); free(g_selftest_model.write_vals);
      free(g_selftest_model.fast_r); free(g_selftest_model.med_r);
      free(g_selftest_model.slow_r); free(g_selftest_model.concat);
      free(g_selftest_model.mod); free(g_selftest_model.bottleneck_out);
      free(g_selftest_model.output);
      return all_ok ? 0 : 1;
    }

  /* Normal mode: load real weights */
  if (v10_init(model_path) != 0)
    {
      fprintf(stderr, "建木: V10 模型加载失败 (%s)\n", model_path);
      fprintf(stderr, "      请确认权重文件已部署到该路径（烧录 / adb push / ROMFS 挂载）\n");
      fprintf(stderr, "      或使用 jianmu --selftest 进行管道自检\n");
      return 1;
    }
  printf("V10 engine ready, dim = %d (real model)\n", v10_dim());

  semantic_index_t idx;
  si_build(&idx, k_corpus, sizeof(k_corpus) / sizeof(k_corpus[0]));
  printf("Indexed %d local documents (fully offline).\n", idx.count);

  const char *queries[] =
  {
    "找上次聊装修的朋友",
    "提醒我妈生日",
    "这周去跑步",
  };

  for (int q = 0; q < 3; q++)
    {
      int   res[SI_TOP_K];
      float sc[SI_TOP_K];
      int   n = si_search(&idx, queries[q], res, sc);

      printf("\nQuery: \"%s\"\n", queries[q]);
      if (n == 0)
        {
          printf("  (no match / model not loaded)\n");
          continue;
        }
      for (int k = 0; k < n; k++)
        {
          printf("  [%d] score=%.3f  %s\n", k + 1, sc[k],
                 idx.docs[res[k]].text);
        }
    }

  printf("\n=== demo complete (offline semantic search OK) ===\n");
  si_free(&idx);
  v10_free();
  return 0;
}
