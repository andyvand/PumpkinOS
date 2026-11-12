// The Retro68 sysroot keeps newlib's <strings.h> as <bsdstrings.h>: on a
// case-insensitive file system (macOS) the name is taken by the Toolbox
// <Strings.h>, which newlib's <string.h> would otherwise pull in (and with
// it the whole Mac interface, whose Boolean, ProcPtr, ... clash with the
// PalmOS types). PumpkinOS uses the Toolbox string functions nowhere.
#include <bsdstrings.h>
