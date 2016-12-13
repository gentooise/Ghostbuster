#ifndef __LOG_H
#define __LOG_H

// Define primitives to use for logging purposes
#define GHOSTBUSTER     	"Ghostbuster: "

#define log_info(s, ...)	printk(KERN_INFO GHOSTBUSTER s, ##__VA_ARGS__)
#define log_err(s, ...) 	printk(KERN_ERR GHOSTBUSTER s, ##__VA_ARGS__)
#define log_cont(s, ...)	printk(KERN_CONT s, ##__VA_ARGS__)

#endif
