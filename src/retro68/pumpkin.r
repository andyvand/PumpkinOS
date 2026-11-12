/*
 * Resources of PumpkinOS for classic Mac OS (Retro68).
 *
 * Rez defines (from GNUMakefile):
 *   PPC              the PowerPC application (a 'cfrg' for the code
 *                    fragment in the data fork)
 *   MEM_PREFERRED    the memory partition, in bytes
 *   MEM_MINIMUM
 *   VERSION_STR      "1.0"
 *
 * The icon family ('ICN#', 'icl8', ... 128) is generated from icon.png by
 * mkicon.py into icon.r.
 */

#include "Multiverse.r"

#ifndef MEM_PREFERRED
#define MEM_PREFERRED (24 * 1024 * 1024)
#endif
#ifndef MEM_MINIMUM
#define MEM_MINIMUM   (16 * 1024 * 1024)
#endif
#ifndef VERSION_STR
#define VERSION_STR "1.0"
#endif

#ifdef PPC
resource 'cfrg' (0) {
	{
		kPowerPCCFragArch, kIsCompleteCFrag, kNoVersionNum, kNoVersionNum,
		kDefaultStackSize, kNoAppSubFolder,
		kApplicationCFrag, kDataForkCFragLocator, kZeroOffset, kCFragGoesToEOF,
		"PumpkinOS"
	}
};
#endif

/* canBackground: the threads (network transfers, sound) keep running
   behind other applications */
resource 'SIZE' (-1) {
	reserved,
	acceptSuspendResumeEvents,
	reserved,
	canBackground,
	doesActivateOnFGSwitch,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreChildDiedEvents,
	is32BitCompatible,
	notHighLevelEventAware,
	onlyLocalHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	reserved,
	reserved,
	reserved,
	MEM_PREFERRED,
	MEM_MINIMUM
};

/* fatal errors at startup: ParamText ^0 */
resource 'ALRT' (128) {
	{60, 60, 190, 440},
	128,
	{
		OK, visible, sound1;
		OK, visible, sound1;
		OK, visible, sound1;
		OK, visible, sound1
	},
	alertPositionMainScreen
};

resource 'DITL' (128) {
	{
		{95, 290, 115, 360}, Button { enabled, "Quit" };
		{10, 70, 85, 360}, StaticText { disabled, "^0" }
	}
};

resource 'ALRT' (129) {
	{60, 60, 210, 440},
	129,
	{
		OK, visible, silent;
		OK, visible, silent;
		OK, visible, silent;
		OK, visible, silent
	},
	alertPositionMainScreen
};

resource 'DITL' (129) {
	{
		{115, 290, 135, 360}, Button { enabled, "OK" };
		{10, 70, 28, 360}, StaticText { disabled, "PumpkinOS " VERSION_STR };
		{32, 70, 66, 360}, StaticText { disabled, "A re-implementation of PalmOS, running Palm applications on the Mac." };
		{70, 70, 106, 360}, StaticText { disabled, "The mouse is the pen. The Palm menu has the Home, Menu and application buttons." }
	}
};

resource 'vers' (1) {
	0x01, 0x00, release, 0x00, verUS,
	VERSION_STR,
	VERSION_STR ", PumpkinOS for classic Mac OS"
};

resource 'vers' (2) {
	0x01, 0x00, release, 0x00, verUS,
	VERSION_STR,
	"PumpkinOS"
};

/* the Finder bundle: the application icon, owner 'PmOS' */
resource 'BNDL' (128) {
	'PmOS',
	0,
	{
		'ICN#', { 0, 128 };
		'FREF', { 0, 128 }
	}
};

resource 'FREF' (128) {
	'APPL',
	0,
	""
};

data 'PmOS' (0, "Owner resource") {
	$"0950756D706B696E4F53"   /* "PumpkinOS" */
};

#include "icon.r"
