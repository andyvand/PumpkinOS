#ifndef PUMPKIN_VITA_H
#define PUMPKIN_VITA_H

// set a boot parameter (width, height, density, hdepth, depth, mode, dia, fullscreen)
int vita_script_set(const char *name, int value);

// set the application started instead of the Launcher (-app <name>)
int vita_script_set_launcher(const char *name);

#endif
