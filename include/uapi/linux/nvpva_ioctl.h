/*
 * Tegra PVA Driver ioctls
 *
 * Copyright (c) 2021, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program;  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __NVPVA_IOCTL_H__
#define __NVPVA_IOCTL_H__

#include <linux/ioctl.h>
#include <linux/types.h>

#define NVPVA_DEVICE_NODE "/dev/nvhost-ctrl-pva"

struct nvpva_ioctl_part {
	uint64_t addr;
	uint64_t size;
};

/*
 * VPU REGISTER UNREGISTER command details
 */

struct nvpva_vpu_exe_register_in_arg {
	struct nvpva_ioctl_part exe_data;
};

/* IOCTL magic number - seen available in ioctl-number.txt */
struct nvpva_vpu_exe_register_out_arg {
	/* Exe id assigned by KMD for the executable */
	uint16_t exe_id;
	/* Number of symbols */
	uint32_t num_of_symbols;
	/* Total size of symbols in executable */
	uint32_t symbol_size_total;
};

union nvpva_vpu_exe_register_args {
	struct nvpva_vpu_exe_register_in_arg in;
	struct nvpva_vpu_exe_register_out_arg out;
};

struct nvpva_vpu_exe_unregister_in_arg {
	/* Exe id assigned by KMD for the executable */
	uint16_t exe_id;
};

union nvpva_vpu_exe_unregister_args {
	struct nvpva_vpu_exe_unregister_in_arg in;
};

/*
 * VPU SYMBOL command details
 */

struct nvpva_symbol {
	uint16_t id;
	uint32_t size;
};

struct nvpva_get_symbol_in_arg {
	uint16_t exe_id;
	struct nvpva_ioctl_part name; /*size including null*/
};

struct nvpva_get_symbol_out_arg {
	struct nvpva_symbol symbol;
};

union nvpva_get_symbol_args {
	struct nvpva_get_symbol_in_arg in;
	struct nvpva_get_symbol_out_arg out;
};

/*
 * PIN UNPIN command details
 */

enum nvpva_pin_segment {
	NVPVA_SEGMENT_LOWMEM = 1U,
	NVPVA_SEGMENT_HIGHMEM = 2U,
	NVPVA_SEGMENT_CVSRAM = 3U,
};

enum nvpva_pin_buf {
	NVPVA_BUFFER_GEN = 0U,
	NVPVA_BUFFER_SEM = 1U,
};

enum nvpva_pin_access {
	NVPVA_ACCESS_RD = 1U,
	NVPVA_ACCESS_WR = 2U,
	NVPVA_ACCESS_RW = 3U,
};

struct nvpva_pin_handle {
	uint32_t import_id;
	uint64_t offset;
	uint64_t size;
	enum nvpva_pin_access access;
	enum nvpva_pin_segment segment;
	enum nvpva_pin_buf type;
};

struct nvpva_pin_in_arg {
	struct nvpva_pin_handle pin;
};

struct nvpva_pin_out_arg {
	uint32_t pin_id; /* Unique ID assigned by KMD for the Pin */
};

union nvpva_pin_args {
	struct nvpva_pin_in_arg in;
	struct nvpva_pin_out_arg out;
};

struct nvpva_unpin_in_arg {
	uint32_t pin_id;
};

union nvpva_unpin_args {
	struct nvpva_unpin_in_arg in;
};

/*
 * TASK SUBMIT command details
 */

enum nvpva_flags {
	NVPVA_AFFINITY_VPU0 = 1U,
	NVPVA_AFFINITY_VPU1 = 1U << 1U,
	NVPVA_AFFINITY_VPU_ANY = NVPVA_AFFINITY_VPU0 | NVPVA_AFFINITY_VPU1,
	NVPVA_PRE_BARRIER_TASK_TRUE = 1U << 2U,
};

