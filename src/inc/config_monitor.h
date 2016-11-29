#ifndef __CONFIG_MONITOR_H
#define __CONFIG_MONITOR_H

// Need further analysis to find a proper value.
// It strongly depends on the target system.
#define MONITOR_INTERVAL 100 // ms

int start_config_monitor(void);

void stop_config_monitor(void);

#endif
