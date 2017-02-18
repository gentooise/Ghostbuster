#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define SCAN_CYCLE_DURATION     	10 // ms
#define SCAN_CYCLES_PER_INTERVAL	5
#define PERF_INTERVAL_DURATION  	(SCAN_CYCLE_DURATION * SCAN_CYCLES_PER_INTERVAL * 1000) // Duration in microseconds (us)
#define PERF_INTERVAL_ACCURACY  	50 // us
#define PERF_MIN_RANGE          	(PERF_INTERVAL_DURATION - PERF_INTERVAL_ACCURACY)
#define PERF_MAX_RANGE          	(PERF_INTERVAL_DURATION + PERF_INTERVAL_ACCURACY)
#define PERF_INTERVALS          	100
#define WARMUP_INTERVALS        	2 // Skip first iterations to avoid cache-related overheads

#define PMCR_VAL_NODIV          	0x00000005 // Reset and Enable Cycle Counter Register. No divider (count each cycle).
#define PMCR_VAL                	0x0000000D // Reset and Enable Cycle Counter Register. Set divider to 1 (count each 64 cycles).

static struct task_struct* task;

static int perf_loop(void* data) {
	unsigned i;
	unsigned count;

	for (i = 0; i < PERF_INTERVALS + WARMUP_INTERVALS; i++) {
		
		// Write Performance Monitor Control Register (PMCR)
		asm volatile("mcr p15, 0, %0, c15, c12, 0" : : "r" (PMCR_VAL_NODIV));
		usleep_range(PERF_MIN_RANGE, PERF_MAX_RANGE);
		// Read Cycle Counter Register value
		asm volatile("mrc p15, 0, %0, c15, c12, 1" : "=r" (count));

		if (i >= WARMUP_INTERVALS) {
			printk(KERN_INFO "Perf: [%2u] CPU cycles = %u\n", i - WARMUP_INTERVALS, count);
		}
		if (kthread_should_stop()) goto reset_exit;
	}

reset_exit:
	asm volatile("mcr p15, 0, %0, c15, c12, 0" : : "r" (0));
	while (!kthread_should_stop()) msleep(20);
	return 0;
}

int __init init_module(void) {
	task = kthread_run(&perf_loop, NULL, "perf_mon");
	if (IS_ERR((void*)task)) {
		printk(KERN_ERR "Unable to create thread: %ld\n", PTR_ERR((void*)task));
		return PTR_ERR((void*)task);
	}
	return 0;
}

void __exit cleanup_module() {
	kthread_stop(task);
}

MODULE_AUTHOR("tu4st");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ghostbuster Performance Monitor");
