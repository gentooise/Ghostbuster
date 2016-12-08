#ifndef __DR_MONITOR_H
#define __DR_MONITOR_H

// Need further analysis to find a proper value.
// It strongly depends on the target system.
#define DR_MONITOR_INTERVAL 2000 // ms

int start_dr_monitor(void);

void stop_dr_monitor(void);

#endif
