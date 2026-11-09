#ifndef PUMPKIN_WII_H
#define PUMPKIN_WII_H

// set a boot parameter (width, height, density, hdepth, depth, mode, dia, fullscreen)
int wii_script_set(const char *name, int value);

// read a boot parameter back (-1 when unknown)
int wii_script_get(const char *name);

// set the application started instead of the Launcher (-app <name>)
int wii_script_set_launcher(const char *name);

#endif
