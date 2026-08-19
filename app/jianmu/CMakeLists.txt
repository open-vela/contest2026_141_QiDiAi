# Contest 2026 team 141 - QiDiAi 建木 on-device semantic index
# Builds the demo app with the V10 Pure Source Pool C engine
# (v10_infer.c + v10_api.c). Anti-collapse trained (repulsion+contraction).
if(CONFIG_LVX_USE_DEMO_CONTEST2026_141_JIANMU)
  nuttx_add_application(
    NAME jianmu
    SRCS jianmu_main.c v10_api.c v10_infer.c semantic_index.c
    STACKSIZE 16384)
endif()
