/*
 * SPDX-FileCopyrightText: 2023-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <reent.h>
#include <pthread.h>
#include <setjmp.h>
#include <getopt.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/select.h> /* Voor select */
#include <sys/stat.h>   /* Voor stat, fstat, mkdir */
#include <arpa/inet.h>
#include <ctype.h>
#include <esp_dlfcn.h>
#include <netdb.h>
#include <signal.h>
#include <dirent.h>     /* Voor opendir, readdir, closedir */

#include "lwip/sockets.h"
#include "lwip/netdb.h"

#include "rom/ets_sys.h"

#include "esp_elf.h"

#include "../../libpit/sys.h"
#include "../../libpit/filter.h"
#include "../../libpit/telnet.h"
#include "../../libpit/mutex.h"
#include "../../libpit/rgb.h"
#include "../../libpit/util.h"
#include "../../libpit/debug.h"
#include "../../libpit/xalloc.h"
#include "../../libpit/thread.h"
#include "../../libpit/ptr.h"
#include "../../libpit/editor.h"
#include "../../libpit/bytes.h"
#include "../../PalmOS/PalmOS.h"
#include "../../PalmOS/Extensions/ExpansionMgr/VFSMgr.h"
#include "../../PalmOS/Libraries/Pdi/PdiLib.h"
#include "../../PalmOS/Libraries/PalmOSGlue/FntGlue.h"
#include "../../libpumpkin/pumpkin.h"
#include "../../libpumpkin/storage.h"
#include "../../libpumpkin/tos.h"
#include "../../libpumpkin/syntax.h"
#include "../../libpumpkin/color.h"
#include "../../libpumpkin/wav.h"
#include "../../libpumpkin/peditor.h"
#include "../../libpumpkin/edit.h"
#include "../../libpumpkin/plibc.h"
#include "../../libpumpkin/ColorTable.h"

#if CONFIG_ELF_DYNAMIC_LOAD_SHARED_OBJECT
#include "private/esp_dlmod.h"
#endif
#include "private/elf_symbol.h"

extern WChar TxtGlueUpperChar(WChar inChar);

extern int __ltdf2(double a, double b);
extern unsigned int __fixunsdfsi(double a);
extern int __gtdf2(double a, double b);
extern float __modsi3 (float a, float b);
extern float __divsf3 (float a, float b);
extern float __addsf3 (float a, float b);
extern float __subsf3 (float a, float b);
extern float __mulsf3 (float a, float b);
extern float __divsf3 (float a, float b);
extern float __udivsi3 (float a, float b);
extern float __divsi3 (float a, float b);
extern double __moddi3 (double a, double b);
extern double __floatunsidf(unsigned int i);
extern double __divdf3(double a, double b);
extern double __adddf3 (double a, double b);
extern double __subdf3 (double a, double b);
extern double __muldf3 (double a, double b);
extern double __divdf3 (double a, double b);
extern double __udivdi3 (double a, double b);
extern double __divdi3 (double a, double b);
extern double __floatsidf (double a);
extern double __truncdfsf2 (double a);
extern double __extendsfdf2 (double a);
extern double __fixdfsi (double a);
extern double __floatundidf (double a);
extern double __floatdidf (double a);
extern double __umoddi3 (double a, double b);
extern int __ledf2 (double a, double b);
extern int __gedf2 (double a, double b);
extern int __unorddf2 (double a, double b);
extern int __nedf2 (double a, double b);
extern int __eqdf2 (double a, double b);
extern unsigned int __fixunssfdi (float a);

/** @brief Libc public functions symbols look-up table */

