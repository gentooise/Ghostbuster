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
#include <linux/gpio.h>

struct gpio_desc {
	struct gpio_chip	*chip;
	unsigned long		flags;
/* flag symbols are bit numbers */
#define FLAG_REQUESTED	0
#define FLAG_IS_OUT	1
#define FLAG_EXPORT	2	/* protected by sysfs_lock */
#define FLAG_SYSFS	3	/* exported via /sys/class/gpio/control */
#define FLAG_TRIG_FALL	4	/* trigger on falling edge */
#define FLAG_TRIG_RISE	5	/* trigger on rising edge */
#define FLAG_ACTIVE_LOW	6	/* value has active low */
#define FLAG_OPEN_DRAIN	7	/* Gpio is open drain type */
#define FLAG_OPEN_SOURCE 8	/* Gpio is open source type */
#define FLAG_USED_AS_IRQ 9	/* GPIO is connected to an IRQ */

#define ID_SHIFT	16	/* add new flags before this one */

#define GPIO_FLAGS_MASK		((1 << ID_SHIFT) - 1)
#define GPIO_TRIGGER_MASK	(BIT(FLAG_TRIG_FALL) | BIT(FLAG_TRIG_RISE))

	const char		*label;
};

// Needed to compile.
// Taken from "drivers/gpio/gpio-omap.c" and "include/linux/platform_data/gpio-omap.h".

struct gpio_regs {
	u32 irqenable1;
	u32 irqenable2;
	u32 wake_en;
	u32 ctrl;
	u32 oe;
	u32 leveldetect0;
	u32 leveldetect1;
	u32 risingdetect;
	u32 fallingdetect;
	u32 dataout;
	u32 debounce;
	u32 debounce_en;
};

struct omap_gpio_reg_offs {
	u16 revision;
	u16 direction;
	u16 datain;
	u16 dataout;
	u16 set_dataout;
	u16 clr_dataout;
	u16 irqstatus;
	u16 irqstatus2;
	u16 irqstatus_raw0;
	u16 irqstatus_raw1;
	u16 irqenable;
	u16 irqenable2;
	u16 set_irqenable;
	u16 clr_irqenable;
	u16 debounce;
	u16 debounce_en;
	u16 ctrl;
	u16 wkup_en;
	u16 leveldetect0;
	u16 leveldetect1;
	u16 risingdetect;
	u16 fallingdetect;
	u16 irqctrl;
	u16 edgectrl1;
	u16 edgectrl2;
	u16 pinctrl;

	bool irqenable_inv;
};

struct gpio_bank {
	struct list_head node;
	void __iomem *base;
	int irq;
	u32 non_wakeup_gpios;
	u32 enabled_non_wakeup_gpios;
	struct gpio_regs context;
	u32 saved_datain;
	u32 level_mask;
	u32 toggle_mask;
	raw_spinlock_t lock;
	raw_spinlock_t wa_lock;
	struct gpio_chip chip;
	struct clk *dbck;
	u32 mod_usage;
	u32 irq_usage;
	u32 dbck_enable_mask;
	bool dbck_enabled;
	struct device *dev;
	bool is_mpuio;
	bool dbck_flag;

	bool context_valid;
	int stride;
	u32 width;
	int context_loss_count;
	int power_mode;
	bool workaround_enabled;

	void (*set_dataout)(struct gpio_bank *bank, unsigned gpio, int enable);
	int (*get_context_loss_count)(struct device *dev);

	struct omap_gpio_reg_offs *regs;
};



struct kprobe kp;
unsigned long gpio_base;
unsigned set_reg, clr_reg;
unsigned reg_number = 0;
int value;
volatile int count = 0;

int pre_handler(struct kprobe *p, struct pt_regs *regs) {
	count++;
	if (count == 1) {
		struct gpio_bank* bank = (struct gpio_bank*)regs->ARM_r0;
		unsigned long r1 = regs->ARM_r1;
		unsigned long r2 = regs->ARM_r2;
		gpio_base = (unsigned long)bank->base;
		set_reg = bank->regs->set_dataout;
		clr_reg = bank->regs->clr_dataout;
		reg_number = (unsigned)r1;
		value = (int)r2;
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
	kp.symbol_name = "omap_set_gpio_dataout_reg";
	// kp.addr = (kprobe_opcode_t *)0xc0321398;
	if ((res = register_kprobe(&kp)) == 0) {
		printk(KERN_INFO "kprobe registered\n");
	}
	return 0;
}

void cleanup_module(void) {
	unregister_kprobe(&kp);
	printk(KERN_INFO "unregistered; gpio base: 0x%08lx, SET reg: base+0x%02x, CLR reg: base+0x%02x, reg number: %u, value: %d\n", gpio_base, set_reg, clr_reg, reg_number, value);
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea");
MODULE_DESCRIPTION("gpio kprobe");
