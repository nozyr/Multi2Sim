/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef X86_ARCH_TIMING_THREAD_H
#define X86_ARCH_TIMING_THREAD_H

#include <arch/x86/emu/uinst.h>
#include <lib/util/class.h>
#include "arch/x86/timing/MemoryBehaviorLogger.h"
#include "arch/x86/timing/MemoryDrivenPrefetcher.h"

/*HACK - to not add a new class file
 *Add a long latency predictor to thread ctx
 */
struct x86_inst_pattern_t
{
        /* Instruction Type Counters */
	long long compute_type;
	long long memory_type;
	long long cache_miss;
	long long cache_hits;

        /* Pattern history */
        long long uinst_int_count;
        long long uinst_logic_count;
        long long uinst_fp_count;
        long long uinst_mem_count;
        long long uinst_ctrl_count;
        long long uinst_total;

        long long uinst_issue_when;

        long long life_time;
        long long start_cycle;
        long long end_cycle;
};

struct x86_inst_pred_t
{
	char *name;
        /* Array of opcode types */
        long long num_uinst_array[x86_uinst_opcode_count];

#define THESHOLD_PATTERN_ITER_COUNT 3
#define MAX_PATTERN_ITER_COUNT 500
        struct x86_inst_pattern_t pattern_history[MAX_PATTERN_ITER_COUNT];
        int curr_pattern_index;
        long long total_pattern_processed;
        float running_avg_inst_per_pattern;
        int next_pred_mem_inst_distance;

	/* Statistics */
	long long accesses;
	long long hits;

        /* instr pointer */
	unsigned int pc;  /* eip */
        int cycles;

        /* Prediction */  
        int equal_phase; 
        int stable_phase; 
        int alt_phase; 
        int remaining_cycles;  
        long long when_predicted;  
        int confidence;
};



/*
 * Class 'X86Thread'
 */

CLASS_BEGIN(X86Thread, Object)

	/* Core and CPU that it belongs to */
	X86Core *core;
	X86Cpu *cpu;

	/* IDs */
	int id_in_core;
	int id_in_cpu;

	/* Name */
	char *name;

	/* Context currently running in this thread. This is a context present
	 * in the thread's 'mapped' list. */
	X86Context *ctx;
	X86Context *next_ctx;

	/* Double-linked list of mapped contexts */
	X86Context *mapped_list_head;
	X86Context *mapped_list_tail;
	int mapped_list_count;
	int mapped_list_max;

	/* Reorder buffer */
	int rob_count;
	int rob_left_bound;
	int rob_right_bound;
	int rob_head;
	int rob_tail;

	/* Number of uops in private structures */
	int iq_count;
	int lsq_count;
	int reg_file_int_count;
	int reg_file_fp_count;
	int reg_file_xmm_count;

	/* Private structures */
	struct list_t *fetch_queue;
	struct list_t *uop_queue;
	struct linked_list_t *iq;
	struct linked_list_t *lq;
	struct linked_list_t *sq;
	struct linked_list_t *preq;
	struct x86_bpred_t *bpred;  /* branch predictor */
	struct x86_trace_cache_t *trace_cache;  /* trace cache */
	struct x86_reg_file_t *reg_file;  /* physical register file */

        /* Add new struct */
#define MAX_PRED_BUF 100
        struct x86_inst_pred_t ipred[MAX_PRED_BUF]; /* instruction predictor for 500 program counters */
        int ipred_index;
#if 0
        /* This goes in ctx also for scheduling purpose */
        int ll_pred_remaining_cycles; 
        int confidence; 
#endif
	/* Fetch */
	unsigned int fetch_eip, fetch_neip;  /* eip and next eip */
	int fetchq_occ;  /* Number of bytes occupied in the fetch queue */
	int trace_cache_queue_occ;  /* Number of uops occupied in the trace cache queue */
	unsigned int fetch_block;  /* Virtual address of last fetched block */
	unsigned int fetch_address;  /* Physical address of last instruction fetch */
	long long fetch_access;  /* Module access ID of last instruction fetch */
	long long fetch_stall_until;  /* Cycle until which fetching is stalled (inclussive) */

	/* Entries to the memory system */
	struct mod_t *data_mod;  /* Entry for data */
	struct mod_t *inst_mod;  /* Entry for instructions */

        /*yurui add memory behavior logger*/
        struct x86_mem_behavr_logger_t memlogger;

        struct x86_SumDrivenPrefetcher SDPrefetcher;

	/* Cycle in which last micro-instruction committed */
	long long last_commit_cycle;

	/* Statistics */
	long long num_fetched_uinst;
	long long num_dispatched_uinst_array[x86_uinst_opcode_count];
	long long num_issued_uinst_array[x86_uinst_opcode_count];
	long long num_committed_uinst_array[x86_uinst_opcode_count];
	long long num_squashed_uinst;
	long long num_branch_uinst;
	long long num_mispred_branch_uinst;
	
	/* Statistics for structures */
	long long rob_occupancy;
	long long rob_full;
	long long rob_reads;
	long long rob_writes;

	long long iq_occupancy;
	long long iq_full;
	long long iq_reads;
	long long iq_writes;
	long long iq_wakeup_accesses;

	long long lsq_occupancy;
	long long lsq_full;
	long long lsq_reads;
	long long lsq_writes;
	long long lsq_wakeup_accesses;

	long long reg_file_int_occupancy;
	long long reg_file_int_full;
	long long reg_file_int_reads;
	long long reg_file_int_writes;

	long long reg_file_fp_occupancy;
	long long reg_file_fp_full;
	long long reg_file_fp_reads;
	long long reg_file_fp_writes;

	long long reg_file_xmm_occupancy;
	long long reg_file_xmm_full;
	long long reg_file_xmm_reads;
	long long reg_file_xmm_writes;

	long long rat_int_reads;
	long long rat_int_writes;
	long long rat_fp_reads;
	long long rat_fp_writes;
	long long rat_xmm_reads;
	long long rat_xmm_writes;

	long long btb_reads;
	long long btb_writes;

CLASS_END(X86Thread)


void X86ThreadCreate(X86Thread *self, X86Core *core);
void X86ThreadDestroy(X86Thread *self);

void X86ThreadSetName(X86Thread *self, char *name);

int X86ThreadIsPipelineEmpty(X86Thread *self);


#endif

