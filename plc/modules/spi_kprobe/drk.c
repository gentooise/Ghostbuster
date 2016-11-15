/*
 * Usage: insmod drk.ko
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <linux/unistd.h>
#include <linux/kprobes.h>
#include <linux/ptrace.h>
#include <linux/spi/spi.h>
#include <linux/kthread.h>



struct kprobe kp;
unsigned long transfer;
volatile int count = 0;

int pre_handler(struct kprobe *p, struct pt_regs *regs) {
	count++;
	if (count == 1) {
		struct kthread_work* work = (struct kthread_work*)regs->ARM_r0;
		struct spi_master *master = container_of(work, struct spi_master, pump_messages);
		transfer = (unsigned long) master->transfer_one_message;
	}
	return 0;
}

void post_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags) {
	// Nop
}

int fault_handler(struct kprobe *p, struct pt_regs *regs, int trapnr) {
	// Nop
	return 0;
}


int init_module(void) {
	int res;
	kp.pre_handler = pre_handler;
	kp.post_handler = post_handler;
	kp.fault_handler = fault_handler;
	kp.symbol_name = "spi_pump_messages";
	// kp.addr = (kprobe_opcode_t *)0xc0321398;
	if ((res = register_kprobe(&kp)) == 0) {
		printk(KERN_INFO "kprobe registered\n");
	}
	return 0;
}

void cleanup_module(void) {
	unregister_kprobe(&kp);
	printk(KERN_INFO "unregistered; transfer function address: 0x%08lx\n", transfer);
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea");
MODULE_DESCRIPTION("spi kprobe");
