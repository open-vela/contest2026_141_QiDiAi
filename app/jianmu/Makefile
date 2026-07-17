############################################################################
# Contest 2026 team 141 - QiDiAi 建木 on-device semantic index - Makefile
# Builds the demo app together with the real V9v3 C engine
# (v9v3_infer.c + v9v3_api.c). The engine's own main() was renamed to
# v9v3_infer_main() so this app keeps jianmu_main.c as the entry point.
############################################################################

include $(APPDIR)/Make.defs

PROGNAME  = jianmu
PRIORITY  = SCHED_PRIORITY_DEFAULT
STACKSIZE = 16384
MODULE    = $(CONFIG_LVX_USE_DEMO_CONTEST2026_141_JIANMU)

MAINSRC = jianmu_main.c v9v3.c v9v3_infer.c v9v3_api.c semantic_index.c

include $(APPDIR)/Application.mk
