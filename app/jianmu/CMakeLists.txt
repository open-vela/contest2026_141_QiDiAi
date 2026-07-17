# Contest 2026 team 141 - QiDiAi 建木 on-device semantic index
# Builds the demo app together with the real V9v3 C engine
# (v9v3_infer.c + v9v3_api.c). The engine's own main() was renamed to
# v9v3_infer_main() in our copy so this app keeps jianmu_main.c as entry.
if(CONFIG_LVX_USE_DEMO_CONTEST2026_141_JIANMU)
  nuttx_add_application(
    NAME jianmu
    SRCS jianmu_main.c v9v3.c v9v3_infer.c v9v3_api.c semantic_index.c
    STACKSIZE 16384)
endif()