enum nvpva_fence_action_type {
	NVPVA_FENCE_PRE = 1U,
	NVPVA_FENCE_SOT_R5 = 2U,
	NVPVA_FENCE_SOT_VPU = 3U,
	NVPVA_FENCE_EOT_VPU = 4U,
	NVPVA_FENCE_EOT_R5 = 5U,
	NVPVA_FENCE_POST = 6U,
	NVPVA_MAX_FENCE_TYPES = 7U,
};

enum nvpva_fence_obj_type {
	NVPVA_FENCE_OBJ_SYNCPT = 0U,
	NVPVA_FENCE_OBJ_SEM = 1U,
	/* Below types are not being used in QNX KMD for now */
	NVPVA_FENCE_OBJ_SEMAPHORE_TS = 2U,
	NVPVA_FENCE_OBJ_SYNC_FD = 3U,
};

enum nvpva_symbol_config {
	NVPVA_SYMBOL_PARAM = 0U,
	NVPVA_SYMBOL_POINTER = 1U,
};

enum nvpva_hwseq_trigger_mode {
	NVPVA_HWSEQTM_VPUTRIG = 0U,
	NVPVA_HWSEQTM_DMATRIG = 1U,
};

#define NVPVA_MEM_REGISTERED_SIZE (0U)
struct nvpva_mem {
	uint32_t pin_id;
	uint32_t offset;
	/* size=NVPVA_MEM_REGISTERED_SIZE
	 *considered as entire pinned area
	 */
	uint32_t size;
};

struct nvpva_fence_obj_syncpt {
	uint32_t id;
	uint32_t value;
};

struct nvpva_fence_obj_sem {
	struct nvpva_mem mem;
	uint32_t value;
};

struct nvpva_fence_obj_syncfd {
	uint32_t fd;
};

union nvpva_fence_obj {
	struct nvpva_fence_obj_syncpt syncpt;
	struct nvpva_fence_obj_sem sem;
	struct nvpva_fence_obj_syncfd syncfd;
};

struct nvpva_submit_fence {
	enum nvpva_fence_obj_type type;
	union nvpva_fence_obj obj;
};

struct nvpva_fence_action {
	enum nvpva_fence_action_type type;
	/* For syncpt, ID is the per-queue ID allocated by KMD */
	struct nvpva_submit_fence fence;
	/* Buffer to capture event timestamp */
	struct nvpva_mem timestamp_buf;
};

struct nvpva_pointer_symbol {
	/* Base address of pinned area, where
	 * lower 32bits filled with pin_id by UMD and
	 * at KMD will replace it with actual base address.
	 */
	uint64_t base;
	/* Offset in pinned area */
	uint32_t offset;
	/* Size of pinned area, filled by KMD */
	uint32_t size;
};

/* Used to pass both param and pointer type symbols.
 * Based on nvpva_symbol_config selection the data in payload
 * pointed by offset will differ.
 * For NVPVA_SYMBOL_PARAM, payload data is raw data.
 * For NVPVA_SYMBOL_POINTER, data is of type nvpva_pointer_symbol.
 */
struct nvpva_symbol_param {
	enum nvpva_symbol_config config; /* Type of symbol configuration */
	struct nvpva_symbol symbol;	 /* Symbol to be configured */
	uint32_t offset;		 /* Offset of symbol data in payload */
};

/* NOTE: Redefining the user side structure here
 * This is done to allow UMD to pass the descriptor as it is and
 * to handle the (user struct -> hw struct) coversion at KMD side.
 * KMD needs redefinition to avoid circular dependency.
 *
 * An update in user structure would need corresponding change here
 */
