/****************************************************************************
 * weights_blob.S — V10 权重文件嵌入（建木 QiDiAi）
 *
 * 把 v10_weights.baize 以只读数据段形式链接进固件镜像。
 * 板级 ld.script 将该 .weights_blob 段放在 XIP NOR flash 中
 * （紧随镜像其余部分之后）。运行时 V10 加载器原地解析该 blob，
 * 张量指针直接指向 flash —— 零堆分配、零拷贝。
 *
 * The relative path below is resolved from the CMake build directory
 * (i.e. cmake_out/vendor → ../../apps/examples/jianmu/).
 ****************************************************************************/

	.section .weights_blob,"a"
	.align 4

	.global v10_weights_blob_start
	.type v10_weights_blob_start, %object
v10_weights_blob_start:
	.incbin "../../apps/examples/jianmu/v10_weights.baize"

	.global v10_weights_blob_end
	.type v10_weights_blob_end, %object
v10_weights_blob_end:
	.balign 4
	.size v10_weights_blob_start, . - v10_weights_blob_start