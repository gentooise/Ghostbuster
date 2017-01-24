#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>

static struct kprobe kp;

int pre_handler(struct kprobe *p, struct pt_regs *regs) {
	unsigned irq = regs->ARM_r0;
	printk(KERN_INFO "IRQ Line: %u\n", irq);
	return 0;
}

void post_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags) {
	// Nop
}

int fault_handler(struct kprobe *p, struct pt_regs *regs, int trapnr) {
	// Nop
	return 0;
}

int __init init_module(void) {
	int res;

	kp.pre_handler = pre_handler;
	kp.post_handler = post_handler;
	kp.fault_handler = fault_handler;
	kp.symbol_name = "enable_irq";
	if ((res = register_kprobe(&kp)) == 0) {
		printk(KERN_INFO "kprobe registered\n");
	}

	return res;
}

void __exit cleanup_module(void) {
	unregister_kprobe(&kp);
	printk(KERN_INFO "kprobe unregistered\n");
}

MODULE_AUTHOR("tu4st");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ghost in the PLC");
