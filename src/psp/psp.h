#ifndef PUMPKIN_PSP_H
#define PUMPKIN_PSP_H

// set a boot parameter (width, height, density, hdepth, depth, mode, dia, fullscreen)
int psp_script_set(const char *name, int value);

// set the application started instead of the Launcher (-app <name>)
int psp_script_set_launcher(const char *name);

#endif