struct nvpva_dma_descriptor {
	uint32_t srcPtr;
	uint32_t dstPtr;
	uint32_t dst2Ptr;
	uint32_t src_offset;
	uint32_t dst_offset;
	uint32_t dst2Offset;
	uint32_t surfBLOffset;
	uint16_t tx;
	uint16_t ty;
	uint16_t srcLinePitch;
	uint16_t dstLinePitch;
	int32_t srcAdv1;
	int32_t dstAdv1;
	int32_t srcAdv2;
	int32_t dstAdv2;
	int32_t srcAdv3;
	int32_t dstAdv3;
	uint8_t srcRpt1;
	uint8_t dstRpt1;
	uint8_t srcRpt2;
	uint8_t dstRpt2;
	uint8_t srcRpt3;
	uint8_t dstRpt3;
	uint8_t linkDescId;
	uint8_t px;
	uint32_t py;
	uint8_t srcCbEnable;
	uint8_t dstCbEnable;
	uint32_t srcCbStart;
	uint32_t dstCbStart;
	uint32_t srcCbSize;
	uint32_t dstCbSize;
	uint8_t trigEventMode;
	uint8_t trigVpuEvents;
	uint8_t descReloadEnable;
	uint8_t srcTransferMode;
	uint8_t dstTransferMode;
	uint8_t srcFormat;
	uint8_t dstFormat;
	uint8_t bytePerPixel;
	uint8_t pxDirection;
	uint8_t pyDirection;
	uint8_t boundaryPixelExtension;
	uint8_t transTrueCompletion;
	uint8_t prefetchEnable;
};

/* NOTE: Redefining the user side structure here
 * This is done to allow UMD to pass the channel info as it is and
 * to handle the (user struct -> hw struct) coversion at KMD side.
 * KMD needs redefinition to avoid circular dependency.
 *
 * An update in user structure would need corresponding change here
 */
struct nvpva_dma_channel {
	uint8_t descIndex;
	uint8_t blockHeight;
	uint16_t adbSize;
	uint8_t vdbSize;
	uint16_t adbOffset;
	uint8_t vdbOffset;
	uint32_t outputEnableMask;
	uint32_t padValue;
	uint8_t reqPerGrant;
	uint8_t prefetchEnable;
	uint8_t chRepFactor;
	uint8_t hwseqStart;
	uint8_t hwseqEnd;
	uint8_t hwseqEnable;
	uint8_t hwseqTraversalOrder;
	uint8_t hwseqTxSelect;
	uint8_t hwseqTriggerDone;
};

/**
 * Used to pass config for Hardware Sequencer (HWSeq).
 * For HWSeq operations, all DMA channels will be configured
 * based on the selection of hardware sequencer trigger mode.
 * For NVPVA_HWSEQTM_VPUTRIG, VPU trigger mode will be used.
 * For NVPVA_HWSEQTM_DMATRIG, DMA trigger mode will be used.
 */
struct nvpva_hwseq_config {
	struct nvpva_mem hwseqBuf;
	enum nvpva_hwseq_trigger_mode hwseqTrigMode;
};

struct nvpva_ioctl_task {
	uint16_t exe_id;
	uint32_t flags;
	uint32_t l2_alloc_size; /* Not applicable for Xavier */
	struct nvpva_ioctl_part prefences;
	struct nvpva_ioctl_part user_fence_actions;
	struct nvpva_ioctl_part input_task_status;
	struct nvpva_ioctl_part output_task_status;
	struct nvpva_ioctl_part dma_descriptors;
	struct nvpva_ioctl_part dma_channels;
	struct nvpva_ioctl_part hwseq_config;
	struct nvpva_ioctl_part symbols;
	struct nvpva_ioctl_part symbol_payload;
};

struct nvpva_ioctl_submit_in_arg {
	uint32_t version;
	uint64_t submission_timeout_us;
	uint64_t execution_timeout_us;
	struct nvpva_ioctl_part tasks;
};

struct nvpva_submit_in_arg_s {
	uint32_t version;
	uint16_t num_tasks;
	uint64_t submission_timeout_us;
	uint64_t execution_timeout_us;
};

union nvpva_ioctl_submit_args {
	struct nvpva_ioctl_submit_in_arg in;
};

/* There are 64 DMA descriptors in T19x and T23x. But R5 FW reserves
 * 4 DMA descriptors for internal use.
 */
#define NVPVA_TASK_MAX_DMA_DESCRIPTORS (60U)
/*TODO: Remove NVPVA_TASK_MAX_DMA_CHANNELS */
/*There are 14 DMA channels in T19x and 16 DMA channels in T23X.
 * R5 FW reserves one DMA channel for internal use.
 */
