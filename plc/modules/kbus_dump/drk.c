/*
 * Usage: insmod drk.ko
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <linux/unistd.h>
#include <linux/kprobes.h>
#include <linux/file.h>
#include <linux/spi/kbus.h>
#include <linux/gpio.h>

struct kretprobe kr;
volatile int count = 0;

int pre_handler(struct kretprobe_instance *p, struct pt_regs *regs) {
	unsigned int cmd;
	struct kbus_spi_config* ks;
	struct kbus_cmd* kc;
	struct kbus_data* kd;
	
	u8 bits, mode;
	u32 speed;
	
	u32 tx, rx, tm;

	u32 len, timeout;
	u8 tx1, tx2, rx1, rx2;

	cmd = regs->ARM_r1;

	if (cmd == KBUS_IOC_CONFIG) {

		if (count != 0) {
			printk(KERN_INFO "Completed %d KBUS transfers\n", count);
			count = 0;
		}
		ks = (struct kbus_spi_config*)regs->ARM_r2;
		bits = ks->bits_per_word;
		mode = ks->mode;
		speed = ks->max_speed_hz;
		printk(KERN_INFO "SPI config: bits per word: 0x%02x, mode: 0x%02x, speed: 0x%08x\n", bits, mode, speed);

	} else if (cmd == KBUS_IOC_CMD) {

		if (count != 0) {
			printk(KERN_INFO "Completed %d KBUS transfers\n", count);
			count = 0;
		}
		kc = (struct kbus_cmd*)regs->ARM_r2;
		tx = kc->byte_len_tx;
		rx = kc->byte_len_rx;
		tm = kc->timeout_ms;
		printk(KERN_INFO "KBUS cmd: tx_len: %u, rx_len: %u, timeout: %u\n", tx, rx, tm);
		print_hex_dump_bytes("", DUMP_PREFIX_NONE, kc->tx_buf, tx);
		print_hex_dump_bytes("", DUMP_PREFIX_NONE, kc->rx_buf, rx);

	} else if (cmd == KBUS_IOC_DATA) {

		count++;
		if (count == 1) {
			printk(KERN_INFO "KBUS data transfers...\n");
		}

		if ((count % 500) == 0) { // Too many prints here
			kd = (struct kbus_data*)regs->ARM_r2;
			len = kd->byte_len;
			tx1 = kd->tx_buf[0];
			tx2 = kd->tx_buf[1];
			rx1 = kd->rx_buf[0];
			rx2 = kd->rx_buf[1];
			timeout = kd->timeout_ms;
			printk(KERN_INFO "I/O: len = %u, write = [0x%02x, 0x%02x], read = [0x%02x, 0x%02x], timeout: %d\n", len, tx1, tx2, rx1, rx2, timeout);
		}
	}

	return 0;
}

int post_handler(struct kretprobe_instance *ri, struct pt_regs *regs) {
	int ret_val = regs_return_value(regs); //(int)regs->ARM_r0;
	if (ret_val != 4) { // 4 bytes data write
		printk(KERN_INFO "Return value: %d\n", ret_val);
	}
	return 0;
}




int init_module(void) {

/*
	// I/O Attack from kernel space (NOT WORKING: functions expecting user space data)
	
	long res;
	struct kbus_data kd;
	u8 tx[2] = {0x40, 0x00};
	u8 rx[2] = {0x00, 0x00};
	long (*kbus_data_txrx)(struct kbus_data *);
	kbus_data_txrx = (long(*)(struct kbus_data *))0xc03bc02c;

	kd.tx_buf = tx;
	kd.rx_buf = rx;
	kd.byte_len = 2;
	kd.timeout_ms = 1500;

//	while (gpio_get_value(97) == 0) ; // Wait for other transaction

	gpio_set_value(97, 0); // Pin 97 (GPIO4_IO1) value 0
	if ((res = kbus_data_txrx(&kd)) == 0) {
		printk(KERN_INFO "kbus write ok\n"); // Pin 97 will be reset by isr
	} else {
		gpio_set_value(97, 1); // Reset pin 97
		printk(KERN_INFO "kbus error %ld\n", res);
	}
*/

	
	kr.entry_handler = pre_handler;
	kr.handler = post_handler;
	kr.kp.symbol_name = "kbus_ioctl";
	if (register_kretprobe(&kr) == 0) {
		printk(KERN_INFO "kretprobe registered\n");
	}
	return 0;
}

void cleanup_module(void) {
	if (count != 0) {
		printk(KERN_INFO "Completed %d KBUS transfers\n", count);
	}
	unregister_kretprobe(&kr);
	printk(KERN_INFO "unregistered\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea");
MODULE_DESCRIPTION("kbus dump");