static const struct esp_elfsym g_palm_pumpkin_elfsyms[] = {
    /* --- SYSTEEM WRAPPERS (sys_*) --- */
    ESP_ELFSYM_EXPORT(sys_malloc), ESP_ELFSYM_EXPORT(sys_calloc), ESP_ELFSYM_EXPORT(sys_realloc), ESP_ELFSYM_EXPORT(sys_free),
    ESP_ELFSYM_EXPORT(sys_memcpy), ESP_ELFSYM_EXPORT(sys_memset), ESP_ELFSYM_EXPORT(sys_memmove), ESP_ELFSYM_EXPORT(sys_memchr),
    ESP_ELFSYM_EXPORT(sys_memrchr), ESP_ELFSYM_EXPORT(sys_strlen), ESP_ELFSYM_EXPORT(sys_strcpy), ESP_ELFSYM_EXPORT(sys_strdup),
    ESP_ELFSYM_EXPORT(sys_strndup), ESP_ELFSYM_EXPORT(sys_strchr), ESP_ELFSYM_EXPORT(sys_strncmp), ESP_ELFSYM_EXPORT(sys_strcasecmp),
    ESP_ELFSYM_EXPORT(sys_strncasecmp), ESP_ELFSYM_EXPORT(sys_sprintf), ESP_ELFSYM_EXPORT(sys_snprintf), ESP_ELFSYM_EXPORT(sys_vsnprintf),
    ESP_ELFSYM_EXPORT(sys_sscanf), ESP_ELFSYM_EXPORT(sys_lib_defsymbol), ESP_ELFSYM_EXPORT(sys_socket_open_connect),
    ESP_ELFSYM_EXPORT(plibc_init), ESP_ELFSYM_EXPORT(plibc_dup), ESP_ELFSYM_EXPORT(sys_lib_close),
    ESP_ELFSYM_EXPORT(plibc_close), ESP_ELFSYM_EXPORT(plibc_setfd), ESP_ELFSYM_EXPORT(plibc_fgets),
    ESP_ELFSYM_EXPORT(plibc_stdin), ESP_ELFSYM_EXPORT(plibc_finish), ESP_ELFSYM_EXPORT(sys_setjmp),

    /* --- PUMPKIN SCRIPT API --- */
    ESP_ELFSYM_EXPORT(pumpkin_script_create),
    ESP_ELFSYM_EXPORT(pumpkin_script_destroy),
    ESP_ELFSYM_EXPORT(pumpkin_script_global_iconst),
    ESP_ELFSYM_EXPORT(pumpkin_script_global_iconst_value),
    ESP_ELFSYM_EXPORT(pumpkin_script_call),
    ESP_ELFSYM_EXPORT(pumpkin_script_finish_env),
    ESP_ELFSYM_EXPORT(pumpkin_script_engine_id),
    ESP_ELFSYM_EXPORT(pumpkin_script_run_string),
    ESP_ELFSYM_EXPORT(pumpkin_script_global_function_data),
    ESP_ELFSYM_EXPORT(pumpkin_script_create_obj),
    ESP_ELFSYM_EXPORT(script_push_integer),
    ESP_ELFSYM_EXPORT(script_opt_integer),
    ESP_ELFSYM_EXPORT(script_push_boolean),
    ESP_ELFSYM_EXPORT(script_get_named_value),
    ESP_ELFSYM_EXPORT(script_add_sconst),
    ESP_ELFSYM_EXPORT(script_add_boolean),
    ESP_ELFSYM_EXPORT(script_push_string),
    ESP_ELFSYM_EXPORT(script_opt_string),
    ESP_ELFSYM_EXPORT(script_push_object),
    ESP_ELFSYM_EXPORT(script_loadlib),
    ESP_ELFSYM_EXPORT(script_remove_ref),
    ESP_ELFSYM_EXPORT(script_get_string),
    ESP_ELFSYM_EXPORT(script_get_integer),
    ESP_ELFSYM_EXPORT(script_get_pointer),

    /* --- CHARACTER CLASSIFICATION (sys_is*) --- */
    ESP_ELFSYM_EXPORT(sys_toupper), ESP_ELFSYM_EXPORT(sys_tolower), ESP_ELFSYM_EXPORT(sys_isupper), ESP_ELFSYM_EXPORT(sys_islower),
    ESP_ELFSYM_EXPORT(sys_isdigit), ESP_ELFSYM_EXPORT(sys_isalnum), ESP_ELFSYM_EXPORT(sys_isspace), ESP_ELFSYM_EXPORT(sys_isprint),
    ESP_ELFSYM_EXPORT(sys_isblank), ESP_ELFSYM_EXPORT(sys_ispunct),

    /* --- PUMPKIN / LIBPIT SPECIFIEK --- */
    ESP_ELFSYM_EXPORT(EvtPumpEvents), ESP_ELFSYM_EXPORT(EvtSysEventAvail), ESP_ELFSYM_EXPORT(syntax_get_plugin),
    ESP_ELFSYM_EXPORT(pumpkin_error_msg), ESP_ELFSYM_EXPORT(pumpkin_get_lasterr), ESP_ELFSYM_EXPORT(RGBToLong),
    ESP_ELFSYM_EXPORT(debug_full), ESP_ELFSYM_EXPORT(getext), ESP_ELFSYM_EXPORT(pumpkin_set_size),
    ESP_ELFSYM_EXPORT(pterm_cursor), ESP_ELFSYM_EXPORT(pterm_getchar), ESP_ELFSYM_EXPORT(pterm_setfg),
    ESP_ELFSYM_EXPORT(pterm_setbg), ESP_ELFSYM_EXPORT(pterm_send), ESP_ELFSYM_EXPORT(pterm_home),
    ESP_ELFSYM_EXPORT(pterm_init), ESP_ELFSYM_EXPORT(EvtKeyEventAvail), ESP_ELFSYM_EXPORT(pterm_cursor_blink),
    ESP_ELFSYM_EXPORT(pterm_cls), ESP_ELFSYM_EXPORT(pterm_crlf_mode), ESP_ELFSYM_EXPORT(LongToRGB),
    ESP_ELFSYM_EXPORT(pterm_callback), ESP_ELFSYM_EXPORT(get2b), ESP_ELFSYM_EXPORT(debug_bytes_full),

    /* --- LIBC & SYSTEEM --- */
    ESP_ELFSYM_EXPORT(malloc), ESP_ELFSYM_EXPORT(calloc), ESP_ELFSYM_EXPORT(realloc), ESP_ELFSYM_EXPORT(free),
    ESP_ELFSYM_EXPORT(memset), ESP_ELFSYM_EXPORT(memcpy), ESP_ELFSYM_EXPORT(memmove), ESP_ELFSYM_EXPORT(memcmp),
    ESP_ELFSYM_EXPORT(printf), ESP_ELFSYM_EXPORT(sprintf), ESP_ELFSYM_EXPORT(snprintf), ESP_ELFSYM_EXPORT(vsnprintf),
    ESP_ELFSYM_EXPORT(puts), ESP_ELFSYM_EXPORT(putchar), ESP_ELFSYM_EXPORT(fopen), ESP_ELFSYM_EXPORT(fclose),
    ESP_ELFSYM_EXPORT(fread), ESP_ELFSYM_EXPORT(fwrite), ESP_ELFSYM_EXPORT(fflush), ESP_ELFSYM_EXPORT(fstat),
    ESP_ELFSYM_EXPORT(open), ESP_ELFSYM_EXPORT(close), ESP_ELFSYM_EXPORT(read), ESP_ELFSYM_EXPORT(write),
    ESP_ELFSYM_EXPORT(lseek), ESP_ELFSYM_EXPORT(stat), ESP_ELFSYM_EXPORT(unlink), ESP_ELFSYM_EXPORT(rename),
    ESP_ELFSYM_EXPORT(opendir), ESP_ELFSYM_EXPORT(readdir), ESP_ELFSYM_EXPORT(closedir), ESP_ELFSYM_EXPORT(mkdir),
    ESP_ELFSYM_EXPORT(exit), ESP_ELFSYM_EXPORT(abort), ESP_ELFSYM_EXPORT(getenv), ESP_ELFSYM_EXPORT(setenv),
    ESP_ELFSYM_EXPORT(time), ESP_ELFSYM_EXPORT(clock_gettime), ESP_ELFSYM_EXPORT(usleep), ESP_ELFSYM_EXPORT(sleep),
    ESP_ELFSYM_EXPORT(SysTicksPerSecond), ESP_ELFSYM_EXPORT(SysUIAppSwitch), ESP_ELFSYM_EXPORT(SysTaskDelay),

    /* --- MATH & GCC INTRINSICS --- */
    ESP_ELFSYM_EXPORT(sin), ESP_ELFSYM_EXPORT(cos), ESP_ELFSYM_EXPORT(tan), ESP_ELFSYM_EXPORT(sqrt),
    ESP_ELFSYM_EXPORT(pow), ESP_ELFSYM_EXPORT(log), ESP_ELFSYM_EXPORT(exp), ESP_ELFSYM_EXPORT(floor),
    ESP_ELFSYM_EXPORT(ceil), ESP_ELFSYM_EXPORT(fabs), ESP_ELFSYM_EXPORT(atan2),
    ESP_ELFSYM_EXPORT(__divdi3), ESP_ELFSYM_EXPORT(__udivdi3), ESP_ELFSYM_EXPORT(__moddi3), ESP_ELFSYM_EXPORT(__umoddi3),
    ESP_ELFSYM_EXPORT(__adddf3), ESP_ELFSYM_EXPORT(__subdf3), ESP_ELFSYM_EXPORT(__muldf3), ESP_ELFSYM_EXPORT(__divdf3),
    ESP_ELFSYM_EXPORT(__floatunsidf), ESP_ELFSYM_EXPORT(__fixdfsi), ESP_ELFSYM_EXPORT(__eqdf2), ESP_ELFSYM_EXPORT(__ltdf2),

    /* --- PUMPKINOS CORE API --- */
    ESP_ELFSYM_EXPORT(pumpkin_id2s), ESP_ELFSYM_EXPORT(pumpkin_s2id),
    ESP_ELFSYM_EXPORT(pumpkin_get_mode), ESP_ELFSYM_EXPORT(pumpkin_get_depth), ESP_ELFSYM_EXPORT(pumpkin_get_density),
    ESP_ELFSYM_EXPORT(pumpkin_get_app_creator), ESP_ELFSYM_EXPORT(pumpkin_get_app_localid), ESP_ELFSYM_EXPORT(pumpkin_get_data),
    ESP_ELFSYM_EXPORT(pumpkin_set_data), ESP_ELFSYM_EXPORT(pumpkin_set_finish), ESP_ELFSYM_EXPORT(pumpkin_deploy_files),
    ESP_ELFSYM_EXPORT(pumpkin_ps), ESP_ELFSYM_EXPORT(pumpkin_fork), ESP_ELFSYM_EXPORT(pumpkin_heap_alloc),
    ESP_ELFSYM_EXPORT(pumpkin_heap_realloc), ESP_ELFSYM_EXPORT(pumpkin_local_refresh), ESP_ELFSYM_EXPORT(pumpkin_create_form),
    ESP_ELFSYM_EXPORT(pumpkin_destroy_form), ESP_ELFSYM_EXPORT(pumpkin_refresh_desktop), ESP_ELFSYM_EXPORT(pumpkin_reg_get),
    ESP_ELFSYM_EXPORT(pumpkin_reg_set), ESP_ELFSYM_EXPORT(pumpkin_delete_registry), ESP_ELFSYM_EXPORT(pumpkin_delete_preferences),
    ESP_ELFSYM_EXPORT(pumpkin_taskbar_add), ESP_ELFSYM_EXPORT(pumpkin_taskbar_create), ESP_ELFSYM_EXPORT(pumpkin_taskbar_update),
    ESP_ELFSYM_EXPORT(pumpkin_get_preference), ESP_ELFSYM_EXPORT(pumpkin_set_preference), ESP_ELFSYM_EXPORT(pumpkin_calibrate),
    ESP_ELFSYM_EXPORT(pumpkin_get_build), ESP_ELFSYM_EXPORT(pumpkin_dia_enabled), ESP_ELFSYM_EXPORT(pumpkin_dirty_region_mode),
    ESP_ELFSYM_EXPORT(RctSetRectangle), ESP_ELFSYM_EXPORT(RctInsetRectangle), ESP_ELFSYM_EXPORT(pumpkin_setio),
    ESP_ELFSYM_EXPORT(pumpkin_script_init_env), ESP_ELFSYM_EXPORT(pumpkin_editor_init_io), ESP_ELFSYM_EXPORT(pumpkin_script_init),
    ESP_ELFSYM_EXPORT(pumpkin_get_taskid), ESP_ELFSYM_EXPORT(pumpkin_change_display), ESP_ELFSYM_EXPORT(pumpkin_test_exception),
    ESP_ELFSYM_EXPORT(pumpkin_kill), ESP_ELFSYM_EXPORT(pumpkin_editor_init_term), ESP_ELFSYM_EXPORT(conn_close),
    ESP_ELFSYM_EXPORT(pumpkin_taskbar_remove), ESP_ELFSYM_EXPORT(pumpkin_get_default_osversion), ESP_ELFSYM_EXPORT(pumpkin_taskbar_add_widget),
    ESP_ELFSYM_EXPORT(RctPtInRectangle), ESP_ELFSYM_EXPORT(pumpkin_taskbar_remove_widget), ESP_ELFSYM_EXPORT(pumpkin_taskbar_destroy),
    ESP_ELFSYM_EXPORT(RctCopyRectangle),

    /* --- DEBUG MEMORY WRAPPERS --- */
    ESP_ELFSYM_EXPORT(xfree_debug),
    ESP_ELFSYM_EXPORT(xcalloc_debug),

    /* --- PALMOS WINDOWS & GRAPHICS --- */
    ESP_ELFSYM_EXPORT(TxtCharAttr),
    ESP_ELFSYM_EXPORT(WinSetActiveWindow), ESP_ELFSYM_EXPORT(WinGetActiveWindow), ESP_ELFSYM_EXPORT(WinGetDisplayWindow),
    ESP_ELFSYM_EXPORT(WinGetDrawWindow), ESP_ELFSYM_EXPORT(WinSetDrawWindow), ESP_ELFSYM_EXPORT(WinDrawChars),
    ESP_ELFSYM_EXPORT(WinPaintChars), ESP_ELFSYM_EXPORT(WinSetForeColor), ESP_ELFSYM_EXPORT(WinSetForeColorRGB),
    ESP_ELFSYM_EXPORT(WinSetBackColor), ESP_ELFSYM_EXPORT(WinSetBackColorRGB), ESP_ELFSYM_EXPORT(WinSetTextColor),
    ESP_ELFSYM_EXPORT(WinSetTextColorRGB), ESP_ELFSYM_EXPORT(WinSetDrawMode), ESP_ELFSYM_EXPORT(WinSetBounds),
    ESP_ELFSYM_EXPORT(WinEraseRectangle), ESP_ELFSYM_EXPORT(WinPaintRectangle), ESP_ELFSYM_EXPORT(WinPaintRectangleFrame),
    ESP_ELFSYM_EXPORT(WinInvertRectangle), ESP_ELFSYM_EXPORT(WinCopyRectangle), ESP_ELFSYM_EXPORT(WinRestoreRectangle),
    ESP_ELFSYM_EXPORT(WinDeleteWindow), ESP_ELFSYM_EXPORT(WinPaintBitmap), ESP_ELFSYM_EXPORT(WinPaintPixel),
    ESP_ELFSYM_EXPORT(WinCreateOffscreenWindow), ESP_ELFSYM_EXPORT(WinGetColorTable), ESP_ELFSYM_EXPORT(WinRGBToIndex),
    ESP_ELFSYM_EXPORT(BmpGetBestBitmap), ESP_ELFSYM_EXPORT(WinGetBitmap), ESP_ELFSYM_EXPORT(WinScreenMode),
    ESP_ELFSYM_EXPORT(WinPaintChar), ESP_ELFSYM_EXPORT(BmpCreate3), ESP_ELFSYM_EXPORT(WinDrawChar),
    ESP_ELFSYM_EXPORT(TxtCharSize), ESP_ELFSYM_EXPORT(TxtCompare), ESP_ELFSYM_EXPORT(TxtGlueUpperChar),
    ESP_ELFSYM_EXPORT(TxtGetNextChar), ESP_ELFSYM_EXPORT(TxtCaselessCompare),

    /* --- PALMOS FORMS & CONTROLS --- */
    ESP_ELFSYM_EXPORT(FrmNewForm), ESP_ELFSYM_EXPORT(FrmInitForm), ESP_ELFSYM_EXPORT(FrmDrawForm),
    ESP_ELFSYM_EXPORT(FrmSetActiveForm), ESP_ELFSYM_EXPORT(FrmGetActiveForm), ESP_ELFSYM_EXPORT(FrmGetWindowHandle),
    ESP_ELFSYM_EXPORT(FrmGetObjectPtr), ESP_ELFSYM_EXPORT(FrmGetObjectIndex), ESP_ELFSYM_EXPORT(FrmGetObjectType),
    ESP_ELFSYM_EXPORT(FrmGetObjectBounds), ESP_ELFSYM_EXPORT(FrmSetObjectBounds), ESP_ELFSYM_EXPORT(FrmSetEventHandler),
    ESP_ELFSYM_EXPORT(FrmDispatchEvent), ESP_ELFSYM_EXPORT(FrmDoDialog), ESP_ELFSYM_EXPORT(FrmGotoForm),
    ESP_ELFSYM_EXPORT(FrmUpdateForm), ESP_ELFSYM_EXPORT(FrmCustomAlert), ESP_ELFSYM_EXPORT(FrmSetUsable),
    ESP_ELFSYM_EXPORT(FrmShowObject), ESP_ELFSYM_EXPORT(FrmHideObject), ESP_ELFSYM_EXPORT(FrmDeleteForm),
    ESP_ELFSYM_EXPORT(FrmGetActiveFormID), ESP_ELFSYM_EXPORT(FrmSetColorTrigger), ESP_ELFSYM_EXPORT(FrmCenterDialogs),
    ESP_ELFSYM_EXPORT(FrmCloseAllForms), ESP_ELFSYM_EXPORT(FrmGetFormBounds), ESP_ELFSYM_EXPORT(FrmSetGadgetHandler),
    ESP_ELFSYM_EXPORT(FrmPopupForm), ESP_ELFSYM_EXPORT(FrmReturnToForm), ESP_ELFSYM_EXPORT(FrmDrawObject),
    ESP_ELFSYM_EXPORT(FrmSetTitle), ESP_ELFSYM_EXPORT(FrmSetFocus), ESP_ELFSYM_EXPORT(FrmNewLabel),
    ESP_ELFSYM_EXPORT(FrmSetDIAPolicyAttr), ESP_ELFSYM_EXPORT(FrmSetGadgetData), ESP_ELFSYM_EXPORT(FrmCopyLabel),
    ESP_ELFSYM_EXPORT(FrmGetGadgetData), ESP_ELFSYM_EXPORT(FrmGetFormId), ESP_ELFSYM_EXPORT(FrmGetTitle),
    ESP_ELFSYM_EXPORT(FrmEraseForm), ESP_ELFSYM_EXPORT(FrmAlert), ESP_ELFSYM_EXPORT(FrmGetControlValue),
    ESP_ELFSYM_EXPORT(FrmGetFocus), ESP_ELFSYM_EXPORT(FrmSetControlValue), ESP_ELFSYM_EXPORT(FrmGetFormPtr),
    ESP_ELFSYM_EXPORT(FrmUpdateScrollers), ESP_ELFSYM_EXPORT(FrmGetObjectPosition), ESP_ELFSYM_EXPORT(FrmSaveAllForms),
    ESP_ELFSYM_EXPORT(FrmGetNumberOfObjects), ESP_ELFSYM_EXPORT(FrmSetCategoryLabel), ESP_ELFSYM_EXPORT(FrmSetObjectPosition),
    ESP_ELFSYM_EXPORT(FrmHandleEvent), ESP_ELFSYM_EXPORT(FrmGetFirstForm), ESP_ELFSYM_EXPORT(CategoryCreateList),
    ESP_ELFSYM_EXPORT(FrmGetControlGroupSelection), ESP_ELFSYM_EXPORT(FrmGetActiveField), ESP_ELFSYM_EXPORT(FrmGetObjectId),
    ESP_ELFSYM_EXPORT(CategoryFind), ESP_ELFSYM_EXPORT(CategoryTruncateName), ESP_ELFSYM_EXPORT(SndPlaySmfResource),
    ESP_ELFSYM_EXPORT(FrmCopyTitle), ESP_ELFSYM_EXPORT(FrmVisible), ESP_ELFSYM_EXPORT(SndPlaySmf),
    ESP_ELFSYM_EXPORT(FrmSetControlGroupSelection),

    /* --- DATA MANAGER & STORAGE --- */
    ESP_ELFSYM_EXPORT(DmOpenDatabase), ESP_ELFSYM_EXPORT(DmCloseDatabase), ESP_ELFSYM_EXPORT(DmDeleteDatabase),
    ESP_ELFSYM_EXPORT(DmDatabaseInfo), ESP_ELFSYM_EXPORT(DmDatabaseSize), ESP_ELFSYM_EXPORT(DmNumRecords),
    ESP_ELFSYM_EXPORT(DmGetRecord), ESP_ELFSYM_EXPORT(DmReleaseRecord), ESP_ELFSYM_EXPORT(DmGetResource),
    ESP_ELFSYM_EXPORT(DmGet1Resource), ESP_ELFSYM_EXPORT(DmReleaseResource), ESP_ELFSYM_EXPORT(DmSetDirty),
    ESP_ELFSYM_EXPORT(DmGetLastErr), ESP_ELFSYM_EXPORT(VFSFileOpen), ESP_ELFSYM_EXPORT(VFSFileClose),
    ESP_ELFSYM_EXPORT(VFSFileCreate), ESP_ELFSYM_EXPORT(VFSCurrentDir),
    ESP_ELFSYM_EXPORT(DmGetResourceIndex), ESP_ELFSYM_EXPORT(VFSVolumeEnumerate), ESP_ELFSYM_EXPORT(VFSDirEntryEnumerate),
    ESP_ELFSYM_EXPORT(VFSFileRead), ESP_ELFSYM_EXPORT(VFSFileSize), ESP_ELFSYM_EXPORT(VFSFileDelete),
    ESP_ELFSYM_EXPORT(MemHandleLock), ESP_ELFSYM_EXPORT(MemHandleUnlock), ESP_ELFSYM_EXPORT(DmFindResource), ESP_ELFSYM_EXPORT(DmFindDatabase),
    ESP_ELFSYM_EXPORT(DmCreateDatabase), ESP_ELFSYM_EXPORT(MemHandleLockEx), ESP_ELFSYM_EXPORT(MemHandleSize), ESP_ELFSYM_EXPORT(DmSetDatabaseInfo),
    ESP_ELFSYM_EXPORT(DmNewResourceEx), ESP_ELFSYM_EXPORT(DmResourceLoadLib), ESP_ELFSYM_EXPORT(VFSChangeDir),
    ESP_ELFSYM_EXPORT(DmGetNextDatabaseByTypeCreator), ESP_ELFSYM_EXPORT(CtbGetNumEntries), ESP_ELFSYM_EXPORT(DmNumResources),
    ESP_ELFSYM_EXPORT(MemLocalIDToPtr), ESP_ELFSYM_EXPORT(MemHandleToLocalID), ESP_ELFSYM_EXPORT(DmResourceInfo),
    ESP_ELFSYM_EXPORT(PhoneNumberLookup), ESP_ELFSYM_EXPORT(DmQueryNextInCategory), ESP_ELFSYM_EXPORT(DmStrCopy),
    ESP_ELFSYM_EXPORT(CategoryGetNext), ESP_ELFSYM_EXPORT(DmArchiveRecord), ESP_ELFSYM_EXPORT(DmMoveRecord),
    ESP_ELFSYM_EXPORT(MemHandleNew), ESP_ELFSYM_EXPORT(MemHandleDataStorage), ESP_ELFSYM_EXPORT(DmNumRecordsInCategory),
    ESP_ELFSYM_EXPORT(DmWrite), ESP_ELFSYM_EXPORT(DmNewHandle), ESP_ELFSYM_EXPORT(CategoryInitialize),
    ESP_ELFSYM_EXPORT(DmResizeRecord), ESP_ELFSYM_EXPORT(DmSetRecordInfo), ESP_ELFSYM_EXPORT(DmPositionInCategory),
    ESP_ELFSYM_EXPORT(DmOpenDatabaseInfo), ESP_ELFSYM_EXPORT(DmRemoveRecord), ESP_ELFSYM_EXPORT(CategoryGetName),
    ESP_ELFSYM_EXPORT(DmFindRecordByID), ESP_ELFSYM_EXPORT(DmSet), ESP_ELFSYM_EXPORT(DmAttachRecord),
    ESP_ELFSYM_EXPORT(DmInsertionSort), ESP_ELFSYM_EXPORT(DmQueryRecord), ESP_ELFSYM_EXPORT(DmSeekRecordInCategory),
    ESP_ELFSYM_EXPORT(DmCreateDatabaseFromImage), ESP_ELFSYM_EXPORT(DmOpenDatabaseByTypeCreator), ESP_ELFSYM_EXPORT(DmRecordInfo),
    ESP_ELFSYM_EXPORT(DmDeleteRecord), ESP_ELFSYM_EXPORT(MemLocalIDToGlobal), ESP_ELFSYM_EXPORT(DmNewRecord),
    ESP_ELFSYM_EXPORT(DmDetachRecord), ESP_ELFSYM_EXPORT(MemLocalIDToLockedPtr), ESP_ELFSYM_EXPORT(DmFindSortPosition),
    ESP_ELFSYM_EXPORT(UDAExchangeWriterNew), ESP_ELFSYM_EXPORT(UDAExchangeReaderNew), ESP_ELFSYM_EXPORT(DmQuickSort),

    /* --- STRINGS, FONTS & UTILS --- */
    ESP_ELFSYM_EXPORT(StrLen), ESP_ELFSYM_EXPORT(StrDup), ESP_ELFSYM_EXPORT(StrCat), ESP_ELFSYM_EXPORT(StrCopy),
    ESP_ELFSYM_EXPORT(StrCompare), ESP_ELFSYM_EXPORT(StrNCopy), ESP_ELFSYM_EXPORT(StrNCat), ESP_ELFSYM_EXPORT(StrPrintF),
    ESP_ELFSYM_EXPORT(StrCaselessCompare), ESP_ELFSYM_EXPORT(FntLoadFont), ESP_ELFSYM_EXPORT(FntSetFont),
    ESP_ELFSYM_EXPORT(FntCharHeight), ESP_ELFSYM_EXPORT(FntCharWidth), ESP_ELFSYM_EXPORT(FntCharsWidth),
    ESP_ELFSYM_EXPORT(LstSetListChoices), ESP_ELFSYM_EXPORT(LstGetSelection), ESP_ELFSYM_EXPORT(LstGetSelectionText), ESP_ELFSYM_EXPORT(LstSetSelection),
    ESP_ELFSYM_EXPORT(SelectDay), ESP_ELFSYM_EXPORT(CtlSetValue), ESP_ELFSYM_EXPORT(SysStringByIndex),
    ESP_ELFSYM_EXPORT(TimeToAscii), ESP_ELFSYM_EXPORT(SelectTimeZone), ESP_ELFSYM_EXPORT(DateToAscii),
    ESP_ELFSYM_EXPORT(SysQSortP), ESP_ELFSYM_EXPORT(MemSet), ESP_ELFSYM_EXPORT(LmGetLocaleSetting),
    ESP_ELFSYM_EXPORT(PrefSetPreference), ESP_ELFSYM_EXPORT(PrefGetPreferences), ESP_ELFSYM_EXPORT(SelectOneTime), ESP_ELFSYM_EXPORT(EvtGetEvent),
    ESP_ELFSYM_EXPORT(EvtAddEventToQueue), ESP_ELFSYM_EXPORT(PrefGetAppPreferences),
    ESP_ELFSYM_EXPORT(SysHandleEvent), ESP_ELFSYM_EXPORT(LmGetNumLocales), ESP_ELFSYM_EXPORT(SysNotifyBroadcast),
    ESP_ELFSYM_EXPORT(TimGetSeconds), ESP_ELFSYM_EXPORT(TimSecondsToDateTime), ESP_ELFSYM_EXPORT(MemPtrNew),
    ESP_ELFSYM_EXPORT(SndStreamStart), ESP_ELFSYM_EXPORT(ptr_new), ESP_ELFSYM_EXPORT(ptr_lock_full),
    ESP_ELFSYM_EXPORT(PrefSetAppPreferences), ESP_ELFSYM_EXPORT(ClipboardAddItem), ESP_ELFSYM_EXPORT(editor_get_plugin),
    ESP_ELFSYM_EXPORT(telnet_filter), ESP_ELFSYM_EXPORT(MemChunkFree), ESP_ELFSYM_EXPORT(telnet_close),
    ESP_ELFSYM_EXPORT(CtlGetValue), ESP_ELFSYM_EXPORT(SndStreamCreate), ESP_ELFSYM_EXPORT(conn_filter),
    ESP_ELFSYM_EXPORT(WavFileHeader), ESP_ELFSYM_EXPORT(MemMove), ESP_ELFSYM_EXPORT(MemCmp),
    ESP_ELFSYM_EXPORT(SysAppLaunch), ESP_ELFSYM_EXPORT(StrNCompare), ESP_ELFSYM_EXPORT(SndStreamDelete),
    ESP_ELFSYM_EXPORT(ClipboardGetItem), ESP_ELFSYM_EXPORT(ptr_unlock_full), ESP_ELFSYM_EXPORT(ptr_free_full),
    ESP_ELFSYM_EXPORT(SysQSort), ESP_ELFSYM_EXPORT(LstSetDrawFunction), ESP_ELFSYM_EXPORT(StrAToI),
    ESP_ELFSYM_EXPORT(LstPopupList), ESP_ELFSYM_EXPORT(FtrGet), ESP_ELFSYM_EXPORT(LstSetPosition),
    ESP_ELFSYM_EXPORT(SysNotifyUnregister), ESP_ELFSYM_EXPORT(PINSetInputTriggerState), ESP_ELFSYM_EXPORT(SysNotifyRegister),
    ESP_ELFSYM_EXPORT(KbdGrfSetState), ESP_ELFSYM_EXPORT(StrNPrintF), ESP_ELFSYM_EXPORT(LstSetHeight),
    ESP_ELFSYM_EXPORT(KbdGrfGetState), ESP_ELFSYM_EXPORT(PrefGetPreference), ESP_ELFSYM_EXPORT(TblSetColumnUsable),
    ESP_ELFSYM_EXPORT(TblSetRowUsable), ESP_ELFSYM_EXPORT(FntWidthToOffset), ESP_ELFSYM_EXPORT(CtbGetEntry),
    ESP_ELFSYM_EXPORT(CtlNewControl), ESP_ELFSYM_EXPORT(SysAppLaunchEx), ESP_ELFSYM_EXPORT(CategorySetTriggerLabel),
    ESP_ELFSYM_EXPORT(TblInsertRow), ESP_ELFSYM_EXPORT(TblSetRowHeight), ESP_ELFSYM_EXPORT(TxtEncodingName),
    ESP_ELFSYM_EXPORT(ExgGetRegisteredApplications), ESP_ELFSYM_EXPORT(TblSetItemFont), ESP_ELFSYM_EXPORT(ErrDisplayFileLineMsg),
    ESP_ELFSYM_EXPORT(ErrExceptionList), ESP_ELFSYM_EXPORT(ExgPut), ESP_ELFSYM_EXPORT(FindGetLineBounds),
    ESP_ELFSYM_EXPORT(TxtParamString), ESP_ELFSYM_EXPORT(TblGetLastUsableRow), ESP_ELFSYM_EXPORT(TblGetBounds),
    ESP_ELFSYM_EXPORT(TblSetRowID), ESP_ELFSYM_EXPORT(TblMarkRowInvalid), ESP_ELFSYM_EXPORT(TblRowUsable),
    ESP_ELFSYM_EXPORT(StrStr), ESP_ELFSYM_EXPORT(SysCurAppDatabase), ESP_ELFSYM_EXPORT(SndPlaySystemSound),
    ESP_ELFSYM_EXPORT(SysFormPointerArrayToStrings), ESP_ELFSYM_EXPORT(TblGetRowID), ESP_ELFSYM_EXPORT(TblGetNumberOfRows),
    ESP_ELFSYM_EXPORT(StrChr), ESP_ELFSYM_EXPORT(MemHandleFree), ESP_ELFSYM_EXPORT(TxtFindString),
    ESP_ELFSYM_EXPORT(FntLineHeight), ESP_ELFSYM_EXPORT(MemPtrUnlock), ESP_ELFSYM_EXPORT(TblSetRowData),
    ESP_ELFSYM_EXPORT(TblRemoveRow), ESP_ELFSYM_EXPORT(TxtStrEncoding), ESP_ELFSYM_EXPORT(ExgAccept),
    ESP_ELFSYM_EXPORT(TblDrawTable), ESP_ELFSYM_EXPORT(SecVerifyPW), ESP_ELFSYM_EXPORT(FntCharsInWidth),
    ESP_ELFSYM_EXPORT(CategorySelect), ESP_ELFSYM_EXPORT(CtlGetLabel), ESP_ELFSYM_EXPORT(TblRedrawTable),
    ESP_ELFSYM_EXPORT(TblGetItemBounds), ESP_ELFSYM_EXPORT(TblGetRowHeight), ESP_ELFSYM_EXPORT(GrfGetState),
    ESP_ELFSYM_EXPORT(SecSelectViewStatus), ESP_ELFSYM_EXPORT(StrIToA), ESP_ELFSYM_EXPORT(TblGetRowData),
    ESP_ELFSYM_EXPORT(ErrThrow), ESP_ELFSYM_EXPORT(FontSelect), ESP_ELFSYM_EXPORT(ExgDoDialog),
    ESP_ELFSYM_EXPORT(EvtGetPen), ESP_ELFSYM_EXPORT(ExgRegisterDatatype), ESP_ELFSYM_EXPORT(ExgReceive),
    ESP_ELFSYM_EXPORT(TblSetCustomDrawProcedure), ESP_ELFSYM_EXPORT(TblEraseTable), ESP_ELFSYM_EXPORT(GrfSetState),
    ESP_ELFSYM_EXPORT(ExgSend), ESP_ELFSYM_EXPORT(ExgDisconnect), ESP_ELFSYM_EXPORT(FindSaveMatch),
    ESP_ELFSYM_EXPORT(TblFindRowID), ESP_ELFSYM_EXPORT(TblSetItemStyle), ESP_ELFSYM_EXPORT(TblSetSaveDataProcedure),
    ESP_ELFSYM_EXPORT(CtlHideControl), ESP_ELFSYM_EXPORT(PdiWritePropertyValue), ESP_ELFSYM_EXPORT(PdiReadProperty),
    ESP_ELFSYM_EXPORT(TblSetColumnSpacing), ESP_ELFSYM_EXPORT(PdiDefineResizing), ESP_ELFSYM_EXPORT(PdiWritePropertyStr),
    ESP_ELFSYM_EXPORT(MemHandleResize), ESP_ELFSYM_EXPORT(LstGetVisibleItems), ESP_ELFSYM_EXPORT(PdiReaderNew),
    ESP_ELFSYM_EXPORT(PdiWriteParameter), ESP_ELFSYM_EXPORT(TblSetItemInt), ESP_ELFSYM_EXPORT(TblGetCurrentField),
    ESP_ELFSYM_EXPORT(MemPtrRecoverHandle), ESP_ELFSYM_EXPORT(TblSelectItem), ESP_ELFSYM_EXPORT(AttnIndicatorEnable),
    ESP_ELFSYM_EXPORT(TblSetColumnMasked), ESP_ELFSYM_EXPORT(TblGetSelection), ESP_ELFSYM_EXPORT(StrVPrintF),
    ESP_ELFSYM_EXPORT(TblSetLoadDataProcedure), ESP_ELFSYM_EXPORT(KeyCurrentState), ESP_ELFSYM_EXPORT(PdiLibOpen),
    ESP_ELFSYM_EXPORT(PdiWriteBeginObject), ESP_ELFSYM_EXPORT(FntGetFont), ESP_ELFSYM_EXPORT(TblReleaseFocus),
    ESP_ELFSYM_EXPORT(LstScrollList), ESP_ELFSYM_EXPORT(TblEditing), ESP_ELFSYM_EXPORT(TimGetTicks),
    ESP_ELFSYM_EXPORT(PdiReadPropertyField), ESP_ELFSYM_EXPORT(TxtSetNextChar), ESP_ELFSYM_EXPORT(SysLibFind),
    ESP_ELFSYM_EXPORT(TblGrabFocus), ESP_ELFSYM_EXPORT(PdiWriterDelete), ESP_ELFSYM_EXPORT(TblHandleEvent),
    ESP_ELFSYM_EXPORT(TblRowMasked), ESP_ELFSYM_EXPORT(PdiEnterObject), ESP_ELFSYM_EXPORT(TxtGetChar),
    ESP_ELFSYM_EXPORT(PdiWriteProperty), ESP_ELFSYM_EXPORT(TblSetRowMasked), ESP_ELFSYM_EXPORT(TblSetItemPtr),
    ESP_ELFSYM_EXPORT(PdiWritePropertyFields), ESP_ELFSYM_EXPORT(SysLibLoad), ESP_ELFSYM_EXPORT(StrNCaselessCompare),
    ESP_ELFSYM_EXPORT(KeySetMask), ESP_ELFSYM_EXPORT(TblSetColumnWidth), ESP_ELFSYM_EXPORT(PdiWriterNew),
    ESP_ELFSYM_EXPORT(TblGetItemFont), ESP_ELFSYM_EXPORT(SysLibRemove), ESP_ELFSYM_EXPORT(TblGetColumnWidth),
    ESP_ELFSYM_EXPORT(TblMarkTableInvalid), ESP_ELFSYM_EXPORT(TblGetItemInt), ESP_ELFSYM_EXPORT(PdiWriteParameterStr),
    ESP_ELFSYM_EXPORT(LstSetTopItem), ESP_ELFSYM_EXPORT(PdiReaderDelete), ESP_ELFSYM_EXPORT(MemPtrResize),
    ESP_ELFSYM_EXPORT(TblUnhighlightSelection), ESP_ELFSYM_EXPORT(PdiLibClose), ESP_ELFSYM_EXPORT(MemPtrSize),
    ESP_ELFSYM_EXPORT(FntGlueGetDefaultFontID), ESP_ELFSYM_EXPORT(CategoryFreeList), ESP_ELFSYM_EXPORT(DateToDOWDMFormat),
    ESP_ELFSYM_EXPORT(TblFindRowData), ESP_ELFSYM_EXPORT(DayOfWeek), ESP_ELFSYM_EXPORT(TxtTransliterate),
    ESP_ELFSYM_EXPORT(TblGetColumnSpacing), ESP_ELFSYM_EXPORT(SndGetDefaultVolume), ESP_ELFSYM_EXPORT(DateSecondsToDate),
    ESP_ELFSYM_EXPORT(SndSetDefaultVolume), ESP_ELFSYM_EXPORT(TimAdjust), ESP_ELFSYM_EXPORT(TblRowInvalid),
    ESP_ELFSYM_EXPORT(FntLineWidth), ESP_ELFSYM_EXPORT(CtlEnabled), ESP_ELFSYM_EXPORT(DateAdjust),
    ESP_ELFSYM_EXPORT(StrToLower), ESP_ELFSYM_EXPORT(AttnGetAttention), ESP_ELFSYM_EXPORT(DateDaysToDate),
    ESP_ELFSYM_EXPORT(DateToDays), ESP_ELFSYM_EXPORT(FtrSet), ESP_ELFSYM_EXPORT(SelectTime),
    ESP_ELFSYM_EXPORT(TimDateTimeToSeconds), ESP_ELFSYM_EXPORT(DayOfMonth), ESP_ELFSYM_EXPORT(DaysInMonth),
    ESP_ELFSYM_EXPORT(ResLoadConstant), ESP_ELFSYM_EXPORT(SndCreateMidiList), ESP_ELFSYM_EXPORT(TimUTCToTimeZone),
    ESP_ELFSYM_EXPORT(CtlEraseControl), ESP_ELFSYM_EXPORT(TblSetBounds), ESP_ELFSYM_EXPORT(CtlDrawControl),
    ESP_ELFSYM_EXPORT(AttnIterate), ESP_ELFSYM_EXPORT(TblSetColumnEditIndicator), ESP_ELFSYM_EXPORT(CtlHandleEvent),
    ESP_ELFSYM_EXPORT(DateTemplateToAscii), ESP_ELFSYM_EXPORT(FntAverageCharWidth), ESP_ELFSYM_EXPORT(AttnUpdate),
    ESP_ELFSYM_EXPORT(SysInsertionSort), ESP_ELFSYM_EXPORT(AlmGetAlarm), ESP_ELFSYM_EXPORT(AttnForgetIt),
    ESP_ELFSYM_EXPORT(MemPtrSetOwner),

    /* --- PUMPKINOS GRAPHICS --- */
    ESP_ELFSYM_EXPORT(surface_create), ESP_ELFSYM_EXPORT(surface_destroy), ESP_ELFSYM_EXPORT(surface_load_mem),
    ESP_ELFSYM_EXPORT(surface_scale), ESP_ELFSYM_EXPORT(surface_buffer), ESP_ELFSYM_EXPORT(rgb565),
    ESP_ELFSYM_EXPORT(r565), ESP_ELFSYM_EXPORT(g565), ESP_ELFSYM_EXPORT(b565), ESP_ELFSYM_EXPORT(tos_convert_color),
    ESP_ELFSYM_EXPORT(BmpSetLittleEndianBits), ESP_ELFSYM_EXPORT(BmpDrawSurface), ESP_ELFSYM_EXPORT(BmpCreateBitmapV3), ESP_ELFSYM_EXPORT(BmpGetDensity),
    ESP_ELFSYM_EXPORT(BmpGetBitDepth), ESP_ELFSYM_EXPORT(WinDrawLine), ESP_ELFSYM_EXPORT(UIPickColor), ESP_ELFSYM_EXPORT(AbtShowAboutPumpkin),
    ESP_ELFSYM_EXPORT(CtlSetLabel), ESP_ELFSYM_EXPORT(BmpDelete), ESP_ELFSYM_EXPORT(MenuHandleEvent),
    ESP_ELFSYM_EXPORT(WinDrawRectangle), ESP_ELFSYM_EXPORT(BmpGetDimensions), ESP_ELFSYM_EXPORT(BmpCreate),
    ESP_ELFSYM_EXPORT(WinScreenGetAttribute), ESP_ELFSYM_EXPORT(WinPaintLine), ESP_ELFSYM_EXPORT(WinScrollRectangle),
    ESP_ELFSYM_EXPORT(BmpGetBits), ESP_ELFSYM_EXPORT(BmpGetNextBitmapAnyDensity), ESP_ELFSYM_EXPORT(MenuSetActiveMenu),
    ESP_ELFSYM_EXPORT(WinSetClipingBounds), ESP_ELFSYM_EXPORT(BmpSetPixel), ESP_ELFSYM_EXPORT(FldGetTextLength),
    ESP_ELFSYM_EXPORT(BmpGetTransparentValue), ESP_ELFSYM_EXPORT(FldGetAttributes), ESP_ELFSYM_EXPORT(MenuDispose),
    ESP_ELFSYM_EXPORT(WinSetPatternType), ESP_ELFSYM_EXPORT(UIColorSetTableEntry), ESP_ELFSYM_EXPORT(UIColorGetTableEntryIndex),
    ESP_ELFSYM_EXPORT(FldDelete), ESP_ELFSYM_EXPORT(BmpGetColortable), ESP_ELFSYM_EXPORT(FldGetTextPtr),
    ESP_ELFSYM_EXPORT(BmpGetVersion), ESP_ELFSYM_EXPORT(FldNewField), ESP_ELFSYM_EXPORT(LstGetNumberOfItems),
    ESP_ELFSYM_EXPORT(SclSetScrollBar), ESP_ELFSYM_EXPORT(FldSetAttributes), ESP_ELFSYM_EXPORT(WinGetPatternType),
    ESP_ELFSYM_EXPORT(PINSetInputAreaState), ESP_ELFSYM_EXPORT(FldInsert), ESP_ELFSYM_EXPORT(tos_write_screen),
    ESP_ELFSYM_EXPORT(WinAdjustCoords), ESP_ELFSYM_EXPORT(FldSetTextHandle), ESP_ELFSYM_EXPORT(UIColorGetTableEntryRGB),
    ESP_ELFSYM_EXPORT(FldGetTextHandle), ESP_ELFSYM_EXPORT(BmpGetPixelValue), ESP_ELFSYM_EXPORT(MenuInit),
    ESP_ELFSYM_EXPORT(FldGetScrollPosition), ESP_ELFSYM_EXPORT(WinFillRectangle), ESP_ELFSYM_EXPORT(FindDrawHeader),
    ESP_ELFSYM_EXPORT(WinGetPattern), ESP_ELFSYM_EXPORT(WinPopDrawState), ESP_ELFSYM_EXPORT(WinDrawBitmap),
    ESP_ELFSYM_EXPORT(WinPushDrawState), ESP_ELFSYM_EXPORT(FldScrollable), ESP_ELFSYM_EXPORT(MenuShowItem),
    ESP_ELFSYM_EXPORT(FldGetVisibleLines), ESP_ELFSYM_EXPORT(FldDirty), ESP_ELFSYM_EXPORT(FldSetScrollPosition),
    ESP_ELFSYM_EXPORT(FldSetFont), ESP_ELFSYM_EXPORT(WinRestoreBits), ESP_ELFSYM_EXPORT(FldSetSelection),
    ESP_ELFSYM_EXPORT(MenuCmdBarAddButton), ESP_ELFSYM_EXPORT(FldGetScrollValues), ESP_ELFSYM_EXPORT(FldScrollField),
    ESP_ELFSYM_EXPORT(FldGetSelection), ESP_ELFSYM_EXPORT(FldReleaseFocus), ESP_ELFSYM_EXPORT(WinSaveBits),
    ESP_ELFSYM_EXPORT(FldGetNumberOfBlankLines), ESP_ELFSYM_EXPORT(MenuEraseStatus), ESP_ELFSYM_EXPORT(WinSetPattern),
    ESP_ELFSYM_EXPORT(AbtShowAbout), ESP_ELFSYM_EXPORT(WinGetDisplayExtent), ESP_ELFSYM_EXPORT(FldHandleEvent),
    ESP_ELFSYM_EXPORT(MenuHideItem), ESP_ELFSYM_EXPORT(FldGrabFocus), ESP_ELFSYM_EXPORT(FldCompactText),
    ESP_ELFSYM_EXPORT(WinDrawTruncChars), ESP_ELFSYM_EXPORT(FldSetMaxChars), ESP_ELFSYM_EXPORT(FldGetBounds),
    ESP_ELFSYM_EXPORT(WinGetDrawWindowBounds), ESP_ELFSYM_EXPORT(FldWordWrap), ESP_ELFSYM_EXPORT(WinDrawInvertedChars),
    ESP_ELFSYM_EXPORT(FldCalcFieldHeight), ESP_ELFSYM_EXPORT(FldSetText), ESP_ELFSYM_EXPORT(FldDrawField),
    ESP_ELFSYM_EXPORT(FldGetMaxChars), ESP_ELFSYM_EXPORT(WinScreenUnlock), ESP_ELFSYM_EXPORT(FldGetInsPtPosition),
    ESP_ELFSYM_EXPORT(FldMakeFullyVisible), ESP_ELFSYM_EXPORT(FldSetInsPtPosition), ESP_ELFSYM_EXPORT(FldFreeMemory),
    ESP_ELFSYM_EXPORT(WinScreenLock), ESP_ELFSYM_EXPORT(WinDrawRectangleFrame), ESP_ELFSYM_EXPORT(WinSetClip),
    ESP_ELFSYM_EXPORT(WinGetClip), ESP_ELFSYM_EXPORT(WinEraseLine), ESP_ELFSYM_EXPORT(AlmSetAlarm),
    ESP_ELFSYM_EXPORT(FldEraseField), ESP_ELFSYM_EXPORT(WinGetWindowExtent), ESP_ELFSYM_EXPORT(WinDrawGrayLine),
    ESP_ELFSYM_EXPORT(SclGetScrollBar),

    /* --- SYNC & MUTEX & THREAD --- */
    ESP_ELFSYM_EXPORT(mutex_create), ESP_ELFSYM_EXPORT(mutex_lock), ESP_ELFSYM_EXPORT(mutex_unlock), ESP_ELFSYM_EXPORT(mutex_destroy),
    ESP_ELFSYM_EXPORT(thread_must_end),

    ESP_ELFSYM_END
};

