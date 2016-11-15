/*
 * Usage: insmod drk.ko
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <linux/unistd.h>
#include <linux/spi/kbus.h>

void **sys_call_table;
asmlinkage long(*original_call_ioctl)(unsigned int, unsigned int, unsigned long);

asmlinkage long our_sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg) {
	if (fd == 11) { // ioctl to /dev/kbus0
		//int v = (int)(*((char*)arg));
		struct kbus_data* p = (struct kbus_data*)arg;
		u32 len = p->byte_len;
		u8 byte1 = p->tx_buf[0];
		u8 byte2 = p->tx_buf[1];
		u32 timeout = p->timeout_ms;
		printk(KERN_INFO "Write: len = %u, bytes = [0x%02x, 0x%02x], timeout: %d\n", len, byte1, byte2, timeout);
	}
	return original_call_ioctl(fd, cmd, arg);
}


static int __init my_trap_init(void) {
    sys_call_table = (void*)0xc000e944; // cat /proc/kallsyms | grep sys_call_table

    original_call_ioctl = sys_call_table[__NR_ioctl];
    sys_call_table[__NR_ioctl] = our_sys_ioctl;

    return 0;
}

static void __exit my_trap_exit(void) {
    sys_call_table[__NR_ioctl] = original_call_ioctl;
}

module_init(my_trap_init);
module_exit(my_trap_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea");
MODULE_DESCRIPTION("ioctl hook");
