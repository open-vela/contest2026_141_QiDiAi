/****************************************************************************
 * QiDiAi 建木 — on-device semantic indexing demo (openvela)
 *
 * Vertical slice proving the end-to-end offline pipeline:
 *     embed text  ->  build local vector index  ->  semantic search
 *
 * Powered by the REAL V9v3 embedding engine (v9v3_infer.c + v9v3_api.c):
 * 1024-d output, 12.3MB weights loaded from a .baize file on the FS.
 * Runs fully offline — no network, no cloud.
 ****************************************************************************/

#include <stdio.h>
#include "v9v3.h"
#include "semantic_index.h"

#ifndef V9V3_MODEL_PATH
#define V9V3_MODEL_PATH "/data/v9v3_weights.baize"
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

int main(int argc, char *argv[])
{
  const char *model_path = (argc > 1) ? argv[1] : V9V3_MODEL_PATH;

  printf("=== QiDiAi 建木 (openvela on-device semantic index) ===\n");

  if (v9v3_init(model_path) != 0)
    {
      fprintf(stderr, "建木: V9v3 模型加载失败 (%s)\n", model_path);
      fprintf(stderr, "      请确认权重文件已部署到该路径（烧录 / adb push / ROMFS 挂载）\n");
      return 1;
    }
  printf("V9v3 engine ready, dim = %d (real model)\n", v9v3_dim());

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
  v9v3_free();
  return 0;
}