static const struct esp_elfsym g_esp_libc_elfsyms[] = {

    /* string.h */

    ESP_ELFSYM_EXPORT(strerror),
    ESP_ELFSYM_EXPORT(memset),
    ESP_ELFSYM_EXPORT(memcpy),
    ESP_ELFSYM_EXPORT(strlen),
    ESP_ELFSYM_EXPORT(strtod),
    ESP_ELFSYM_EXPORT(strrchr),
    ESP_ELFSYM_EXPORT(strchr),
    ESP_ELFSYM_EXPORT(strcmp),
    ESP_ELFSYM_EXPORT(strtol),
    ESP_ELFSYM_EXPORT(strcspn),
    ESP_ELFSYM_EXPORT(strncat),

    /* stdio.h */

    ESP_ELFSYM_EXPORT(puts),
    ESP_ELFSYM_EXPORT(putchar),
    ESP_ELFSYM_EXPORT(fputc),
    ESP_ELFSYM_EXPORT(fputs),
    ESP_ELFSYM_EXPORT(printf),
    ESP_ELFSYM_EXPORT(vfprintf),
    ESP_ELFSYM_EXPORT(fprintf),
    ESP_ELFSYM_EXPORT(fwrite),

    /* unistd.h */

    ESP_ELFSYM_EXPORT(usleep),
    ESP_ELFSYM_EXPORT(sleep),
    ESP_ELFSYM_EXPORT(exit),
    ESP_ELFSYM_EXPORT(close),

    /* stdlib.h */

    ESP_ELFSYM_EXPORT(malloc),
    ESP_ELFSYM_EXPORT(calloc),
    ESP_ELFSYM_EXPORT(realloc),
    ESP_ELFSYM_EXPORT(free),

    /* time.h */

    ESP_ELFSYM_EXPORT(clock_gettime),
    ESP_ELFSYM_EXPORT(strftime),

    /* pthread.h */

    ESP_ELFSYM_EXPORT(pthread_create),
    ESP_ELFSYM_EXPORT(pthread_attr_init),
    ESP_ELFSYM_EXPORT(pthread_attr_setstacksize),
    ESP_ELFSYM_EXPORT(pthread_detach),
    ESP_ELFSYM_EXPORT(pthread_join),
    ESP_ELFSYM_EXPORT(pthread_exit),

    /* newlib */

#if !CONFIG_LIBC_PICOLIBC
    ESP_ELFSYM_EXPORT(__errno),
    ESP_ELFSYM_EXPORT(__getreent),

#if defined(__HAVE_LOCALE_INFO__)
    ESP_ELFSYM_EXPORT(__locale_ctype_ptr),
#else
    ESP_ELFSYM_EXPORT(_ctype_),
#endif
#endif

    /* math */

    ESP_ELFSYM_EXPORT(__ltdf2),
    ESP_ELFSYM_EXPORT(__fixunsdfsi),
    ESP_ELFSYM_EXPORT(__gtdf2),
    ESP_ELFSYM_EXPORT(__floatunsidf),
    ESP_ELFSYM_EXPORT(__divdf3),

    /* getopt.h */

    ESP_ELFSYM_EXPORT(getopt_long),
    ESP_ELFSYM_EXPORT(optind),
    ESP_ELFSYM_EXPORT(opterr),
    ESP_ELFSYM_EXPORT(optarg),
    ESP_ELFSYM_EXPORT(optopt),

    /* setjmp.h */

    ESP_ELFSYM_EXPORT(longjmp),
    ESP_ELFSYM_EXPORT(setjmp),

    ESP_ELFSYM_END
};

