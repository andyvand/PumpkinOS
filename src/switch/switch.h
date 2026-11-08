#ifndef PUMPKIN_SWITCH_H
#define PUMPKIN_SWITCH_H

// set a boot parameter (width, height, density, hdepth, depth, mode, dia, fullscreen)
int switch_script_set(const char *name, int value);

// set the application started instead of the Launcher (-app <name>)
int switch_script_set_launcher(const char *name);

#endif
