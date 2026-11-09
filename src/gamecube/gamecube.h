#ifndef PUMPKIN_GAMECUBE_H
#define PUMPKIN_GAMECUBE_H

// set a boot parameter (width, height, density, hdepth, depth, mode, dia, fullscreen)
int gc_script_set(const char *name, int value);

// read a boot parameter back (-1 when unknown)
int gc_script_get(const char *name);

// set the application started instead of the Launcher (-app <name>)
int gc_script_set_launcher(const char *name);

#endif