/** @brief ESP-IDF public functions symbols look-up table */

static const struct esp_elfsym g_esp_espidf_elfsyms[] = {

    /* sys/socket.h */

    ESP_ELFSYM_EXPORT(lwip_bind),
    ESP_ELFSYM_EXPORT(lwip_setsockopt),
    ESP_ELFSYM_EXPORT(lwip_socket),
    ESP_ELFSYM_EXPORT(lwip_listen),
    ESP_ELFSYM_EXPORT(lwip_accept),
    ESP_ELFSYM_EXPORT(lwip_recv),
    ESP_ELFSYM_EXPORT(lwip_recvfrom),
    ESP_ELFSYM_EXPORT(lwip_send),
    ESP_ELFSYM_EXPORT(lwip_sendto),
    ESP_ELFSYM_EXPORT(lwip_connect),

    /* arpa/inet.h */

    ESP_ELFSYM_EXPORT(ipaddr_addr),
    ESP_ELFSYM_EXPORT(lwip_htons),
    ESP_ELFSYM_EXPORT(lwip_htonl),
    ESP_ELFSYM_EXPORT(ip4addr_ntoa),

    /* ROM functions */

    ESP_ELFSYM_EXPORT(ets_printf),

    ESP_ELFSYM_END
};

/**
 * @brief Find symbol address by name.
 *
 * @param sym_name - Symbol name
 *
 * @return Symbol address if success or 0 if failed.
 */
