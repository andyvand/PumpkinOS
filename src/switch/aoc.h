#ifndef PUMPKIN_SWITCH_AOC_H
#define PUMPKIN_SWITCH_AOC_H

#include <stdint.h>

// maximum number of add-on contents handled
#define SWITCH_DLC_MAX 32

// connect to / disconnect from the aoc:u service (0 = ok, -1 = unavailable)
int switch_dlc_init(void);
void switch_dlc_exit(void);

// data IDs of the installed add-on contents of this application (at most
// max), returns how many there are, -1 on error, or -2 when they do not
// belong to application app_id (base ID != app_id + 0x1000: an NRO started
// through another title)
int switch_dlc_list(uint64_t app_id, uint64_t *ids, int max);

// mount the RomFS of add-on content id (from switch_dlc_list) as the
// device name ("name:/"), 0 = ok, -1 = error
int switch_dlc_mount(uint64_t id, const char *name);
void switch_dlc_unmount(const char *name);

#endif
