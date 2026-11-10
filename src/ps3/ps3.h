#ifndef PUMPKIN_PS3_H
#define PUMPKIN_PS3_H

// set a boot parameter (width, height, density, hdepth, depth, mode, dia, fullscreen)
int ps3_script_set(const char *name, int value);

// read a boot parameter back (-1 when unknown)
int ps3_script_get(const char *name);

// set the application started instead of the Launcher (-app <name>)
int ps3_script_set_launcher(const char *name);

#endif