uintptr_t elf_find_sym(const char *sym_name)
{
    esp_elf_symbol_table_t *syms;

#ifdef CONFIG_ELF_LOADER_LIBC_SYMBOLS
    syms = g_esp_libc_elfsyms;
    while (syms->name) {
        if (!strcmp(syms->name, sym_name)) {
            return (uintptr_t)syms->sym;
        }

        syms++;
    }

    syms = g_palm_pumpkin_elfsyms;
    while (syms->name) {
        if (!strcmp(syms->name, sym_name)) {
            return (uintptr_t)syms->sym;
        }

        syms++;
    }
#else
    syms = g_esp_libc_elfsyms;
    (void)syms;
#endif

#ifdef CONFIG_ELF_LOADER_ESPIDF_SYMBOLS
    syms = g_esp_espidf_elfsyms;
    while (syms->name) {
        if (!strcmp(syms->name, sym_name)) {
            return (uintptr_t)syms->sym;
        }

        syms++;
    }
#else
    syms = g_esp_espidf_elfsyms;
    (void)syms;
#endif

#ifdef CONFIG_ELF_LOADER_CUSTOMER_SYMBOLS
    extern const struct esp_elfsym g_customer_elfsyms[];

    syms = g_customer_elfsyms;
    while (syms->name) {
        if (!strcmp(syms->name, sym_name)) {
            return (uintptr_t)syms->sym;
        }

        syms++;
    }

#endif

    uintptr_t sym_addr = esp_elf_find_symbol(sym_name);
    if (sym_addr) {
        return sym_addr;
    }

#if CONFIG_ELF_DYNAMIC_LOAD_SHARED_OBJECT
    return (uintptr_t)dlmod_getaddr(sym_name);
#else
    return 0;
#endif
}