#define NVPVA_TASK_MAX_DMA_CHANNELS 16U
#define NVPVA_TASK_MAX_DMA_CHANNELS_T19X (13U)
#define NVPVA_TASK_MAX_DMA_CHANNELS_T23X (15U)
#define NVPVA_NOOP_EXE_ID 65535
#define NVPVA_SUBMIT_MAX_TASKS 16U

#define NVPVA_IOCTL_MAGIC 'Q'

#define NVPVA_IOCTL_REGISTER_VPU_EXEC \
	_IOWR(NVPVA_IOCTL_MAGIC, 1, union nvpva_vpu_exe_register_args)

#define NVPVA_IOCTL_UNREGISTER_VPU_EXEC \
	_IOW(NVPVA_IOCTL_MAGIC, 2, union nvpva_vpu_exe_unregister_args)

#define NVPVA_IOCTL_GET_SYMBOL_ID \
	_IOWR(NVPVA_IOCTL_MAGIC, 3, union nvpva_get_symbol_args)

#define NVPVA_IOCTL_PIN \
	_IOWR(NVPVA_IOCTL_MAGIC, 4, union nvpva_pin_args)

#define NVPVA_IOCTL_UNPIN \
	_IOW(NVPVA_IOCTL_MAGIC, 5, union nvpva_unpin_args)

#define NVPVA_IOCTL_SUBMIT \
	_IOW(NVPVA_IOCTL_MAGIC, 6, union nvpva_ioctl_submit_args)

#define NVPVA_IOCTL_NOP \
	_IOW(NVPVA_IOCTL_MAGIC, 7)

#define NVPVA_IOCTL_ACQUIRE_QUEUE \
	_IOW(NVPVA_IOCTL_MAGIC, 8)

#define NVPVA_IOCTL_RELEASE_QUEUE \
	_IOW(NVPVA_IOCTL_MAGIC, 9)

#define NVPVA_IOCTL_NUMBER_MAX 9

/* NvPva Task param limits */
#define NVPVA_TASK_MAX_PREFENCES 8U
#define NVPVA_TASK_MAX_FENCEACTIONS 4U
#define NVPVA_TASK_MAX_INPUT_STATUS 8U
#define NVPVA_TASK_MAX_OUTPUT_STATUS 8U
#define NVPVA_TASK_MAX_SYMBOLS 128U
/* VMEM configurable size */
#define NVPVA_TASK_MAX_PAYLOAD_SIZE 8192U
#define NVPVA_TASK_MAX_SIZE                                                  \
	(sizeof(struct nvpva_submit_task_header) +                           \
	NVPVA_TASK_MAX_PREFENCES * sizeof(struct nvpva_submit_fence) +       \
	NVPVA_TASK_MAX_FENCEACTIONS *                                        \
		NVPVA_MAX_FENCE_TYPES * sizeof(struct nvpva_fence_action) +  \
	NVPVA_TASK_MAX_INPUT_STATUS * sizeof(struct nvpva_mem) +             \
	NVPVA_TASK_MAX_OUTPUT_STATUS * sizeof(struct nvpva_mem) +            \
	NVPVA_TASK_MAX_DMA_DESCRIPTORS *                                     \
		sizeof(struct nvpva_dma_descriptor) +                        \
	NVPVA_TASK_MAX_DMA_CHANNELS * sizeof(struct nvpva_dma_channel) +     \
	sizeof(struct nvpva_hwseq_config) +                                  \
	NVPVA_TASK_MAX_SYMBOLS * sizeof(struct nvpva_symbol_param) +         \
	NVPVA_TASK_MAX_PAYLOAD_SIZE)

/* NvPva submit param limits */
#define NVPVA_SUBMIT_MAX_SIZE                                                \
	(NVPVA_SUBMIT_MAX_TASKS * NVPVA_TASK_MAX_SIZE +                      \
	sizeof(struct nvpva_submit_in_arg_s))

#endif /* __NVPVA_IOCTL_H__ */
