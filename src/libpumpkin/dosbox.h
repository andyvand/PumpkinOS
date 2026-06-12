#define DOSBOX_LIBRETRO VFS_PROGRAMS_DIR "/Libretro"
#define DOSBOX_HOME     DOSBOX_LIBRETRO "/dosbox"

#define DOSBOX_DRIVEC DOSBOX_HOME "/C"
#define DOSBOX_RUN    "run.bat"

#define dosboxLaunchCmd sysAppLaunchCmdCustomBase

UInt32 DOSBoxMain(Boolean sameTask);
