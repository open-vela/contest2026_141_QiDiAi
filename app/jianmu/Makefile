############################################################################
# Contest 2026 team 141 - QiDiAi 建木 on-device semantic index - Makefile
# Builds the demo app with the V10 Pure Source Pool C engine
# (v10_infer.c + v10_api.c). Anti-collapse trained (repulsion+contraction).
############################################################################

include $(APPDIR)/Make.defs

PROGNAME  = jianmu
PRIORITY  = SCHED_PRIORITY_DEFAULT
STACKSIZE = 16384
MODULE    = $(CONFIG_LVX_USE_DEMO_CONTEST2026_141_JIANMU)

MAINSRC = jianmu_main.c v10_api.c v10_infer.c semantic_index.c

include $(APPDIR)/Application.mk
