/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>

#include "private/elf_symbol.h"

/* Extern declarations from ELF symbol table */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wbuiltin-declaration-mismatch"
extern int deserialize_launch;
extern int deserialize_sysAppLaunchCmdAlarmTriggered;
extern int deserialize_sysAppLaunchCmdDisplayAlarm;
extern int deserialize_sysAppLaunchCmdGoTo;
extern int deserialize_sysAppLaunchCmdPanelCalledFromApp;
extern int deserialize_sysAppLaunchCmdSystemReset;
extern int serialize_launch;
extern int serialize_sysAppLaunchCmdAlarmTriggered;
extern int serialize_sysAppLaunchCmdDisplayAlarm;
extern int serialize_sysAppLaunchCmdGoTo;
extern int serialize_sysAppLaunchCmdPanelCalledFromApp;
extern int serialize_sysAppLaunchCmdSystemReset;
extern int deserialize_notif;
extern int deserialize_sysNotifyAppCrashedEvent;
extern int deserialize_sysNotifyAppLaunchingEvent;
extern int deserialize_sysNotifyAppQuittingEvent;
extern int deserialize_sysNotifyCardInsertedEvent;
extern int deserialize_sysNotifyCardRemovedEvent;
extern int deserialize_sysNotifyDBCreatedEvent;
extern int deserialize_sysNotifyDBDeletedEvent;
extern int deserialize_sysNotifyDisplayChangeEvent;
extern int deserialize_sysNotifyGPSDataEvent;
extern int deserialize_sysNotifyLocaleChangedEvent;
extern int deserialize_sysNotifySelectDay;
extern int deserialize_sysNotifySyncFinishEvent;
extern int deserialize_sysNotifyTimeChangeEvent;
extern int deserialize_sysNotifyVolumeMountedEvent;
extern int deserialize_sysNotifyVolumeUnmountedEvent;
extern int serialize_notif;
extern int serialize_sysNotifyAppCrashedEvent;
extern int serialize_sysNotifyAppLaunchingEvent;
extern int serialize_sysNotifyAppQuittingEvent;
extern int serialize_sysNotifyCardInsertedEvent;
extern int serialize_sysNotifyCardRemovedEvent;
extern int serialize_sysNotifyDBCreatedEvent;
extern int serialize_sysNotifyDBDeletedEvent;
extern int serialize_sysNotifyDisplayChangeEvent;
extern int serialize_sysNotifyGPSDataEvent;
extern int serialize_sysNotifyLocaleChangedEvent;
extern int serialize_sysNotifySelectDay;
extern int serialize_sysNotifySyncFinishEvent;
extern int serialize_sysNotifyTimeChangeEvent;
extern int serialize_sysNotifyVolumeMountedEvent;
extern int serialize_sysNotifyVolumeUnmountedEvent;
extern int pumpkin_get_build;
extern int LanguageGet;
extern int LanguageSelect;
extern int LongToRGB;
extern int RGBToLong;
extern int SysAppLaunch;
extern int SysAppLaunchEx;
extern int SysLibCancelRefNum68K;
extern int SysLibFind68K;
extern int SysLibGetDispatch68K;
extern int SysLibNewRefNum68K;
extern int SysLibRegister68K;
extern int SysLibTblEntry68K;
extern int SysNotifyBroadcast;
extern int SysNotifyBroadcastDeferred;
extern int SysNotifyBroadcastFromInterrupt;
extern int SysNotifyBroadcastQueued;
extern int SysNotifyRegister;
extern int SysNotifyUnregister;
extern int grail_draw_stroke;
extern int heap_assertion_error;
extern int heap_exhausted_error;
extern int heap_get;
extern int logtrap_get_def;
extern int pumpkin_add_serial;
extern int pumpkin_alarm_check;
extern int pumpkin_alarm_get;
extern int pumpkin_alarm_set;
extern int pumpkin_app_crashed;
extern int pumpkin_audio_get;
extern int pumpkin_audio_set;
extern int pumpkin_audio_task_finish;
extern int pumpkin_audio_task_init;
extern int pumpkin_baud_serial;
extern int pumpkin_calibrate;
extern int pumpkin_change_display;
extern int pumpkin_clipboard_add_bitmap;
extern int pumpkin_clipboard_add_text;
extern int pumpkin_clipboard_append_text;
extern int pumpkin_clipboard_get_text;
extern int pumpkin_close_serial;
extern int pumpkin_crash_log;
extern int pumpkin_delete_preferences;
extern int pumpkin_delete_registry;
extern int pumpkin_deploy_files;
extern int pumpkin_dia_enabled;
extern int pumpkin_dia_get_state;
extern int pumpkin_dia_get_taskbar_dimension;
extern int pumpkin_dia_get_trigger;
extern int pumpkin_dia_set_graffiti_state;
extern int pumpkin_dia_set_state;
extern int pumpkin_dia_set_trigger;
extern int pumpkin_dirty_region_mode;
extern int pumpkin_dt;
extern int pumpkin_enum_plugins;
extern int pumpkin_error_dialog;
extern int pumpkin_error_msg;
extern int pumpkin_event;
extern int pumpkin_event_peek;
extern int pumpkin_event_timeout;
extern int pumpkin_extkey_down;
extern int pumpkin_fatal_error;
extern int pumpkin_fork;
extern int pumpkin_forward_event;
extern int pumpkin_forward_msg;
extern int pumpkin_generic_error;
extern int pumpkin_get_app_creator;
extern int pumpkin_get_app_localid;
extern int pumpkin_get_battery;
extern int pumpkin_get_boolean_option;
extern int pumpkin_get_current;
extern int pumpkin_get_data;
extern int pumpkin_get_default_osversion;
extern int pumpkin_get_density;
extern int pumpkin_get_depth;
extern int pumpkin_get_encoding;
extern int pumpkin_get_exception;
extern int pumpkin_get_font;
extern int pumpkin_get_id_option;
extern int pumpkin_get_integer_option;
extern int pumpkin_get_lasterr;
extern int pumpkin_get_local_storage;
extern int pumpkin_get_mode;
extern int pumpkin_get_native_keys;
extern int pumpkin_get_osversion;
extern int pumpkin_get_param_size;
extern int pumpkin_get_plugin;
extern int pumpkin_get_preference;
extern int pumpkin_get_serial;
extern int pumpkin_get_serial_by_creator;
extern int pumpkin_get_spawner;
extern int pumpkin_get_string_option;
extern int pumpkin_get_subdata;
extern int pumpkin_get_taskid;
extern int pumpkin_get_window;
extern int pumpkin_getchar;
extern int pumpkin_gets;
extern int pumpkin_getstr;
extern int pumpkin_gettable;
extern int pumpkin_global_finish;
extern int pumpkin_global_init;
extern int pumpkin_haschar;
extern int pumpkin_heap_alloc;
extern int pumpkin_heap_base;
extern int pumpkin_heap_dump;
extern int pumpkin_heap_dup;
extern int pumpkin_heap_free;
extern int pumpkin_heap_realloc;
extern int pumpkin_heap_size;
extern int pumpkin_http_abort;
extern int pumpkin_http_get;
extern int pumpkin_httpd_create;
extern int pumpkin_httpd_destroy;
extern int pumpkin_httpd_status;
extern int pumpkin_id2s;
extern int pumpkin_info_serial;
extern int pumpkin_init_misc;
extern int pumpkin_is_launched;
extern int pumpkin_is_m68k;
extern int pumpkin_is_paused;
extern int pumpkin_is_spawner;
extern int pumpkin_keymask;
extern int pumpkin_kill;
extern int pumpkin_launch;
extern int pumpkin_launcher;
extern int pumpkin_load_plugins;
extern int pumpkin_local_refresh;
extern int pumpkin_map_char;
extern int pumpkin_must_finish;
extern int pumpkin_next_char;
extern int pumpkin_num_serial;
extern int pumpkin_open_serial;
extern int pumpkin_pause;
extern int pumpkin_printf;
extern int pumpkin_ps;
extern int pumpkin_putchar;
extern int pumpkin_puts;
extern int pumpkin_refresh_desktop;
extern int pumpkin_reg_get;
extern int pumpkin_reg_set;
extern int pumpkin_s2id;
extern int pumpkin_save_bitmap;
extern int pumpkin_save_bmp;
extern int pumpkin_save_surface;
extern int pumpkin_screen_copy;
extern int pumpkin_screen_dirty;
extern int pumpkin_screen_lock;
extern int pumpkin_screen_unlock;
extern int pumpkin_script_call;
extern int pumpkin_script_create;
extern int pumpkin_script_create_obj;
extern int pumpkin_script_destroy;
extern int pumpkin_script_engine_id;
extern int pumpkin_script_get_last_error;
extern int pumpkin_script_global_function;
extern int pumpkin_script_global_function_data;
extern int pumpkin_script_global_iconst;
extern int pumpkin_script_global_iconst_value;
extern int pumpkin_script_global_pointer_value;
extern int pumpkin_script_init;
extern int pumpkin_script_obj_boolean;
extern int pumpkin_script_obj_function;
extern int pumpkin_script_obj_iconst;
extern int pumpkin_script_obj_sconst;
extern int pumpkin_script_run_file;
extern int pumpkin_script_run_string;
extern int pumpkin_send_deploy;
extern int pumpkin_set_abgr;
extern int pumpkin_set_battery;
extern int pumpkin_set_data;
extern int pumpkin_set_density;
extern int pumpkin_set_depth;
extern int pumpkin_set_finish;
extern int pumpkin_set_fullrefresh;
extern int pumpkin_set_lasterr;
extern int pumpkin_set_local_storage;
extern int pumpkin_set_lockable;
extern int pumpkin_set_m68k;
extern int pumpkin_set_mode;
extern int pumpkin_set_mono;
extern int pumpkin_set_native_keys;
extern int pumpkin_set_obj;
extern int pumpkin_set_osversion;
extern int pumpkin_set_preference;
extern int pumpkin_set_secure;
extern int pumpkin_set_size;
extern int pumpkin_set_spawner;
extern int pumpkin_set_subdata;
extern int pumpkin_set_taskbar;
extern int pumpkin_set_window;
extern int pumpkin_setcolor;
extern int pumpkin_setio;
extern int pumpkin_shader;
extern int pumpkin_status;
extern int pumpkin_sys_event;
extern int pumpkin_taskbar_add;
extern int pumpkin_taskbar_add_widget;
extern int pumpkin_taskbar_create;
extern int pumpkin_taskbar_destroy;
extern int pumpkin_taskbar_remove;
extern int pumpkin_taskbar_remove_widget;
extern int pumpkin_taskbar_ui;
extern int pumpkin_taskbar_update;
extern int pumpkin_test_exception;
extern int pumpkin_trace;
extern int pumpkin_vprintf;
extern int pumpkin_word_serial;
extern int pumpkin_write;
extern int pumpkin_system_call;
extern int pumpkin_deploy_files_session;
extern int pumpkin_deploy_from_image;
extern int pumpkin_registry_create;
extern int pumpkin_remove_locks;
extern int DmArchiveRecord;
extern int DmAttachRecord;
extern int DmAttachResource;
extern int DmCloseDatabase;
extern int DmCreateDatabase;
extern int DmCreateDatabaseEx;
extern int DmCreateDatabaseFromImage;
extern int DmDatabaseInfo;
extern int DmDatabaseProtect;
extern int DmDatabaseSize;
extern int DmDeleteCategory;
extern int DmDeleteDatabase;
extern int DmDeleteRecord;
extern int DmDetachRecord;
extern int DmDetachResource;
extern int DmFindDatabase;
extern int DmFindRecordByID;
extern int DmFindResource;
extern int DmFindResourceID;
extern int DmFindResourceType;
extern int DmFindSortPosition;
extern int DmFindSortPosition68K;
extern int DmFindSortPositionV10;
extern int DmGet1Resource;
extern int DmGetAppInfoID;
extern int DmGetDatabase;
extern int DmGetDatabaseLockState;
extern int DmGetLastErr;
extern int DmGetNextDatabaseByTypeCreator;
extern int DmGetRecord;
extern int DmGetResource;
extern int DmGetResourceDecoded;
extern int DmGetResourceIndex;
extern int DmInit;
extern int DmInsertionSort;
extern int DmInsertionSort68K;
extern int DmMoveCategory;
extern int DmMoveRecord;
extern int DmNewHandle;
extern int DmNewRecord;
extern int DmNewRecordEx;
extern int DmNewResource;
extern int DmNewResourceEx;
extern int DmNextOpenDatabase;
extern int DmNextOpenResDatabase;
extern int DmNumDatabases;
extern int DmNumRecords;
extern int DmNumRecordsInCategory;
extern int DmNumResources;
extern int DmOpenDBNoOverlay;
extern int DmOpenDatabase;
extern int DmOpenDatabaseByTypeCreator;
extern int DmOpenDatabaseInfo;
extern int DmPositionInCategory;
extern int DmQueryNextInCategory;
extern int DmQueryRecord;
extern int DmQuickSort;
extern int DmQuickSort68K;
extern int DmRecordInfo;
extern int DmReleaseRecord;
extern int DmReleaseResource;
extern int DmRemoveRecord;
extern int DmRemoveResource;
extern int DmRemoveSecretRecords;
extern int DmResetRecordStates;
extern int DmResizeRecord;
extern int DmResizeResource;
extern int DmResourceInfo;
extern int DmResourceLoadLib;
extern int DmResourceType;
extern int DmSearchRecord;
extern int DmSearchResource;
extern int DmSeekRecordInCategory;
extern int DmSet;
extern int DmSetDatabaseInfo;
extern int DmSetDirty;
extern int DmSetRecordInfo;
extern int DmSetResourceInfo;
extern int DmStrCopy;
extern int DmSync;
extern int DmSyncDatabase;
extern int DmWrite;
extern int DmWriteCheck;
extern int DmWriteOrCheck;
extern int FntSetAppearance;
extern int MemCardFormat;
extern int MemCardInfo;
extern int MemChunkFree;
extern int MemChunkNew;
extern int MemCmp;
extern int MemDebugMode;
extern int MemHandleCardNo;
extern int MemHandleDataStorage;
extern int MemHandleFlags;
extern int MemHandleFree;
extern int MemHandleHeapID;
extern int MemHandleLock;
extern int MemHandleLockCount;
extern int MemHandleLockEx;
extern int MemHandleNew;
extern int MemHandleOwner;
extern int MemHandleResetLock;
extern int MemHandleResize;
extern int MemHandleSetOwner;
extern int MemHandleSize;
extern int MemHandleToLocalID;
extern int MemHandleUnlock;
extern int MemHandleUnlockEx;
extern int MemHeapCheck;
extern int MemHeapCompact;
extern int MemHeapDynamic;
extern int MemHeapFlags;
extern int MemHeapFreeByOwnerID;
extern int MemHeapFreeBytes;
extern int MemHeapID;
extern int MemHeapInit;
extern int MemHeapScramble;
extern int MemHeapSize;
extern int MemInit;
extern int MemInitHeapTable;
extern int MemKernelInit;
extern int MemLocalIDKind;
extern int MemLocalIDToGlobal;
extern int MemLocalIDToHandle;
extern int MemLocalIDToLockedPtr;
extern int MemLocalIDToPtr;
extern int MemMove;
extern int MemNumCards;
extern int MemNumHeaps;
extern int MemNumRAMHeaps;
extern int MemPtrCardNo;
extern int MemPtrDataStorage;
extern int MemPtrFlags;
extern int MemPtrHeapID;
extern int MemPtrNew;
extern int MemPtrOwner;
extern int MemPtrRecoverHandle;
extern int MemPtrResetLock;
extern int MemPtrResize;
extern int MemPtrSetOwner;
extern int MemPtrSize;
extern int MemPtrToLocalID;
extern int MemPtrUnlock;
extern int MemSemaphoreRelease;
extern int MemSemaphoreReserve;
extern int MemSet;
extern int MemSetDebugMode;
extern int MemStoreInfo;
extern int MemStoreSetInfo;
extern int StoFileRead;
extern int StoFileSeek;
extern int StoFileWrite;
extern int StoFinish;
extern int StoHeapWalk;
extern int StoInit;
extern int StoNewDecodedResource;
extern int StoRefresh;
extern int SysCreateDataBaseList;
extern int SysCreateDataBaseList68K;
extern int VFSFileDBInfo;
extern int pumpkin_script_finish_env;
extern int pumpkin_script_init_env;
extern int pumpkin_script_main;
extern int AppSeedFill;
extern int AbtShowAbout;
extern int AbtShowAboutEx;
extern int AbtShowAboutPumpkin;
extern int AddrDBSort;
extern int AddrJDBSort;
extern int AlmAlarmCallback;
extern int AlmCancelAll;
extern int AlmDisplayAlarm;
extern int AlmEnableNotification;
extern int AlmGetAlarm;
extern int AlmInit;
extern int AlmSetAlarm;
extern int AlmTimeChange;
extern int AttnDoSpecialEffects;
extern int AttnForgetIt;
extern int AttnGetAttention;
extern int AttnGetCounts;
extern int AttnIndicatorEnable;
extern int AttnIndicatorEnabled;
extern int AttnIterate;
extern int AttnListOpen;
extern int AttnUpdate;
extern int BmpBitsSize;
extern int BmpColortableSize;
extern int BmpCompress;
extern int BmpConvertFrom16Bits;
extern int BmpConvertFrom1Bit;
extern int BmpConvertFrom24Bits;
extern int BmpConvertFrom2Bits;
extern int BmpConvertFrom4Bits;
extern int BmpConvertFrom8Bits;
extern int BmpCopyBit;
extern int BmpCreate;
extern int BmpCreate3;
extern int BmpCreateBitmapV3;
extern int BmpCreateSurface;
extern int BmpCreateSurfaceBitmap;
extern int BmpDecompressBitmap;
extern int BmpDecompressBitmapChain;
extern int BmpDelete;
extern int BmpDrawSurface;
extern int BmpExportFont;
extern int BmpFlip;
extern int BmpGetBestBitmap;
extern int BmpGetBestBitmapEx;
extern int BmpGetBitDepth;
extern int BmpGetBits;
extern int BmpGetColortable;
extern int BmpGetCompressionType;
extern int BmpGetDensity;
extern int BmpGetDescr;
extern int BmpGetDimensions;
extern int BmpGetLittleEndianBits;
extern int BmpGetNextBitmap;
extern int BmpGetNextBitmapAnyDensity;
extern int BmpGetNoDither;
extern int BmpGetPixel;
extern int BmpGetPixelRGB;
extern int BmpGetPixelValue;
extern int BmpGetSetCommonField;
extern int BmpGetSizes;
extern int BmpGetTransparentValue;
extern int BmpGetVersion;
extern int BmpIndexToRGB;
extern int BmpLittleEndian;
extern int BmpPutBit;
extern int BmpRGBToIndex;
extern int BmpRotate;
extern int BmpSetBitDepth;
extern int BmpSetDensity;
extern int BmpSetLittleEndianBits;
extern int BmpSetPixel;
extern int BmpSetTransparentValue;
extern int BmpSize;
extern int BmpV0GetSetField;
extern int BmpV1GetSetField;
extern int BmpV2GetSetField;
extern int BmpV3GetSetField;
extern int CharAttrFinishModule;
extern int CharAttrInitModule;
extern int GetCharCaselessValue;
extern int GetCharSortValue;
extern int CtbCompare;
extern int CtbGetEntry;
extern int CtbGetNumEntries;
extern int CtbSetEntry;
extern int CtbSetNumEntries;
extern int BtLibAddrAToBtd;
extern int BtLibCancelInquiry;
extern int BtLibClose;
extern int BtLibGetGeneralPreference;
extern int BtLibGetSelectedDevices;
extern int BtLibHandleEvent;
extern int BtLibHandleTransportEvent;
extern int BtLibLinkConnect;
extern int BtLibLinkDisconnect;
extern int BtLibLinkGetState;
extern int BtLibLinkSetState;
extern int BtLibOpen;
extern int BtLibPiconetCreate;
extern int BtLibPiconetDestroy;
extern int BtLibPiconetLockInbound;
extern int BtLibPiconetUnlockInbound;
extern int BtLibRegisterManagementNotification;
extern int BtLibSdpCompareUuids;
extern int BtLibSdpServiceRecordCreate;
extern int BtLibSdpServiceRecordDestroy;
extern int BtLibSdpServiceRecordStartAdvertising;
extern int BtLibSdpServiceRecordStopAdvertising;
extern int BtLibSecurityFindTrustedDeviceRecord;
extern int BtLibSecurityNumTrustedDeviceRecords;
extern int BtLibSecurityRemoveTrustedDeviceRecord;
extern int BtLibServiceClose;
extern int BtLibServiceIndicateSessionStart;
extern int BtLibServiceOpen;
extern int BtLibServicePlaySound;
extern int BtLibSetGeneralPreference;
extern int BtLibSleep;
extern int BtLibSocketAdvanceCredit;
extern int BtLibSocketClose;
extern int BtLibSocketConnect;
extern int BtLibSocketListen;
extern int BtLibSocketRespondToConnection;
extern int BtLibSocketSend;
extern int BtLibStartInquiry;
extern int BtLibUnregisterManagementNotification;
extern int BtLibWake;
extern int CategoryCreateList;
extern int CategoryCreateListV10;
extern int CategoryEdit;
extern int CategoryEditV10;
extern int CategoryEditV20;
extern int CategoryFind;
extern int CategoryFreeList;
extern int CategoryFreeListV10;
extern int CategoryGetName;
extern int CategoryGetNext;
extern int CategoryInitialize;
extern int CategorySelect;
extern int CategorySelectV10;
extern int CategorySetName;
extern int CategorySetTriggerLabel;
extern int CategoryTruncateName;
extern int ClipboardAddItem;
extern int ClipboardAppendItem;
extern int ClipboardGetItem;
extern int ClpFinishModule;
extern int ClpInitModule;
extern int CncAddProfile;
extern int CncDeleteProfile;
extern int CncGetProfileInfo;
extern int CncGetProfileList;
extern int CncProfileCloseDB;
extern int CncProfileCount;
extern int CncProfileCreate;
extern int CncProfileDelete;
extern int CncProfileGetCurrent;
extern int CncProfileGetIDFromIndex;
extern int CncProfileGetIDFromName;
extern int CncProfileGetIndex;
extern int CncProfileOpenDB;
extern int CncProfileSetCurrent;
extern int CncProfileSettingGet;
extern int CncProfileSettingSet;
extern int ConGetS;
extern int ConPutS;
extern int CtlDrawControl;
extern int CtlEnabled;
extern int CtlEraseControl;
extern int CtlGetGraphics;
extern int CtlGetLabel;
extern int CtlGetSliderValues;
extern int CtlGetValue;
extern int CtlHandleEvent;
extern int CtlHideControl;
extern int CtlHitControl;
extern int CtlNewControl;
extern int CtlNewGraphicControl;
extern int CtlNewSliderControl;
extern int CtlSetEnabled;
extern int CtlSetGraphics;
extern int CtlSetLabel;
extern int CtlSetSliderValues;
extern int CtlSetUsable;
extern int CtlSetValue;
extern int CtlShowControl;
extern int CtlUpdateCheckboxGroup;
extern int CtlUpdateGroup;
extern int CtlValidatePointer;
extern int CPMLibAddRandomSeed;
extern int CPMLibClose;
extern int CPMLibDecrypt;
extern int CPMLibDecryptFinal;
extern int CPMLibDecryptInit;
extern int CPMLibDecryptUpdate;
extern int CPMLibEncrypt;
extern int CPMLibEncryptFinal;
extern int CPMLibEncryptInit;
extern int CPMLibEncryptUpdate;
extern int CPMLibEnumerateProviders;
extern int CPMLibExportCipherInfo;
extern int CPMLibExportHashInfo;
extern int CPMLibExportKeyInfo;
extern int CPMLibExportVerifyInfo;
extern int CPMLibGenerateKey;
extern int CPMLibGenerateRandomBytes;
extern int CPMLibGetInfo;
extern int CPMLibGetProviderInfo;
extern int CPMLibHash;
extern int CPMLibHashFinal;
extern int CPMLibHashInit;
extern int CPMLibHashUpdate;
extern int CPMLibImportCipherInfo;
extern int CPMLibImportHashInfo;
extern int CPMLibImportKeyInfo;
extern int CPMLibImportVerifyInfo;
extern int CPMLibOpen;
extern int CPMLibReleaseCipherInfo;
extern int CPMLibReleaseHashInfo;
extern int CPMLibReleaseKeyInfo;
extern int CPMLibReleaseVerifyInfo;
extern int CPMLibSetDebugLevel;
extern int CPMLibSetDefaultProvider;
extern int CPMLibSleep;
extern int CPMLibTest;
extern int CPMLibVerify;
extern int CPMLibVerifyFinal;
extern int CPMLibVerifyInit;
extern int CPMLibVerifyUpdate;
extern int CPMLibWake;
extern int Crc16CalcBigBlock;
extern int Crc16CalcBlock;
extern int DateAdjust;
extern int DateDaysToDate;
extern int DateSecondsToDate;
extern int DateTemplateToAscii;
extern int DateToAscii;
extern int DateToDOWDMFormat;
extern int DateToDays;
extern int DayOfMonth;
extern int DayOfWeek;
extern int DaysInMonth;
extern int TimAdjust;
extern int TimDateTimeToSeconds;
extern int TimSecondsToDateTime;
extern int TimTimeZoneToUTC;
extern int TimUTCToTimeZone;
extern int TimeToAscii;
extern int TimeZoneToAscii;
extern int DayDrawDaySelector;
extern int DayDrawDays;
extern int DayHandleEvent;
extern int DbgBreak;
extern int DbgCommSettings;
extern int DbgGetMessage;
extern int DbgInit;
extern int DbgMessage;
extern int DbgSrcBreak;
extern int DbgSrcMessage;
extern int DlkControl;
extern int DlkDispatchRequest;
extern int DlkGetSyncInfo;
extern int DlkSetLogEntry;
extern int DlkStartServer;
extern int EncDES;
extern int EncDigestMD4;
extern int EncDigestMD5;
extern int md5Finalize;
extern int md5Init;
extern int md5Step;
extern int md5Update;
extern int rotateLeft;
extern int SHA1;
extern int SHA1Final;
extern int SHA1Init;
extern int SHA1Transform;
extern int SHA1Update;
extern int ErrAlertCustom;
extern int ErrDisplayFileLineMsg;
extern int ErrDisplayFileLineMsgEx;
extern int ErrExceptionList;
extern int ErrThrow;
extern int EvtAddEventToQueue;
extern int EvtAddUniqueEventToQueue;
extern int EvtCopyEvent;
extern int EvtEmptyQueue;
extern int EvtEnqueueKey;
extern int EvtEventAvail;
extern int EvtFinishModule;
extern int EvtFlushPenQueue;
extern int EvtGetEvent;
extern int EvtGetEventName;
extern int EvtGetEventUs;
extern int EvtGetPen;
extern int EvtGetPenEx;
extern int EvtInitModule;
extern int EvtKeyEventAvail;
extern int EvtPrintEvent;
extern int EvtPumpEvents;
extern int EvtReturnPenMove;
extern int EvtSetNullEventTick;
extern int EvtSysEventAvail;
extern int ExgLibAccept;
extern int ExgLibClose;
extern int ExgLibConnect;
extern int ExgLibControl;
extern int ExgLibDisconnect;
extern int ExgLibGet;
extern int ExgLibHandleEvent;
extern int ExgLibOpen;
extern int ExgLibPut;
extern int ExgLibReceive;
extern int ExgLibRequest;
extern int ExgLibSend;
extern int ExgLibSleep;
extern int ExgLibWake;
extern int ExgAccept;
extern int ExgConnect;
extern int ExgControl;
extern int ExgDBRead;
extern int ExgDBWrite;
extern int ExgDisconnect;
extern int ExgDoDialog;
extern int ExgGet;
extern int ExgGetDefaultApplication;
extern int ExgGetRegisteredApplications;
extern int ExgGetRegisteredTypes;
extern int ExgGetTargetApplication;
extern int ExgInit;
extern int ExgNotifyGoto;
extern int ExgNotifyPreview;
extern int ExgNotifyReceive;
extern int ExgNotifyReceiveV35;
extern int ExgPut;
extern int ExgReceive;
extern int ExgRegisterData;
extern int ExgRegisterDatatype;
extern int ExgRequest;
extern int ExgSend;
extern int ExgSetDefaultApplication;
extern int ExpCardGetSerialPort;
extern int ExpCardInfo;
extern int ExpCardInserted;
extern int ExpCardPresent;
extern int ExpCardRemoved;
extern int ExpInit;
extern int ExpSlotDriverInstall;
extern int ExpSlotDriverRemove;
extern int ExpSlotEnumerate;
extern int ExpSlotLibFind;
extern int ExpSlotRegister;
extern int ExpSlotUnregister;
extern int SysFatalAlert;
extern int SysFatalAlertFinish;
extern int SysFatalAlertInit;
extern int FtrFinishModule;
extern int FtrGet;
extern int FtrGetByIndex;
extern int FtrGetPtr;
extern int FtrInit;
extern int FtrInitModule;
extern int FtrPtrFree;
extern int FtrPtrNew;
extern int FtrPtrResize;
extern int FtrSet;
extern int FtrUnregister;
extern int FldCalcFieldHeight;
extern int FldCompactText;
extern int FldCopy;
extern int FldCut;
extern int FldDelete;
extern int FldDirty;
extern int FldDrawField;
extern int FldEraseField;
extern int FldFinishModule;
extern int FldFreeMemory;
extern int FldGetActiveField;
extern int FldGetAttributes;
extern int FldGetBounds;
extern int FldGetFont;
extern int FldGetInsPtPosition;
extern int FldGetMaxChars;
extern int FldGetNumberOfBlankLines;
extern int FldGetScrollPosition;
extern int FldGetScrollValues;
extern int FldGetSelection;
extern int FldGetTextAllocatedSize;
extern int FldGetTextHandle;
extern int FldGetTextHeight;
extern int FldGetTextLength;
extern int FldGetTextPtr;
extern int FldGetVisibleLines;
extern int FldGrabFocus;
extern int FldHandleEvent;
extern int FldInitModule;
extern int FldInsert;
extern int FldMakeFullyVisible;
extern int FldNewField;
extern int FldPaste;
extern int FldRecalculateField;
extern int FldReinitModule;
extern int FldReleaseFocus;
extern int FldReplaceText;
extern int FldScrollField;
extern int FldScrollable;
extern int FldSendChangeNotification;
extern int FldSendHeightChangeNotification;
extern int FldSetActiveField;
extern int FldSetAttributes;
extern int FldSetBounds;
extern int FldSetDirty;
extern int FldSetFont;
extern int FldSetInsPtPosition;
extern int FldSetInsertionPoint;
extern int FldSetMaxChars;
extern int FldSetMaxVisibleLines;
extern int FldSetScrollPosition;
extern int FldSetSelection;
extern int FldSetText;
extern int FldSetTextAllocatedSize;
extern int FldSetTextHandle;
extern int FldSetTextPtr;
extern int FldSetUsable;
extern int FldUndo;
extern int FldWordWrap;
extern int FileClose;
extern int FileControl;
extern int FileDelete;
extern int FileOpen;
extern int FileReadLow;
extern int FileSeek;
extern int FileTell;
extern int FileTruncate;
extern int FileWrite;
extern int Find;
extern int FindDrawHeader;
extern int FindGetLineBounds;
extern int FindSaveMatch;
extern int FindStrInStr;
extern int ECFixedDiv;
extern int ECFixedMul;
extern int FlpAToF;
extern int FlpBase10Info;
extern int FlpBufferAToF;
extern int FlpCorrectedAdd;
extern int FlpCorrectedSub;
extern int FlpFToA;
extern int FlpSelectorErrPrv;
extern int FlpVersion;
extern int _d_add;
extern int _d_div;
extern int _d_dtof;
extern int _d_dtoi;
extern int _d_dtou;
extern int _d_feq;
extern int _d_fge;
extern int _d_fgt;
extern int _d_fle;
extern int _d_flt;
extern int _d_fne;
extern int _d_itod;
extern int _d_mul;
extern int _d_neg;
extern int _d_sub;
extern int _d_utod;
extern int _f_add;
extern int _f_div;
extern int _f_feq;
extern int _f_fge;
extern int _f_fgt;
extern int _f_fle;
extern int _f_flt;
extern int _f_fne;
extern int _f_ftod;
extern int _f_ftoi;
extern int _f_ftou;
extern int _f_itof;
extern int _f_mul;
extern int _f_neg;
extern int _f_sub;
extern int _f_utof;
extern int _fp_get_fpscr;
extern int _fp_round;
extern int _fp_set_fpscr;
extern int FntAverageCharWidth;
extern int FntBaseLine;
extern int FntCharHeight;
extern int FntCharWidth;
extern int FntCharsInWidth;
extern int FntCharsWidth;
extern int FntCopyFont;
extern int FntDefineFont;
extern int FntDescenderHeight;
extern int FntDrawChar;
extern int FntFinishModule;
extern int FntFontCharWidth;
extern int FntFreeFont;
extern int FntGetDensity;
extern int FntGetDensityCount;
extern int FntGetFont;
extern int FntGetFontPtr;
extern int FntGetScrollValues;
extern int FntGetVersion;
extern int FntInitModule;
extern int FntLineHeight;
extern int FntLineWidth;
extern int FntLoadFont;
extern int FntReinitModule;
extern int FntSaveFont;
extern int FntSaveFontEx;
extern int FntSaveFonts;
extern int FntSetFont;
extern int FntWCharWidth;
extern int FntWidthToOffset;
extern int FntWordWrap;
extern int FntWordWrapReverseNLines;
extern int pumpkin_create_font;
extern int pumpkin_create_fontv2;
extern int pumpkin_destroy_font;
extern int pumpkin_destroy_fontv2;
extern int FontSelect;
extern int FrmActiveState;
extern int FrmAddSpaceForObject;
extern int FrmAlert;
extern int FrmCenterDialogs;
extern int FrmCloseAllForms;
extern int FrmCopyLabel;
extern int FrmCopyTitle;
extern int FrmCustomAlert;
extern int FrmCustomResponseAlert;
extern int FrmDeleteForm;
extern int FrmDispatchEvent;
extern int FrmDoDialog;
extern int FrmDoDialogEx;
extern int FrmDrawEmptyDialog;
extern int FrmDrawForm;
extern int FrmDrawObject;
extern int FrmEraseForm;
extern int FrmEraseObject;
extern int FrmFinishModule;
extern int FrmGetActiveField;
extern int FrmGetActiveForm;
extern int FrmGetActiveFormID;
extern int FrmGetCenterDialogs;
extern int FrmGetControlGroupSelection;
extern int FrmGetControlValue;
extern int FrmGetDIAPolicyAttr;
extern int FrmGetFirstForm;
extern int FrmGetFocus;
extern int FrmGetFormBounds;
extern int FrmGetFormId;
extern int FrmGetFormPtr;
extern int FrmGetGadgetData;
extern int FrmGetLabel;
extern int FrmGetNumberOfObjects;
extern int FrmGetObjectBounds;
extern int FrmGetObjectId;
extern int FrmGetObjectIndex;
extern int FrmGetObjectIndexFromPtr;
extern int FrmGetObjectPosition;
extern int FrmGetObjectPtr;
extern int FrmGetObjectType;
extern int FrmGetTitle;
extern int FrmGetUsable;
extern int FrmGetUserModifiedState;
extern int FrmGetVisible;
extern int FrmGetWindowHandle;
extern int FrmGotoForm;
extern int FrmHandleEvent;
extern int FrmHelp;
extern int FrmHideObject;
extern int FrmInitForm;
extern int FrmInitModule;
extern int FrmNavObjectTakeFocus;
extern int FrmNewBitmap;
extern int FrmNewForm;
extern int FrmNewGadget;
extern int FrmNewGsi;
extern int FrmNewLabel;
extern int FrmObjectBottomAlign;
extern int FrmObjectRightAlign;
extern int FrmPointInTitle;
extern int FrmPopupForm;
extern int FrmReinitModule;
extern int FrmRemoveObject;
extern int FrmReturnToForm;
extern int FrmSaveAllForms;
extern int FrmSetActiveForm;
extern int FrmSetCategoryLabel;
extern int FrmSetColorTrigger;
extern int FrmSetControlGroupSelection;
extern int FrmSetControlValue;
extern int FrmSetDIAPolicyAttr;
extern int FrmSetEventHandler;
extern int FrmSetFocus;
extern int FrmSetFormBounds;
extern int FrmSetGadgetData;
extern int FrmSetGadgetHandler;
extern int FrmSetMenu;
extern int FrmSetNotUserModified;
extern int FrmSetObjectBounds;
extern int FrmSetObjectPosition;
extern int FrmSetObjectPtr;
extern int FrmSetTitle;
extern int FrmSetUsable;
extern int FrmSetVisible;
extern int FrmShowObject;
extern int FrmTrackPenUp;
extern int FrmUpdateForm;
extern int FrmUpdateScrollers;
extern int FrmValidatePtr;
extern int FrmVisible;
extern int pumpkin_create_alert;
extern int pumpkin_create_form;
extern int pumpkin_destroy_alert;
extern int pumpkin_destroy_form;
extern int pumpkin_fix_popups;
extern int FSDirCreate;
extern int FSDirEntryEnumerate;
extern int FSFileClose;
extern int FSFileCreate;
extern int FSFileDelete;
extern int FSFileEOF;
extern int FSFileGetAttributes;
extern int FSFileGetDate;
extern int FSFileRename;
extern int FSFileResize;
extern int FSFileSeek;
extern int FSFileSetAttributes;
extern int FSFileSetDate;
extern int FSFileSize;
extern int FSFileTell;
extern int FSFilesystemType;
extern int FSLibAPIVersion;
extern int FSLibClose;
extern int FSLibOpen;
extern int FSLibSleep;
extern int FSLibWake;
extern int FSVolumeFormat;
extern int FSVolumeGetLabel;
extern int FSVolumeInfo;
extern int FSVolumeMount;
extern int FSVolumeSetLabel;
extern int FSVolumeSize;
extern int FSVolumeUnmount;
extern int GrfAddMacro;
extern int GrfAddPoint;
extern int GrfBeginStroke;
extern int GrfCleanState;
extern int GrfDeleteMacro;
extern int GrfFieldChange;
extern int GrfFilterPoints;
extern int GrfFindBranch;
extern int GrfFlushPoints;
extern int GrfFree;
extern int GrfGetAndExpandMacro;
extern int GrfGetGlyphMapping;
extern int GrfGetMacro;
extern int GrfGetMacroName;
extern int GrfGetNumPoints;
extern int GrfGetPoint;
extern int GrfGetState;
extern int GrfInit;
extern int GrfInitState;
extern int GrfMatch;
extern int GrfMatchGlyph;
extern int GrfProcessStroke;
extern int GrfSetState;
extern int SysGraffitiReferenceDialog;
extern int GsiEnable;
extern int GsiEnabled;
extern int GsiInitialize;
extern int GsiSetLocation;
extern int GsiSetShiftState;
extern int HwrCustom;
extern int HostAscTime;
extern int HostCTime;
extern int HostClock;
extern int HostCloseDir;
extern int HostDbgClearDataBreak;
extern int HostDbgSetDataBreak;
extern int HostErrNo;
extern int HostExgLibAccept;
extern int HostExgLibClose;
extern int HostExgLibConnect;
extern int HostExgLibControl;
extern int HostExgLibDisconnect;
extern int HostExgLibGet;
extern int HostExgLibHandleEvent;
extern int HostExgLibOpen;
extern int HostExgLibPut;
extern int HostExgLibReceive;
extern int HostExgLibRequest;
extern int HostExgLibSend;
extern int HostExgLibSleep;
extern int HostExgLibWake;
extern int HostExportFile;
extern int HostFClose;
extern int HostFEOF;
extern int HostFError;
extern int HostFFlush;
extern int HostFGetC;
extern int HostFGetPos;
extern int HostFGetS;
extern int HostFOpen;
extern int HostFPrintF;
extern int HostFPutC;
extern int HostFPutS;
extern int HostFRead;
extern int HostFReopen;
extern int HostFScanF;
extern int HostFSeek;
extern int HostFSetPos;
extern int HostFTell;
extern int HostFWrite;
extern int HostFree;
extern int HostGMTime;
extern int HostGestalt;
extern int HostGetDirectory;
extern int HostGetEnv;
extern int HostGetFile;
extern int HostGetFileAttr;
extern int HostGetHostID;
extern int HostGetHostPlatform;
extern int HostGetHostVersion;
extern int HostGetPreference;
extern int HostGremlinCounter;
extern int HostGremlinIsRunning;
extern int HostGremlinLimit;
extern int HostGremlinNew;
extern int HostGremlinNumber;
extern int HostImportFile;
extern int HostImportFileWithID;
extern int HostIsCallingTrap;
extern int HostIsSelectorImplemented;
extern int HostLocalTime;
extern int HostLogFile;
extern int HostMalloc;
extern int HostMkDir;
extern int HostMkTime;
extern int HostOpenDir;
extern int HostProfileCleanup;
extern int HostProfileDetailFn;
extern int HostProfileDump;
extern int HostProfileGetCycles;
extern int HostProfileInit;
extern int HostProfileStart;
extern int HostProfileStop;
extern int HostPutFile;
extern int HostReadDir;
extern int HostRealloc;
extern int HostRemove;
extern int HostRename;
extern int HostRmDir;
extern int HostSaveScreen;
extern int HostSessionClose;
extern int HostSessionCreate;
extern int HostSessionOpen;
extern int HostSessionQuit;
extern int HostSessionSave;
extern int HostSetFileAttr;
extern int HostSetLogFileSize;
extern int HostSetPreference;
extern int HostSignalResume;
extern int HostSignalSend;
extern int HostSignalWait;
extern int HostSlotHasCard;
extern int HostSlotMax;
extern int HostSlotRoot;
extern int HostStat;
extern int HostStrFTime;
extern int HostTime;
extern int HostTmpFile;
extern int HostTmpNam;
extern int HostTraceClose;
extern int HostTraceInit;
extern int HostTraceOutputB;
extern int HostTraceOutputT;
extern int HostTraceOutputTL;
extern int HostTraceOutputVT;
extern int HostTraceOutputVTL;
extern int HostTruncate;
extern int HostUTime;
extern int ImcStringIsAscii;
extern int INetLibCachePurge;
extern int INetLibCheckAntennaState;
extern int INetLibClose;
extern int INetLibConfigDelete;
extern int INetLibPrepareCacheForHistory;
extern int INetLibSleep;
extern int INetLibSockClose;
extern int INetLibURLCrack;
extern int INetLibURLsCompare;
extern int INetLibWake;
extern int INetLibWiCmd;
extern int INetLibWirelessIndicatorCmd;
extern int InsPtCheckBlink;
extern int InsPtEnable;
extern int InsPtEnabled;
extern int InsPtFinishModule;
extern int InsPtGetHeight;
extern int InsPtGetLocation;
extern int InsPtInitModule;
extern int InsPtInitialize;
extern int InsPtSetHeight;
extern int InsPtSetLocation;
extern int IntlGetRoutineAddress;
extern int IntlSetRoutineAddress;
extern int IrBind;
extern int IrClose;
extern int IrConnectIrLap;
extern int IrConnectReq;
extern int IrConnectRsp;
extern int IrDataReq;
extern int IrDisconnectIrLap;
extern int IrDiscoverReq;
extern int IrHandleEvent;
extern int IrIsIrLapConnected;
extern int IrIsMediaBusy;
extern int IrIsNoProgress;
extern int IrIsRemoteBusy;
extern int IrLocalBusy;
extern int IrMaxRxSize;
extern int IrMaxTxSize;
extern int IrOpen;
extern int IrSetDeviceInfo;
extern int IrTestReq;
extern int IrUnbind;
extern int IrWaitForEvent;
extern int KbdDraw;
extern int KbdDrawKeyboard;
extern int KbdErase;
extern int KbdGetLayout;
extern int KbdGetPosition;
extern int KbdGetShiftState;
extern int KbdGrfGetState;
extern int KbdGrfSetState;
extern int KbdHandleEvent;
extern int KbdSetLayout;
extern int KbdSetPosition;
extern int KbdSetShiftState;
extern int KeyboardFinishModule;
extern int KeyboardInitModule;
extern int KeyboardStatusFree;
extern int KeyboardStatusNew;
extern int SysKeyboardDialog;
extern int SysKeyboardDialogV10;
extern int KeyCurrentState;
extern int KeyFinishModule;
extern int KeyInitModule;
extern int KeyRates;
extern int KeySetMask;
extern int SysAppLauncherDialog;
extern int LstDrawList;
extern int LstEraseList;
extern int LstFreeListChoices;
extern int LstGetNumberOfItems;
extern int LstGetSelection;
extern int LstGetSelectionText;
extern int LstGetTopItem;
extern int LstGetVisibleItems;
extern int LstHandleEvent;
extern int LstMakeItemVisible;
extern int LstNewList;
extern int LstNewListEx;
extern int LstPopupList;
extern int LstScrollList;
extern int LstSetDrawFunction;
extern int LstSetHeight;
extern int LstSetListChoices;
extern int LstSetPosition;
extern int LstSetSelection;
extern int LstSetTopItem;
extern int LmGetLocaleSetting;
extern int LmGetNumLocales;
extern int LmLocaleToIndex;
extern int LmTimeZoneToIndex;
extern int LocGetNumberSeparators;
extern int Lz77LibClose;
extern int Lz77LibOpen;
extern int Lz77LibSleep;
extern int Lz77LibWake;
extern int MenuAddItem;
extern int MenuCmdBarAddButton;
extern int MenuCmdBarDisplay;
extern int MenuCmdBarGetButtonData;
extern int MenuDispose;
extern int MenuDrawMenu;
extern int MenuEraseStatus;
extern int MenuFinishModule;
extern int MenuGetActiveMenu;
extern int MenuHandleEvent;
extern int MenuHideItem;
extern int MenuInit;
extern int MenuInitModule;
extern int MenuReinitModule;
extern int MenuSetActiveMenu;
extern int MenuSetActiveMenuRscID;
extern int MenuShowItem;
extern int pumpkin_create_menu;
extern int pumpkin_destroy_menu;
extern int MdmDial;
extern int MdmHangUp;
extern int NetLibBitGetFixed;
extern int NetLibBitGetIntV;
extern int NetLibBitGetUIntV;
extern int NetLibBitMove;
extern int NetLibBitPutFixed;
extern int NetLibBitPutIntV;
extern int NetLibBitPutUIntV;
extern int NetLibAddrAToIN;
extern int NetLibAddrINToA;
extern int NetLibClose;
extern int NetLibConfigAliasGet;
extern int NetLibConfigAliasSet;
extern int NetLibConfigDelete;
extern int NetLibConfigIndexFromName;
extern int NetLibConfigList;
extern int NetLibConfigMakeActive;
extern int NetLibConfigRename;
extern int NetLibConfigSaveAs;
extern int NetLibConnectionRefresh;
extern int NetLibDmReceive;
extern int NetLibFinishCloseWait;
extern int NetLibGetHostByAddr;
extern int NetLibGetHostByName;
extern int NetLibGetMailExchangeByName;
extern int NetLibGetServByName;
extern int NetLibHandlePowerOff;
extern int NetLibIFAttach;
extern int NetLibIFDetach;
extern int NetLibIFDown;
extern int NetLibIFGet;
extern int NetLibIFSettingGet;
extern int NetLibIFSettingSet;
extern int NetLibIFUp;
extern int NetLibMaster;
extern int NetLibOpen;
extern int NetLibOpenConfig;
extern int NetLibOpenCount;
extern int NetLibOpenIfCloseWait;
extern int NetLibReceive;
extern int NetLibReceivePB;
extern int NetLibSelect;
extern int NetLibSend;
extern int NetLibSendPB;
extern int NetLibSettingGet;
extern int NetLibSettingSet;
extern int NetLibSleep;
extern int NetLibSocketAccept;
extern int NetLibSocketAddr;
extern int NetLibSocketBind;
extern int NetLibSocketClose;
extern int NetLibSocketConnect;
extern int NetLibSocketListen;
extern int NetLibSocketOpen;
extern int NetLibSocketOptionGet;
extern int NetLibSocketOptionSet;
extern int NetLibSocketShutdown;
extern int NetLibTracePrintF;
extern int NetLibTracePutS;
extern int NetLibWake;
extern int NetUReadN;
extern int NetUTCPOpen;
extern int NetUWriteN;
extern int netFDClr;
extern int netFDIsSet;
extern int netFDSet;
extern int netFDZero;
extern int OmGetCurrentLocale;
extern int OmGetIndexedLocale;
extern int OmGetNextSystemLocale;
extern int OmGetRoutineAddress;
extern int OmGetSystemLocale;
extern int OmLocaleToOverlayDBName;
extern int OmOverlayDBNameToLocale;
extern int OmSetSystemLocale;
extern int pumpkin_create_overlay;
extern int pumpkin_destroy_overlay;
extern int PwdExists;
extern int PwdRemove;
extern int PwdSet;
extern int PwdVerify;
extern int PceNativeCall;
extern int PdiDefineReaderDictionary;
extern int PdiDefineResizing;
extern int PdiDefineWriterDictionary;
extern int PdiEnterObject;
extern int PdiLibClose;
extern int PdiLibOpen;
extern int PdiReadParameter;
extern int PdiReadProperty;
extern int PdiReadPropertyField;
extern int PdiReadPropertyName;
extern int PdiReaderDelete;
extern int PdiReaderNew;
extern int PdiSetCharset;
extern int PdiSetEncoding;
extern int PdiWriteBeginObject;
extern int PdiWriteParameter;
extern int PdiWriteParameterStr;
extern int PdiWriteProperty;
extern int PdiWritePropertyBinaryValue;
extern int PdiWritePropertyFields;
extern int PdiWritePropertyStr;
extern int PdiWritePropertyValue;
extern int PdiWriterDelete;
extern int PdiWriterNew;
extern int PINGetInputAreaState;
extern int PINGetInputTriggerState;
extern int PINSetInputAreaState;
extern int PINSetInputTriggerState;
extern int StatGetAttribute;
extern int StatHide;
extern int StatShow;
extern int PenCalibrate;
extern int PenClose;
extern int PenGetRawPen;
extern int PenOpen;
extern int PenRawToScreen;
extern int PenResetCalibration;
extern int PenScreenToRaw;
extern int PenSleep;
extern int PenWake;
extern int PhoneNumberLookup;
extern int PhoneNumberLookupCustom;
extern int PrefCountryCode;
extern int PrefCountryName;
extern int PrefFinishModule;
extern int PrefGetAppPreferences;
extern int PrefGetAppPreferencesV10;
extern int PrefGetPreference;
extern int PrefGetPreferences;
extern int PrefInitModule;
extern int PrefLanguageCode;
extern int PrefLanguageName;
extern int PrefOpenPreferenceDB;
extern int PrefOpenPreferenceDBV10;
extern int PrefSetAppPreferences;
extern int PrefSetAppPreferencesV10;
extern int PrefSetPreference;
extern int PrefSetPreferences;
extern int SecSelectViewStatus;
extern int SecVerifyPW;
extern int PrgHandleEvent;
extern int PrgStartDialog;
extern int PrgStartDialogV31;
extern int PrgStopDialog;
extern int PrgUpdateDialog;
extern int RctAbsToRect;
extern int RctCopyRectangle;
extern int RctGetDifference;
extern int RctGetIntersection;
extern int RctGetUnion;
extern int RctInsetRectangle;
extern int RctOffsetRectangle;
extern int RctPtInRectangle;
extern int RctRectToAbs;
extern int RctSetRectangle;
extern int SclDrawScrollBar;
extern int SclGetScrollBar;
extern int SclHandleEvent;
extern int SclSetScrollBar;
extern int SelTimeFinishModule;
extern int SelTimeInitModule;
extern int SelectOneTime;
extern int SelectTime;
extern int SelectTimeV33;
extern int SelectDay;
extern int SelectDayV10;
extern int SelectTimeZone;
extern int SlkClose;
extern int SlkCloseSocket;
extern int SlkFlushSocket;
extern int SlkOpen;
extern int SlkOpenSocket;
extern int SlkProcessRPC;
extern int SlkReceivePacket;
extern int SlkSendPacket;
extern int SlkSetSocketListener;
extern int SlkSocketPortID;
extern int SlkSocketSetTimeout;
extern int SlkSysPktDefaultResponse;
extern int SerialMgrInstall;
extern int SrmClearErr;
extern int SrmClose;
extern int SrmControl;
extern int SrmCustomControl;
extern int SrmExtOpen;
extern int SrmExtOpenBackground;
extern int SrmFinishModule;
extern int SrmGetDeviceCount;
extern int SrmGetDeviceInfo;
extern int SrmGetStatus;
extern int SrmInitModule;
extern int SrmOpen;
extern int SrmOpenBackground;
extern int SrmPrimeWakeupHandler;
extern int SrmReceive;
extern int SrmReceiveCheck;
extern int SrmReceiveFlush;
extern int SrmReceiveWait;
extern int SrmReceiveWindowClose;
extern int SrmReceiveWindowOpen;
extern int SrmSend;
extern int SrmSendCheck;
extern int SrmSendFlush;
extern int SrmSendWait;
extern int SrmSetReceiveBuffer;
extern int SrmSetWakeupHandler;
extern int SrmSleep;
extern int SrmWake;
extern int SerClearErr;
extern int SerClose;
extern int SerControl;
extern int SerGetSettings;
extern int SerGetStatus;
extern int SerOpen;
extern int SerPrimeWakeupHandler;
extern int SerReceive;
extern int SerReceive10;
extern int SerReceiveCheck;
extern int SerReceiveFlush;
extern int SerReceiveISP;
extern int SerReceiveWait;
extern int SerReceiveWindowClose;
extern int SerReceiveWindowOpen;
extern int SerSend;
extern int SerSend10;
extern int SerSendCheck;
extern int SerSendFlush;
extern int SerSendWait;
extern int SerSetReceiveBuffer;
extern int SerSetSettings;
extern int SerSleep;
extern int SerWake;
extern int DrvClose;
extern int DrvReadChar;
extern int DrvStatus;
extern int DrvWriteChar;
extern int VDrvClose;
extern int VDrvOpen;
extern int VDrvStatus;
extern int SlotCardGetSerialPort;
extern int SlotCardInfo;
extern int SlotCardIsFilesystemSupported;
extern int SlotCardLowLevelFormat;
extern int SlotCardMediaType;
extern int SlotCardMetrics;
extern int SlotCardPresent;
extern int SlotCardRelease;
extern int SlotCardReserve;
extern int SlotClose;
extern int SlotLibAPIVersion;
extern int SlotMediaType;
extern int SlotOpen;
extern int SlotSleep;
extern int SlotWake;
extern int SndCreateMidiList;
extern int SndDoCmd;
extern int SndFinishModule;
extern int SndGetDefaultVolume;
extern int SndInit;
extern int SndInitModule;
extern int SndInterruptSmfIrregardless;
extern int SndPlayFile;
extern int SndPlayResource;
extern int SndPlaySmf;
extern int SndPlaySmfIrregardless;
extern int SndPlaySmfResource;
extern int SndPlaySmfResourceIrregardless;
extern int SndPlaySystemSound;
extern int SndSetDefaultVolume;
extern int SndStreamCreate;
extern int SndStreamCreateEx;
extern int SndStreamCreateExtended;
extern int SndStreamDelete;
extern int SndStreamDeviceControl;
extern int SndStreamGetPan;
extern int SndStreamGetVolume;
extern int SndStreamPause;
extern int SndStreamSetPan;
extern int SndStreamSetVolume;
extern int SndStreamStart;
extern int SndStreamStop;
extern int putSample;
extern int SslClose;
extern int SslConsume;
extern int SslContextCreate;
extern int SslContextDestroy;
extern int SslContextGetLong;
extern int SslContextGetPtr;
extern int SslContextSetLong;
extern int SslContextSetPtr;
extern int SslFlush;
extern int SslLibClose;
extern int SslLibCreate;
extern int SslLibDestroy;
extern int SslLibGetLong;
extern int SslLibGetPtr;
extern int SslLibName;
extern int SslLibOpen;
extern int SslLibSetLong;
extern int SslLibSetPtr;
extern int SslLibSleep;
extern int SslLibWake;
extern int SslOpen;
extern int StrAToI;
extern int StrCaselessCompare;
extern int StrCat;
extern int StrChr;
extern int StrCompare;
extern int StrCompareAscii;
extern int StrCopy;
extern int StrDelocalizeNumber;
extern int StrDup;
extern int StrIToA;
extern int StrIToH;
extern int StrLen;
extern int StrLocalizeNumber;
extern int StrNCaselessCompare;
extern int StrNCat;
extern int StrNCompare;
extern int StrNCompareAscii;
extern int StrNCopy;
extern int StrNPrintF;
extern int StrPrintF;
extern int StrStr;
extern int StrToLower;
extern int StrVNPrintF;
extern int StrVPrintF;
extern int EvtDequeueKeyEvent;
extern int EvtDequeuePenPoint;
extern int EvtDequeuePenStrokeInfo;
extern int EvtEnableGraffiti;
extern int EvtEnqueuePenPoint;
extern int EvtFlushKeyQueue;
extern int EvtFlushNextPenStroke;
extern int EvtGetPenBtnList;
extern int EvtGetSilkscreenAreaList;
extern int EvtGetSysEvent;
extern int EvtKeyQueueEmpty;
extern int EvtKeyQueueSize;
extern int EvtPenQueueSize;
extern int EvtProcessSoftKeyStroke;
extern int EvtResetAutoOffTimer;
extern int EvtSetAutoOffTimer;
extern int EvtSetKeyQueuePtr;
extern int EvtSetPenQueuePtr;
extern int EvtSysInit;
extern int EvtWakeup;
extern int EvtWakeupWithoutNilEvent;
extern int HwrGetROMToken;
extern int SysAppExit;
extern int SysAppStartup;
extern int SysBatteryDialog;
extern int SysBatteryInfo;
extern int SysBatteryInfoV20;
extern int SysBroadcastActionCode;
extern int SysColdBoot;
extern int SysCreatePanelList;
extern int SysCurAppDatabase;
extern int SysDisableInts;
extern int SysDoze;
extern int SysEvGroupCreate;
extern int SysEvGroupRead;
extern int SysEvGroupSignal;
extern int SysEvGroupWait;
extern int SysFinishModule;
extern int SysGetOSVersionString;
extern int SysGetOrientation;
extern int SysGetOrientationTriggerState;
extern int SysGetROMToken;
extern int SysGetStackInfo;
extern int SysGetTrapAddress;
extern int SysHandleEvent;
extern int SysInit;
extern int SysInitModule;
extern int SysKernelInfo;
extern int SysLCDBrightness;
extern int SysLCDContrast;
extern int SysLaunchConsole;
extern int SysLibClose;
extern int SysLibFind;
extern int SysLibInstall;
extern int SysLibLoad;
extern int SysLibOpen;
extern int SysLibRemove;
extern int SysLibSleep;
extern int SysLibTblEntry;
extern int SysLibWake;
extern int SysMailboxCreate;
extern int SysMailboxDelete;
extern int SysMailboxFlush;
extern int SysMailboxSend;
extern int SysMailboxWait;
extern int SysNewOwnerID;
extern int SysResSemaphoreCreate;
extern int SysResSemaphoreDelete;
extern int SysResSemaphoreRelease;
extern int SysResSemaphoreReserve;
extern int SysReset;
extern int SysRestoreStatus;
extern int SysSemaphoreCreate;
extern int SysSemaphoreDelete;
extern int SysSemaphoreSet;
extern int SysSemaphoreSignal;
extern int SysSemaphoreWait;
extern int SysSetA5;
extern int SysSetAutoOffTime;
extern int SysSetOrientation;
extern int SysSetOrientationTriggerState;
extern int SysSetPerformance;
extern int SysSetTrapAddress;
extern int SysSleep;
extern int SysTaskCreate;
extern int SysTaskDelay;
extern int SysTaskDelayMs;
extern int SysTaskDelete;
extern int SysTaskID;
extern int SysTaskResume;
extern int SysTaskSetTermProc;
extern int SysTaskSuspend;
extern int SysTaskSwitching;
extern int SysTaskTrigger;
extern int SysTaskWait;
extern int SysTaskWaitClr;
extern int SysTaskWake;
extern int SysTicksPerSecond;
extern int SysTimerCreate;
extern int SysTimerDelete;
extern int SysTimerRead;
extern int SysTimerWrite;
extern int SysTranslateKernelErr;
extern int SysUIAppSwitch;
extern int SysUIAppSwitchCont;
extern int SysUIBusy;
extern int SysUILaunch;
extern int SysUnimplemented;
extern int TimGetTicks;
extern int TimGetTicksMs;
extern int HostControl;
extern int SysBinarySearch;
extern int SysCopyStringResource;
extern int SysErrString;
extern int SysFormPointerArrayToStrings;
extern int SysInsertionSort;
extern int SysQSort;
extern int SysQSort68k;
extern int SysQSortP;
extern int SysRandom;
extern int SysRandom32;
extern int SysStringArray;
extern int SysStringByIndex;
extern int SysUFinishModule;
extern int SysUInitModule;
extern int TblDrawTable;
extern int TblEditing;
extern int TblEraseTable;
extern int TblFindRowData;
extern int TblFindRowID;
extern int TblGetBounds;
extern int TblGetColumnSpacing;
extern int TblGetColumnWidth;
extern int TblGetCurrentField;
extern int TblGetItemBounds;
extern int TblGetItemFont;
extern int TblGetItemInt;
extern int TblGetItemPtr;
extern int TblGetLastUsableRow;
extern int TblGetNumberOfColumns;
extern int TblGetNumberOfRows;
extern int TblGetRowData;
extern int TblGetRowHeight;
extern int TblGetRowID;
extern int TblGetSelection;
extern int TblGetTopRow;
extern int TblGrabFocus;
extern int TblHandleEvent;
extern int TblHasScrollBar;
extern int TblInsertRow;
extern int TblMarkRowInvalid;
extern int TblMarkTableInvalid;
extern int TblRedrawTable;
extern int TblReleaseFocus;
extern int TblRemoveRow;
extern int TblRowInvalid;
extern int TblRowMasked;
extern int TblRowSelectable;
extern int TblRowUsable;
extern int TblSelectItem;
extern int TblSetBounds;
extern int TblSetColumnEditIndicator;
extern int TblSetColumnMasked;
extern int TblSetColumnSpacing;
extern int TblSetColumnUsable;
extern int TblSetColumnWidth;
extern int TblSetCustomDrawProcedure;
extern int TblSetItemFont;
extern int TblSetItemInt;
extern int TblSetItemPtr;
extern int TblSetItemStyle;
extern int TblSetLoadDataProcedure;
extern int TblSetRowData;
extern int TblSetRowHeight;
extern int TblSetRowID;
extern int TblSetRowMasked;
extern int TblSetRowSelectable;
extern int TblSetRowStaticHeight;
extern int TblSetRowUsable;
extern int TblSetSaveDataProcedure;
extern int TblSetSelection;
extern int TblUnhighlightSelection;
extern int TelUnblockNotifications;
extern int TxtByteAttr;
extern int TxtCaselessCompare;
extern int TxtCharAttr;
extern int TxtCharBounds;
extern int TxtCharEncoding;
extern int TxtCharIsValid;
extern int TxtCharSize;
extern int TxtCharWidth;
extern int TxtCharXAttr;
extern int TxtCompare;
extern int TxtConvertEncoding;
extern int TxtEncodingName;
extern int TxtFindString;
extern int TxtGetChar;
extern int TxtGetNextChar;
extern int TxtGetPreviousChar;
extern int TxtGetTruncationOffset;
extern int TxtGetWordWrapOffset;
extern int TxtLowerChar;
extern int TxtMaxEncoding;
extern int TxtNameToEncoding;
extern int TxtParamString;
extern int TxtReplaceStr;
extern int TxtSetNextChar;
extern int TxtStrEncoding;
extern int TxtTransliterate;
extern int TxtUpperChar;
extern int TxtWordBounds;
extern int TsmGetFepMode;
extern int TsmSetFepMode;
extern int TimGetSeconds;
extern int TimInit;
extern int TimSetSeconds;
extern int UDAControl;
extern int UDAExchangeReaderNew;
extern int UDAExchangeWriterNew;
extern int UDAMemoryReaderNew;
extern int UIColorGetDefaultTableEntryRGB;
extern int UIColorGetTableEntryIndex;
extern int UIColorGetTableEntryRGB;
extern int UIColorPopTable;
extern int UIColorPushTable;
extern int UIColorRestoreTable;
extern int UIColorSaveTable;
extern int UIColorSetTableEntry;
extern int UicFinishModule;
extern int UicInitModule;
extern int UIBrightnessAdjust;
extern int UIContrastAdjust;
extern int UIPickColor;
extern int ResLoadConstant;
extern int ResLoadForm;
extern int ResLoadMenu;
extern int ResLoadString;
extern int VFSAddVolume;
extern int VFSChangeDir;
extern int VFSCurrentDir;
extern int VFSCustomControl;
extern int VFSDirCreate;
extern int VFSDirEntryEnumerate;
extern int VFSExportDatabaseToFile;
extern int VFSExportDatabaseToFileCustom;
extern int VFSExportDatabaseToFileEx;
extern int VFSFileClose;
extern int VFSFileCreate;
extern int VFSFileDBGetRecord;
extern int VFSFileDBGetResource;
extern int VFSFileDelete;
extern int VFSFileEOF;
extern int VFSFileGetAttributes;
extern int VFSFileGetDate;
extern int VFSFileGets;
extern int VFSFileOpen;
extern int VFSFilePrintF;
extern int VFSFileRead;
extern int VFSFileReadData;
extern int VFSFileRename;
extern int VFSFileResize;
extern int VFSFileSeek;
extern int VFSFileSetAttributes;
extern int VFSFileSetDate;
extern int VFSFileSize;
extern int VFSFileTell;
extern int VFSFileTruncate;
extern int VFSFileVPrintF;
extern int VFSFileWrite;
extern int VFSFinishModule;
extern int VFSGetAttributes;
extern int VFSGetDefaultDirectory;
extern int VFSGetMount;
extern int VFSImportDatabaseFromFile;
extern int VFSImportDatabaseFromFileCustom;
extern int VFSImportDatabaseFromFileEx;
extern int VFSInit;
extern int VFSInitModule;
extern int VFSInstallFSLib;
extern int VFSRealPath;
extern int VFSRegisterDefaultDirectory;
extern int VFSRemoveFSLib;
extern int VFSTmpName;
extern int VFSUnregisterDefaultDirectory;
extern int VFSVolumeEnumerate;
extern int VFSVolumeExists;
extern int VFSVolumeFormat;
extern int VFSVolumeGetLabel;
extern int VFSVolumeInfo;
extern int VFSVolumeMount;
extern int VFSVolumeSetLabel;
extern int VFSVolumeSize;
extern int VFSVolumeUnmount;
extern int VFSExplorerCreate;
extern int VFSExplorerCurrentPath;
extern int VFSExplorerDestroy;
extern int VFSExplorerEnter;
extern int VFSExplorerHandleEvent;
extern int VFSExplorerPaginate;
extern int VFSExplorerRefresh;
extern int VFSExplorerSelectedItem;
extern int FileBrowserLibClose;
extern int FileBrowserLibOpen;
extern int FileBrowserLibParseFileURL;
extern int FileBrowserLibShowMultiselectDialog;
extern int FileBrowserLibShowOpenDialog;
extern int FileBrowserLibShowSaveAsDialog;
extern int FileBrowserLibSleep;
extern int FileBrowserLibWake;
extern int EvtGetPenNative;
extern int WinAddWindow;
extern int WinAdjustCoords;
extern int WinAdjustCoordsInv;
extern int WinBlitBitmap;
extern int WinClipRectangle;
extern int WinConvertToDisplay;
extern int WinCopyBitmap;
extern int WinCopyRectangle;
extern int WinCopyWindow;
extern int WinCreateBitmapWindow;
extern int WinCreateOffscreenWindow;
extern int WinCreateSurface;
extern int WinCreateWindow;
extern int WinDeleteWindow;
extern int WinDirectAccessHack;
extern int WinDisableWindow;
extern int WinDisplayToWindowPt;
extern int WinDrawBitmap;
extern int WinDrawChar;
extern int WinDrawCharBox;
extern int WinDrawChars;
extern int WinDrawGrayLine;
extern int WinDrawGrayRectangleFrame;
extern int WinDrawInvertedChars;
extern int WinDrawLine;
extern int WinDrawPixel;
extern int WinDrawRectangle;
extern int WinDrawRectangleFrame;
extern int WinDrawTruncChars;
extern int WinDrawWindowFrame;
extern int WinEnableWindow;
extern int WinEraseChars;
extern int WinEraseLine;
extern int WinErasePixel;
extern int WinEraseRectangle;
extern int WinEraseRectangleFrame;
extern int WinEraseWindow;
extern int WinFillLine;
extern int WinFillRectangle;
extern int WinFinishModule;
extern int WinGetActiveWindow;
extern int WinGetBackColor;
extern int WinGetBackColorU;
extern int WinGetBitmap;
extern int WinGetBorderRect;
extern int WinGetBounds;
extern int WinGetClip;
extern int WinGetColorTable;
extern int WinGetCoordinateSystem;
extern int WinGetDescr;
extern int WinGetDisplayExtent;
extern int WinGetDisplayWindow;
extern int WinGetDrawMode;
extern int WinGetDrawWindow;
extern int WinGetDrawWindowBounds;
extern int WinGetFirstWindow;
extern int WinGetForeColor;
extern int WinGetForeColorU;
extern int WinGetFramesRectangle;
extern int WinGetPalette;
extern int WinGetPattern;
extern int WinGetPatternType;
extern int WinGetPixel;
extern int WinGetPixelRGB;
extern int WinGetPosition;
extern int WinGetRealCoordinateSystem;
extern int WinGetScalingMode;
extern int WinGetSupportedDensity;
extern int WinGetWindowExtent;
extern int WinGetWindowFrameRect;
extern int WinIndexToRGB;
extern int WinInitModule;
extern int WinInitializeWindow;
extern int WinInvertChars;
extern int WinInvertLine;
extern int WinInvertPixel;
extern int WinInvertRect;
extern int WinInvertRectangle;
extern int WinInvertRectangleFrame;
extern int WinLegacyGetAddr;
extern int WinLegacyRead;
extern int WinLegacyWrite;
extern int WinModal;
extern int WinMoveWindowAddr;
extern int WinPaintBitmap;
extern int WinPaintBitmapEx;
extern int WinPaintChar;
extern int WinPaintChars;
extern int WinPaintLine;
extern int WinPaintLines;
extern int WinPaintPixel;
extern int WinPaintPixels;
extern int WinPaintRectangle;
extern int WinPaintRectangleFrame;
extern int WinPaintRoundedRectangleFrame;
extern int WinPaintTiledBitmap;
extern int WinPalette;
extern int WinPopDrawState;
extern int WinPushDrawState;
extern int WinRGBToIndex;
extern int WinReinitModule;
extern int WinRemoveWindow;
extern int WinResetClip;
extern int WinRestoreBits;
extern int WinRestoreRectangle;
extern int WinSaveBits;
extern int WinSaveRectangle;
extern int WinScaleAbsRect;
extern int WinScaleCoord;
extern int WinScalePoint;
extern int WinScaleRectangle;
extern int WinScreenGetAttribute;
extern int WinScreenInit;
extern int WinScreenLock;
extern int WinScreenMode;
extern int WinScreenUnlock;
extern int WinScrollRectangle;
extern int WinSendWindowEvents;
extern int WinSetActiveWindow;
extern int WinSetAsciiText;
extern int WinSetBackColor;
extern int WinSetBackColorRGB;
extern int WinSetBounds;
extern int WinSetClip;
extern int WinSetClipingBounds;
extern int WinSetColors;
extern int WinSetConstraintsSize;
extern int WinSetCoordinateSystem;
extern int WinSetDisplayExtent;
extern int WinSetDrawMode;
extern int WinSetDrawWindow;
extern int WinSetForeColor;
extern int WinSetForeColorRGB;
extern int WinSetPattern;
extern int WinSetPatternType;
extern int WinSetScalingMode;
extern int WinSetTextColor;
extern int WinSetTextColorRGB;
extern int WinSetUnderlineMode;
extern int WinUnscaleAbsRect;
extern int WinUnscaleCoord;
extern int WinUnscalePoint;
extern int WinUnscaleRectangle;
extern int WinValidateHandle;
extern int WinWindowToDisplayPt;
extern int ChatClose;
extern int ChatOpen;
extern int ChatQuery;
extern int heap_alloc;
extern int heap_base;
extern int heap_dump;
extern int heap_finish;
extern int heap_free;
extern int heap_init;
extern int heap_realloc;
extern int heap_size;
extern int grail_begin;
extern int grail_end;
extern int grail_reset;
extern int grail_stroke;
extern int WavBufferHeader;
extern int WavFileHeader;
extern int dia_clicked;
extern int dia_color;
extern int dia_draw_stroke;
extern int dia_finish;
extern int dia_get_graffiti_dimension;
extern int dia_get_main_dimension;
extern int dia_get_state;
extern int dia_get_taskbar_dimension;
extern int dia_get_trigger;
extern int dia_init;
extern int dia_refresh;
extern int dia_set_graffiti_state;
extern int dia_set_state;
extern int dia_set_trigger;
extern int dia_set_wh;
extern int dia_stroke;
extern int dia_update;
extern int taskbar_add;
extern int taskbar_add_widget;
extern int taskbar_clicked;
extern int taskbar_create;
extern int taskbar_destroy;
extern int taskbar_draw;
extern int taskbar_remove;
extern int taskbar_remove_widget;
extern int taskbar_update;
extern int taskbar_widget_clicked;
extern int wman_add;
extern int wman_choose_border;
extern int wman_clear;
extern int wman_clicked;
extern int wman_draw_all;
extern int wman_finish;
extern int wman_init;
extern int wman_move;
extern int wman_raise;
extern int wman_remove;
extern int wman_set_background;
extern int wman_set_border;
extern int wman_set_image_background;
extern int wman_texture;
extern int wman_update;
extern int wman_xy;
extern int pumpkin_editor_init_io;
extern int pumpkin_editor_init_term;
extern int syntax_get_plugin;
extern int editor_get_plugin;
extern int RegDelete;
extern int RegFinish;
extern int RegGet;
extern int RegGetById;
extern int RegInit;
extern int RegSet;
extern int LanguageFinish;
extern int LanguageInit;
extern int calibrate;
extern int pumpkin_unzip_file;
extern int pumpkin_unzip_filename;
extern int pumpkin_unzip_memory;
extern int pumpkin_unzip_resource;
extern int jzBuffer;
extern int jzReadCentralDirectory;
extern int jzReadData;
extern int jzReadEndRecord;
extern int jzReadLocalFileHeader;
extern int jzReadLocalFileHeaderRaw;
extern int puff;
extern int plibc_chdir;
extern int plibc_close;
extern int plibc_dup;
extern int plibc_dup2;
extern int plibc_errno;
extern int plibc_error;
extern int plibc_fclose;
extern int plibc_fdopen;
extern int plibc_feof;
extern int plibc_fflush;
extern int plibc_fgetc;
extern int plibc_fgets;
extern int plibc_fileno;
extern int plibc_finish;
extern int plibc_fopen;
extern int plibc_fprintf;
extern int plibc_fputc;
extern int plibc_fputs;
extern int plibc_fread;
extern int plibc_fseek;
extern int plibc_ftell;
extern int plibc_ftruncate;
extern int plibc_fwrite;
extern int plibc_getdir;
extern int plibc_haschar;
extern int plibc_init;
extern int plibc_isatty;
extern int plibc_lseek;
extern int plibc_mkdir;
extern int plibc_open;
extern int plibc_printf;
extern int plibc_read;
extern int plibc_remove;
extern int plibc_rename;
extern int plibc_setfd;
extern int plibc_stderr;
extern int plibc_stdin;
extern int plibc_stdout;
extern int plibc_strerror;
extern int plibc_tmpnam;
extern int plibc_ungetc;
extern int plibc_vfprintf;
extern int plibc_vprintf;
extern int plibc_write;
extern int DOSBoxMain;
extern int _ssfn_zlib_decode;
extern int pumpkin_create_ssfn;
extern int pumpkin_destroy_ssfn;
extern int ssfn_bbox;
extern int ssfn_errstr;
extern int ssfn_free;
extern int ssfn_load;
extern int ssfn_mem;
extern int ssfn_render;
extern int ssfn_select;
extern int ssfn_text;
extern int ssfn_utf8;
extern int logtrap_disasm;
extern int logtrap_finish;
extern int logtrap_global_finish;
extern int logtrap_global_init;
extern int logtrap_init;
extern int logtrap_start;
extern int logtrap_started;
extern int logtrap_trapname;
extern int BmpGlueGetBitDepth;
extern int BmpGlueGetBits;
extern int BmpGlueGetCompressionType;
extern int BmpGlueGetDimensions;
extern int BmpGlueGetNextBitmap;
extern int BmpGlueGetTransparentValue;
extern int BmpGlueSetTransparentValue;
extern int CtlGlueGetControlStyle;
extern int CtlGlueGetFont;
extern int CtlGlueGetGraphics;
extern int CtlGlueIsGraphical;
extern int CtlGlueNewSliderControl;
extern int CtlGlueSetFont;
extern int CtlGlueSetFrameStyle;
extern int CtlGlueSetLeftAnchor;
extern int DateGlueTemplateToAscii;
extern int DateGlueToDOWDMFormat;
extern int FldGlueGetLineInfo;
extern int FntGlueGetDefaultFontID;
extern int FntGlueTruncateString;
extern int FntGlueWCharWidth;
extern int FntGlueWidthToOffset;
extern int FrmGlueGetDefaultButtonID;
extern int FrmGlueGetEventHandler;
extern int FrmGlueGetHelpID;
extern int FrmGlueGetLabelFont;
extern int FrmGlueGetMenuBarID;
extern int FrmGlueGetObjIDFromObjPtr;
extern int FrmGlueGetObjectUsable;
extern int FrmGlueSetDefaultButtonID;
extern int FrmGlueSetHelpID;
extern int FrmGlueSetLabelFont;
extern int LstGlueGetDrawFunction;
extern int LstGlueGetFont;
extern int LstGlueGetItemsText;
extern int LstGlueGetTopItem;
extern int LstGlueSetFont;
extern int LstGlueSetIncrementalSearch;
extern int MemGluePtrNew;
extern int TblGlueGetColumnMasked;
extern int TblGlueGetItemPtr;
extern int TblGlueGetNumberOfColumns;
extern int TblGlueGetTopRow;
extern int TblGlueSetSelection;
extern int TxtGlueCharAttr;
extern int TxtGlueCharIsValid;
extern int TxtGlueCharSize;
extern int TxtGlueCharWidth;
extern int TxtGlueFindString;
extern int TxtGlueGetNextChar;
extern int TxtGlueGetPreviousChar;
extern int TxtGlueLowerChar;
extern int TxtGlueSetNextChar;
extern int TxtGlueUpperChar;
extern int WinGlueDrawTruncChars;
extern int WinGlueGetFrameType;
extern int WinGlueSetFrameType;
extern int GPSClose;
extern int GPSFinishModule;
extern int GPSGetLibAPIVersion;
extern int GPSGetMaxSatellites;
extern int GPSGetPVT;
extern int GPSGetPosition;
extern int GPSGetSatellites;
extern int GPSGetStatus;
extern int GPSGetTime;
extern int GPSGetVelocity;
extern int GPSInitModule;
extern int GPSOpen;
extern int GPDClose;
extern int GPDGetVersion;
extern int GPDOpen;
extern int GPDReadInstant;
extern int GPDReadInstantKCSformat;
extern int CallCompareFunction;
extern int CallDmCompare;
extern int CallFormHandler;
extern int CallGadgetHandler;
extern int CallListDrawItem;
extern int CallNotifyProc;
extern int CallPrgCallback;
extern int CallSndFunc;
extern int CallSndFuncArm;
extern int CallSndVFunc;
extern int CallSndVFuncArm;
extern int CallTableDrawItem;
extern int CallTableLoadData;
extern int CallTableSaveData;
extern int ExgDBReadARM;
extern int arm_native_call_pce;
extern int arm_native_call_sub;
extern int cpu_pulse_reset;
extern int cpu_read_byte;
extern int cpu_read_long;
extern int cpu_read_word;
extern int cpu_write_byte;
extern int cpu_write_long;
extern int cpu_write_word;
extern int decode_FileInfoType;
extern int decode_NetConfigNameType;
extern int decode_NetSocketAddrType;
extern int decode_appinfo;
extern int decode_datetime;
extern int decode_event;
extern int decode_locale;
extern int decode_notify;
extern int decode_point;
extern int decode_rectangle;
extern int decode_rgb;
extern int decode_smfoptions;
extern int emupalmos_check_address;
extern int emupalmos_deinstall;
extern int emupalmos_disasm;
extern int emupalmos_finish;
extern int emupalmos_finished;
extern int emupalmos_init;
extern int emupalmos_install;
extern int emupalmos_main;
extern int emupalmos_memory_hooks;
extern int emupalmos_panic;
extern int emupalmos_ram;
extern int emupalmos_trap_in;
extern int emupalmos_trap_out;
extern int emupalmos_trap_sel_in;
extern int encode_FileInfoType;
extern int encode_NetConfigNameType;
extern int encode_NetHostInfoBufType;
extern int encode_NetSocketAddrType;
extern int encode_VolumeInfoType;
extern int encode_appinfo;
extern int encode_datetime;
extern int encode_deviceinfo;
extern int encode_event;
extern int encode_gadget;
extern int encode_locale;
extern int encode_notify;
extern int encode_point;
extern int encode_rectangle;
extern int encode_rgb;
extern int encode_string;
extern int m68k_get_emu_state;
extern int m68k_get_state;
extern int m68k_trapname;
extern int emupalmos_arm_syscall;
extern int palmos_omtrap;
extern int palmos_pinstrap;
extern int palmos_highdensitytrap;
extern int palmos_serialtrap;
extern int palmos_filesystemtrap;
extern int palmos_intltrap;
extern int palmos_flpemtrap;
extern int palmos_flptrap;
extern int palmos_accessortrap;
extern int palmos_expansiontrap;
extern int palmos_tsmtrap;
extern int palmos_lmtrap;
extern int palmos_navtrap;
extern int palmos_netlibtrap;
extern int palmos_gpdlibtrap;
extern int palmos_systrap;
extern int default_instr_hook_callback;
extern int m68040_fpu_op0;
extern int m68040_fpu_op1;
extern int m68881_mmu_ops;
extern int m68k_context_size;
extern int m68k_cycles_remaining;
extern int m68k_cycles_run;
extern int m68k_end_timeslice;
extern int m68k_execute;
extern int m68k_get_context;
extern int m68k_get_reg;
extern int m68k_get_virq;
extern int m68k_init;
extern int m68k_init_once;
extern int m68k_make_hex;
extern int m68k_modify_timeslice;
extern int m68k_pulse_halt;
extern int m68k_pulse_reset;
extern int m68k_set_bkpt_ack_callback;
extern int m68k_set_cmpild_instr_callback;
extern int m68k_set_context;
extern int m68k_set_cpu_type;
extern int m68k_set_fc_callback;
extern int m68k_set_illg_instr_callback;
extern int m68k_set_instr_hook2_callback;
extern int m68k_set_instr_hook_callback;
extern int m68k_set_int_ack_callback;
extern int m68k_set_irq;
extern int m68k_set_pc_changed_callback;
extern int m68k_set_reg;
extern int m68k_set_reset_instr_callback;
extern int m68k_set_rte_instr_callback;
extern int m68k_set_tas_instr_callback;
extern int m68k_set_virq;
extern int pmmu_translate_addr;
extern int float128_add;
extern int float128_div;
extern int float128_eq;
extern int float128_eq_signaling;
extern int float128_is_nan;
extern int float128_is_signaling_nan;
extern int float128_le;
extern int float128_le_quiet;
extern int float128_lt;
extern int float128_lt_quiet;
extern int float128_mul;
extern int float128_rem;
extern int float128_round_to_int;
extern int float128_sqrt;
extern int float128_sub;
extern int float128_to_float32;
extern int float128_to_float64;
extern int float128_to_floatx80;
extern int float128_to_int32;
extern int float128_to_int32_round_to_zero;
extern int float128_to_int64;
extern int float128_to_int64_round_to_zero;
extern int float32_add;
extern int float32_div;
extern int float32_eq;
extern int float32_eq_signaling;
extern int float32_is_nan;
extern int float32_is_signaling_nan;
extern int float32_le;
extern int float32_le_quiet;
extern int float32_lt;
extern int float32_lt_quiet;
extern int float32_mul;
extern int float32_rem;
extern int float32_round_to_int;
extern int float32_sqrt;
extern int float32_sub;
extern int float32_to_float128;
extern int float32_to_float64;
extern int float32_to_floatx80;
extern int float32_to_int32;
extern int float32_to_int32_round_to_zero;
extern int float32_to_int64;
extern int float32_to_int64_round_to_zero;
extern int float64_add;
extern int float64_div;
extern int float64_eq;
extern int float64_eq_signaling;
extern int float64_is_nan;
extern int float64_is_signaling_nan;
extern int float64_le;
extern int float64_le_quiet;
extern int float64_lt;
extern int float64_lt_quiet;
extern int float64_mul;
extern int float64_rem;
extern int float64_round_to_int;
extern int float64_sqrt;
extern int float64_sub;
extern int float64_to_float128;
extern int float64_to_float32;
extern int float64_to_floatx80;
extern int float64_to_int32;
extern int float64_to_int32_round_to_zero;
extern int float64_to_int64;
extern int float64_to_int64_round_to_zero;
extern int float_detect_tininess;
extern int float_exception_flags;
extern int float_raise;
extern int float_rounding_mode;
extern int floatx80_add;
extern int floatx80_div;
extern int floatx80_eq;
extern int floatx80_eq_signaling;
extern int floatx80_is_nan;
extern int floatx80_is_signaling_nan;
extern int floatx80_le;
extern int floatx80_le_quiet;
extern int floatx80_lt;
extern int floatx80_lt_quiet;
extern int floatx80_mul;
extern int floatx80_rem;
extern int floatx80_round_to_int;
extern int floatx80_rounding_precision;
extern int floatx80_sqrt;
extern int floatx80_sub;
extern int floatx80_to_float128;
extern int floatx80_to_float32;
extern int floatx80_to_float64;
extern int floatx80_to_int32;
extern int floatx80_to_int32_round_to_zero;
extern int floatx80_to_int64;
extern int floatx80_to_int64_round_to_zero;
extern int int32_to_float128;
extern int int32_to_float32;
extern int int32_to_float64;
extern int int32_to_floatx80;
extern int int64_to_float128;
extern int int64_to_float32;
extern int int64_to_float64;
extern int int64_to_floatx80;
extern int propagateFloatx80NaN;
extern int roundAndPackFloatx80;
extern int m68ki_build_opcode_table;
extern int m68ki_cycles;
extern int m68ki_instruction_jump_table;
extern int disasm;
extern int decode_notif;
extern int decode_sysNotifyAppCrashedEvent;
extern int decode_sysNotifyAppLaunchingEvent;
extern int decode_sysNotifyAppQuittingEvent;
extern int decode_sysNotifyCardInsertedEvent;
extern int decode_sysNotifyCardRemovedEvent;
extern int decode_sysNotifyDBCreatedEvent;
extern int decode_sysNotifyDBDeletedEvent;
extern int decode_sysNotifyDisplayChangeEvent;
extern int decode_sysNotifyGPSDataEvent;
extern int decode_sysNotifyLocaleChangedEvent;
extern int decode_sysNotifySelectDay;
extern int decode_sysNotifySyncFinishEvent;
extern int decode_sysNotifyTimeChangeEvent;
extern int decode_sysNotifyVolumeMountedEvent;
extern int decode_sysNotifyVolumeUnmountedEvent;
extern int encode_notif;
extern int encode_sysNotifyAppCrashedEvent;
extern int encode_sysNotifyAppLaunchingEvent;
extern int encode_sysNotifyAppQuittingEvent;
extern int encode_sysNotifyCardInsertedEvent;
extern int encode_sysNotifyCardRemovedEvent;
extern int encode_sysNotifyDBCreatedEvent;
extern int encode_sysNotifyDBDeletedEvent;
extern int encode_sysNotifyDisplayChangeEvent;
extern int encode_sysNotifyGPSDataEvent;
extern int encode_sysNotifyLocaleChangedEvent;
extern int encode_sysNotifySelectDay;
extern int encode_sysNotifySyncFinishEvent;
extern int encode_sysNotifyTimeChangeEvent;
extern int encode_sysNotifyVolumeMountedEvent;
extern int encode_sysNotifyVolumeUnmountedEvent;
extern int decode_launch;
extern int decode_sysAppLaunchCmdAlarmTriggered;
extern int decode_sysAppLaunchCmdDisplayAlarm;
extern int decode_sysAppLaunchCmdGoTo;
extern int decode_sysAppLaunchCmdPanelCalledFromApp;
extern int decode_sysAppLaunchCmdSystemReset;
extern int encode_launch;
extern int encode_sysAppLaunchCmdAlarmTriggered;
extern int encode_sysAppLaunchCmdDisplayAlarm;
extern int encode_sysAppLaunchCmdGoTo;
extern int encode_sysAppLaunchCmdPanelCalledFromApp;
extern int encode_sysAppLaunchCmdSystemReset;
extern int memAccess;
extern int memDeinit;
extern int memInit;
extern int memRegionAdd;
extern int uarmDisasm;
extern int uarmFinish;
extern int uarmGetReg;
extern int uarmInit;
extern int uarmRun;
extern int uarmSetReg;
extern int cpuCoprocessorRegister;
extern int cpuCycle;
extern int cpuDeinit;
extern int cpuDisasm;
extern int cpuGetCPAR;
extern int cpuGetPid;
extern int cpuGetRegExternal;
extern int cpuInit;
extern int cpuIrq;
extern int cpuMemOpExternal;
extern int cpuReg;
extern int cpuSetCPAR;
extern int cpuSetPid;
extern int cpuSetReg;
extern int cpuSetVectorAddr;
extern int icacheDeinit;
extern int icacheFetch;
extern int icacheInit;
extern int icacheInval;
extern int icacheInvalAddr;
extern int mmuDeinit;
extern int mmuDump;
extern int mmuGetDomainCfg;
extern int mmuGetR;
extern int mmuGetS;
extern int mmuGetTTP;
extern int mmuInit;
extern int mmuIsOn;
extern int mmuSetDomainCfg;
extern int mmuSetR;
extern int mmuSetS;
extern int mmuSetTTP;
extern int mmuTlbFlush;
extern int mmuTranslate;
extern int ramDeinit;
extern int ramInit;
extern int pxa270icPrvCoprocAccess;
extern int socIcDeinit;
extern int socIcInit;
extern int socIcInt;
extern int cp15Cycle;
extern int cp15Deinit;
extern int cp15Init;
extern int cp15SetFaultStatus;
extern int darm_armv7_disasm;
extern int darm_condition_index;
extern int darm_condition_meaning_fp;
extern int darm_condition_meaning_int;
extern int darm_condition_name;
extern int darm_enctype_name;
extern int darm_immshift_decode;
extern int darm_mnemonic_name;
extern int darm_register_name;
extern int darm_shift_type_name;
extern int armv7_format_strings;
extern int armv7_instr_labels;
extern int armv7_instr_types;
extern int type_bits_instr_lookup;
extern int type_brnchmisc_instr_lookup;
extern int type_mul_instr_lookup;
extern int type_opless_instr_lookup;
extern int type_pas_instr_lookup;
extern int type_pusr_instr_lookup;
extern int type_sat_instr_lookup;
extern int type_shift_instr_lookup;
extern int type_stack0_instr_lookup;
extern int type_stack1_instr_lookup;
extern int type_stack2_instr_lookup;
extern int type_sync_instr_lookup;
extern int type_uncond2_instr_lookup;
extern int darm_disasm;
extern int darm_init;
extern int darm_reglist;
extern int darm_str;
extern int darm_str2;
extern int darm_enctypes;
extern int darm_mnemonics;
extern int darm_registers;
extern int darm_thumb_disasm;
extern int darm_thumb2_disasm;
extern int darm_thumb2_str;
extern int thumb2_decode_immshift;
extern int thumb2_parse_flag;
extern int thumb2_parse_imm;
extern int thumb2_parse_misc;
extern int thumb2_parse_reg;
extern int thumb_expand_imm;
extern int thumb2_branch_misc_ctrl;
extern int thumb2_coproc_simd;
extern int thumb2_data_reg;
extern int thumb2_data_shifted_reg;
extern int thumb2_decode_instruction;
extern int thumb2_load_byte_hints;
extern int thumb2_load_halfword_hints;
extern int thumb2_load_store_dual;
extern int thumb2_load_store_multiple;
extern int thumb2_load_word;
extern int thumb2_long_mult_acc;
extern int thumb2_misc_ctrl;
extern int thumb2_misc_op;
extern int thumb2_modified_immediate;
extern int thumb2_move_shift;
extern int thumb2_mult_acc_diff;
extern int thumb2_nm_decoder;
extern int thumb2_parallel_signed;
extern int thumb2_parallel_unsigned;
extern int thumb2_plain_immediate;
extern int thumb2_proc_state;
extern int thumb2_store_single_item;
extern int thumb2_instr_labels;
extern int thumb2_instruction_strings;
extern int thumb_instr_labels;
extern int thumb_instr_types;
extern int type_extend_instr_lookup;
extern int type_gpi_instr_lookup;
extern int type_hints_instr_lookup;
extern int type_rev_instr_lookup;
extern int EmulatorMain;
extern int TOSMain;
extern int tos_convert_color;
extern int tos_get_key;
extern int tos_has_key;
extern int tos_read_byte;
extern int tos_systrap;
extern int tos_write_byte;
extern int tos_write_screen;
extern int xtos_convert_color;
extern int Cauxin;
extern int Cauxis;
extern int Cauxos;
extern int Cauxout;
extern int Cconin;
extern int Cconis;
extern int Cconos;
extern int Cconout;
extern int Cconrs;
extern int Cconws;
extern int Cnecin;
extern int Cprnos;
extern int Cprnout;
extern int Crawcin;
extern int Crawio;
extern int Dchroot;
extern int Dclosedir;
extern int Dcntl;
extern int Dcreate;
extern int Ddelete;
extern int Dfree;
extern int Dgetcwd;
extern int Dgetdrv;
extern int Dgetpath;
extern int Dlock;
extern int Dopendir;
extern int Dpathconf;
extern int Dreaddir;
extern int Dreadlabel;
extern int Drewinddir;
extern int Dsetdrv;
extern int Dsetkey;
extern int Dsetpath;
extern int Dwritelabel;
extern int Dxopendir;
extern int Dxreaddir;
extern int Faccept;
extern int Fattrib;
extern int Fbind;
extern int Fchdir;
extern int Fchmod;
extern int Fchown;
extern int Fchown16;
extern int Fclose;
extern int Fconnect;
extern int Fcreate;
extern int Fdatime;
extern int Fdelete;
extern int Fdirfd;
extern int Fdup;
extern int Ffchmod;
extern int Ffchown;
extern int Ffdopendir;
extern int Fflush;
extern int Fforce;
extern int Ffstat64;
extern int Fgetchar;
extern int Fgetpeername;
extern int Fgetsockname;
extern int Fgetsockopt;
extern int Finstat;
extern int Flink;
extern int Flisten;
extern int Flock;
extern int Flock2;
extern int Fmidipipe;
extern int Fopen;
extern int Foutstat;
extern int Fpipe;
extern int Fpoll;
extern int Fputchar;
extern int Fread;
extern int Freadlink;
extern int Freadv;
extern int Frecvfrom;
extern int Frecvmsg;
extern int Frename;
extern int Frlock;
extern int Frunlock;
extern int Fseek;
extern int Fseek64;
extern int Fselect;
extern int Fsendmsg;
extern int Fsendto;
extern int Fsetsockopt;
extern int Fsfirst;
extern int Fshutdown;
extern int Fsnext;
extern int Fsocket;
extern int Fsocketpair;
extern int Fstat64;
extern int Fsymlink;
extern int Fsync;
extern int Funlock;
extern int Fwrite;
extern int Fwritev;
extern int Fxattr;
extern int Maccess;
extern int Maddalt;
extern int Mfree;
extern int Mshrink;
extern int Mvalidate;
extern int Mxalloc;
extern int Nversion;
extern int Pause;
extern int Pdomain;
extern int Pfork;
extern int Pgetauid;
extern int Pgetegid;
extern int Pgeteuid;
extern int Pgetgid;
extern int Pgetgroups;
extern int Pgetpgrp;
extern int Pgetpid;
extern int Pgetppid;
extern int Pgetpriority;
extern int Pgetuid;
extern int Pkill;
extern int Pmsg;
extern int Pnice;
extern int Prenice;
extern int Prusage;
extern int Psemaphore;
extern int Psetauid;
extern int Psetegid;
extern int Pseteuid;
extern int Psetgid;
extern int Psetgroups;
extern int Psetlimit;
extern int Psetpgrp;
extern int Psetpriority;
extern int Psetregid;
extern int Psetreuid;
extern int Psetuid;
extern int Psigaction;
extern int Psigblock;
extern int Psigintr;
extern int Psignal;
extern int Psigpause;
extern int Psigpending;
extern int Psigreturn;
extern int Psigsetmask;
extern int Psysctl;
extern int Pterm;
extern int Pterm0;
extern int Ptermres;
extern int Ptrace;
extern int Pumask;
extern int Pusrval;
extern int Pvfork;
extern int Pwait;
extern int Pwait3;
extern int Pwaitpid;
extern int Salert;
extern int Shutdown;
extern int Srealloc;
extern int Ssystem;
extern int Suptime;
extern int Sversion;
extern int Syield;
extern int Sync;
extern int Sysconf;
extern int Tadjtime;
extern int Talarm;
extern int Tgetdate;
extern int Tgettime;
extern int Tgettimeofday;
extern int Tmalarm;
extern int Tsetdate;
extern int Tsetitimer;
extern int Tsettime;
extern int Tsettimeofday;
extern int Bconin;
extern int Bconout;
extern int Bconstat;
extern int Bcostat;
extern int Drvmap;
extern int Getbpb;
extern int Getmpb;
extern int Kbshift;
extern int Mediach;
extern int Rwabs;
extern int Setexc;
extern int Tickcal;
extern int Bconmap;
extern int Bioskeys;
extern int Blitmode;
extern int CacheCtrl;
extern int Cursconf;
extern int DMAread;
extern int DMAwrite;
extern int Dbmsg;
extern int Dosound;
extern int Dsp_Available;
extern int Dsp_BlkBytes;
extern int Dsp_BlkHandShake;
extern int Dsp_BlkUnpacked;
extern int Dsp_BlkWords;
extern int Dsp_DoBlock;
extern int Dsp_ExecBoot;
extern int Dsp_ExecProg;
extern int Dsp_FlushSubroutines;
extern int Dsp_GetProgAbility;
extern int Dsp_GetWordSize;
extern int Dsp_HStat;
extern int Dsp_Hf0;
extern int Dsp_Hf1;
extern int Dsp_Hf2;
extern int Dsp_Hf3;
extern int Dsp_IOStream;
extern int Dsp_InStream;
extern int Dsp_InqSubrAbility;
extern int Dsp_LoadProg;
extern int Dsp_LoadSubroutine;
extern int Dsp_Lock;
extern int Dsp_LodToBinary;
extern int Dsp_MultBlocks;
extern int Dsp_OutStream;
extern int Dsp_RemoveInterrupts;
extern int Dsp_RequestUniqueAbility;
extern int Dsp_Reserve;
extern int Dsp_RunSubroutine;
extern int Dsp_SetVectors;
extern int Dsp_TriggerHC;
extern int Dsp_Unlock;
extern int EgetPalette;
extern int EgetShift;
extern int EsetBank;
extern int EsetColor;
extern int EsetGray;
extern int EsetPalette;
extern int EsetShift;
extern int EsetSmear;
extern int Flopfmt;
extern int Floprate;
extern int Floprd;
extern int Flopver;
extern int Flopwr;
extern int Gettime;
extern int Giaccess;
extern int Ikbdws;
extern int Initmouse;
extern int Iorec;
extern int Jdisint;
extern int Jenabint;
extern int Kbrate;
extern int Keytbl;
extern int Metaclose;
extern int Metadiscinfo;
extern int Metagettoc;
extern int Metainit;
extern int Metaioctl;
extern int Metaopen;
extern int Metaread;
extern int Metaseek;
extern int Metasetsongtime;
extern int Metastartaudio;
extern int Metastatus;
extern int Metastopaudio;
extern int Metawrite;
extern int Mfpint;
extern int Midiws;
extern int NVMaccess;
extern int Offgibit;
extern int Ongibit;
extern int Protobt;
extern int Prtblk;
extern int Puntaes;
extern int Random;
extern int Rsconf;
extern int Scrdmp;
extern int Setprt;
extern int Settime;
extern int Ssbrk;
extern int ValidMode;
extern int VgetRGB;
extern int VgetSize;
extern int VsetMask;
extern int VsetMode;
extern int VsetRGB;
extern int VsetSync;
extern int Vsync;
extern int WavePlay;
extern int WdgCtrl;
extern int Xbios;
extern int Xbtimer;
extern int buffoper;
extern int buffptr;
extern int devconnect;
extern int dsptristate;
extern int gpio;
extern int locksnd;
extern int mon_type;
extern int setinterrupt;
extern int setmode;
extern int setmontracks;
extern int settracks;
extern int sndstatus;
extern int soundcmd;
extern int unlocksnd;
extern int vdi_call;
extern int aes_call;
extern int kd_clear;
extern int kd_create;
extern int kd_data_destructor;
extern int kd_free;
extern int kd_insert;
extern int kd_insert3;
extern int kd_nearest;
extern int kd_nearest3;
extern int kd_nearest_range;
extern int kd_nearest_range3;
extern int kd_res_end;
extern int kd_res_free;
extern int kd_res_item;
extern int kd_res_item3;
extern int kd_res_item_data;
extern int kd_res_next;
extern int kd_res_rewind;
extern int kd_res_size;
#pragma GCC diagnostic pop

/* Available ELF symbols table: g_PumpkinSyms_elfsyms */

const struct esp_elfsym g_PumpkinSyms_elfsyms[] = {
    ESP_ELFSYM_EXPORT(deserialize_launch),
    ESP_ELFSYM_EXPORT(deserialize_sysAppLaunchCmdAlarmTriggered),
    ESP_ELFSYM_EXPORT(deserialize_sysAppLaunchCmdDisplayAlarm),
    ESP_ELFSYM_EXPORT(deserialize_sysAppLaunchCmdGoTo),
    ESP_ELFSYM_EXPORT(deserialize_sysAppLaunchCmdPanelCalledFromApp),
    ESP_ELFSYM_EXPORT(deserialize_sysAppLaunchCmdSystemReset),
    ESP_ELFSYM_EXPORT(serialize_launch),
    ESP_ELFSYM_EXPORT(serialize_sysAppLaunchCmdAlarmTriggered),
    ESP_ELFSYM_EXPORT(serialize_sysAppLaunchCmdDisplayAlarm),
    ESP_ELFSYM_EXPORT(serialize_sysAppLaunchCmdGoTo),
    ESP_ELFSYM_EXPORT(serialize_sysAppLaunchCmdPanelCalledFromApp),
    ESP_ELFSYM_EXPORT(serialize_sysAppLaunchCmdSystemReset),
    ESP_ELFSYM_EXPORT(deserialize_notif),
    ESP_ELFSYM_EXPORT(deserialize_sysNotifyAppCrashedEvent),
    ESP_ELFSYM_EXPORT(deserialize_sysNotifyAppLaunchingEvent),
    ESP_ELFSYM_EXPORT(deserialize_sysNotifyAppQuittingEvent),
    ESP_ELFSYM_EXPORT(deserialize_sysNotifyCardInsertedEvent),
    ESP_ELFSYM_EXPORT(deserialize_sysNotifyCardRemovedEvent),
    ESP_ELFSYM_EXPORT(deserialize_sysNotifyDBCreatedEvent),
    ESP_ELFSYM_EXPORT(deserialize_sysNotifyDBDeletedEvent),
    ESP_ELFSYM_EXPORT(deserialize_sysNotifyDisplayChangeEvent),
    ESP_ELFSYM_EXPORT(deserialize_sysNotifyGPSDataEvent),
    ESP_ELFSYM_EXPORT(deserialize_sysNotifyLocaleChangedEvent),
    ESP_ELFSYM_EXPORT(deserialize_sysNotifySelectDay),
    ESP_ELFSYM_EXPORT(deserialize_sysNotifySyncFinishEvent),
    ESP_ELFSYM_EXPORT(deserialize_sysNotifyTimeChangeEvent),
    ESP_ELFSYM_EXPORT(deserialize_sysNotifyVolumeMountedEvent),
    ESP_ELFSYM_EXPORT(deserialize_sysNotifyVolumeUnmountedEvent),
    ESP_ELFSYM_EXPORT(serialize_notif),
    ESP_ELFSYM_EXPORT(serialize_sysNotifyAppCrashedEvent),
    ESP_ELFSYM_EXPORT(serialize_sysNotifyAppLaunchingEvent),
    ESP_ELFSYM_EXPORT(serialize_sysNotifyAppQuittingEvent),
    ESP_ELFSYM_EXPORT(serialize_sysNotifyCardInsertedEvent),
    ESP_ELFSYM_EXPORT(serialize_sysNotifyCardRemovedEvent),
    ESP_ELFSYM_EXPORT(serialize_sysNotifyDBCreatedEvent),
    ESP_ELFSYM_EXPORT(serialize_sysNotifyDBDeletedEvent),
    ESP_ELFSYM_EXPORT(serialize_sysNotifyDisplayChangeEvent),
    ESP_ELFSYM_EXPORT(serialize_sysNotifyGPSDataEvent),
    ESP_ELFSYM_EXPORT(serialize_sysNotifyLocaleChangedEvent),
    ESP_ELFSYM_EXPORT(serialize_sysNotifySelectDay),
    ESP_ELFSYM_EXPORT(serialize_sysNotifySyncFinishEvent),
    ESP_ELFSYM_EXPORT(serialize_sysNotifyTimeChangeEvent),
    ESP_ELFSYM_EXPORT(serialize_sysNotifyVolumeMountedEvent),
    ESP_ELFSYM_EXPORT(serialize_sysNotifyVolumeUnmountedEvent),
    ESP_ELFSYM_EXPORT(pumpkin_get_build),
    ESP_ELFSYM_EXPORT(LanguageGet),
    ESP_ELFSYM_EXPORT(LanguageSelect),
    ESP_ELFSYM_EXPORT(LongToRGB),
    ESP_ELFSYM_EXPORT(RGBToLong),
    ESP_ELFSYM_EXPORT(SysAppLaunch),
    ESP_ELFSYM_EXPORT(SysAppLaunchEx),
    ESP_ELFSYM_EXPORT(SysLibCancelRefNum68K),
    ESP_ELFSYM_EXPORT(SysLibFind68K),
    ESP_ELFSYM_EXPORT(SysLibGetDispatch68K),
    ESP_ELFSYM_EXPORT(SysLibNewRefNum68K),
    ESP_ELFSYM_EXPORT(SysLibRegister68K),
    ESP_ELFSYM_EXPORT(SysLibTblEntry68K),
    ESP_ELFSYM_EXPORT(SysNotifyBroadcast),
    ESP_ELFSYM_EXPORT(SysNotifyBroadcastDeferred),
    ESP_ELFSYM_EXPORT(SysNotifyBroadcastFromInterrupt),
    ESP_ELFSYM_EXPORT(SysNotifyBroadcastQueued),
    ESP_ELFSYM_EXPORT(SysNotifyRegister),
    ESP_ELFSYM_EXPORT(SysNotifyUnregister),
    ESP_ELFSYM_EXPORT(grail_draw_stroke),
    ESP_ELFSYM_EXPORT(heap_assertion_error),
    ESP_ELFSYM_EXPORT(heap_exhausted_error),
    ESP_ELFSYM_EXPORT(heap_get),
    ESP_ELFSYM_EXPORT(logtrap_get_def),
    ESP_ELFSYM_EXPORT(pumpkin_add_serial),
    ESP_ELFSYM_EXPORT(pumpkin_alarm_check),
    ESP_ELFSYM_EXPORT(pumpkin_alarm_get),
    ESP_ELFSYM_EXPORT(pumpkin_alarm_set),
    ESP_ELFSYM_EXPORT(pumpkin_app_crashed),
    ESP_ELFSYM_EXPORT(pumpkin_audio_get),
    ESP_ELFSYM_EXPORT(pumpkin_audio_set),
    ESP_ELFSYM_EXPORT(pumpkin_audio_task_finish),
    ESP_ELFSYM_EXPORT(pumpkin_audio_task_init),
    ESP_ELFSYM_EXPORT(pumpkin_baud_serial),
    ESP_ELFSYM_EXPORT(pumpkin_calibrate),
    ESP_ELFSYM_EXPORT(pumpkin_change_display),
    ESP_ELFSYM_EXPORT(pumpkin_clipboard_add_bitmap),
    ESP_ELFSYM_EXPORT(pumpkin_clipboard_add_text),
    ESP_ELFSYM_EXPORT(pumpkin_clipboard_append_text),
    ESP_ELFSYM_EXPORT(pumpkin_clipboard_get_text),
    ESP_ELFSYM_EXPORT(pumpkin_close_serial),
    ESP_ELFSYM_EXPORT(pumpkin_crash_log),
    ESP_ELFSYM_EXPORT(pumpkin_delete_preferences),
    ESP_ELFSYM_EXPORT(pumpkin_delete_registry),
    ESP_ELFSYM_EXPORT(pumpkin_deploy_files),
    ESP_ELFSYM_EXPORT(pumpkin_dia_enabled),
    ESP_ELFSYM_EXPORT(pumpkin_dia_get_state),
    ESP_ELFSYM_EXPORT(pumpkin_dia_get_taskbar_dimension),
    ESP_ELFSYM_EXPORT(pumpkin_dia_get_trigger),
    ESP_ELFSYM_EXPORT(pumpkin_dia_set_graffiti_state),
    ESP_ELFSYM_EXPORT(pumpkin_dia_set_state),
    ESP_ELFSYM_EXPORT(pumpkin_dia_set_trigger),
    ESP_ELFSYM_EXPORT(pumpkin_dirty_region_mode),
    ESP_ELFSYM_EXPORT(pumpkin_dt),
    ESP_ELFSYM_EXPORT(pumpkin_enum_plugins),
    ESP_ELFSYM_EXPORT(pumpkin_error_dialog),
    ESP_ELFSYM_EXPORT(pumpkin_error_msg),
    ESP_ELFSYM_EXPORT(pumpkin_event),
    ESP_ELFSYM_EXPORT(pumpkin_event_peek),
    ESP_ELFSYM_EXPORT(pumpkin_event_timeout),
    ESP_ELFSYM_EXPORT(pumpkin_extkey_down),
    ESP_ELFSYM_EXPORT(pumpkin_fatal_error),
    ESP_ELFSYM_EXPORT(pumpkin_fork),
    ESP_ELFSYM_EXPORT(pumpkin_forward_event),
    ESP_ELFSYM_EXPORT(pumpkin_forward_msg),
    ESP_ELFSYM_EXPORT(pumpkin_generic_error),
    ESP_ELFSYM_EXPORT(pumpkin_get_app_creator),
    ESP_ELFSYM_EXPORT(pumpkin_get_app_localid),
    ESP_ELFSYM_EXPORT(pumpkin_get_battery),
    ESP_ELFSYM_EXPORT(pumpkin_get_boolean_option),
    ESP_ELFSYM_EXPORT(pumpkin_get_current),
    ESP_ELFSYM_EXPORT(pumpkin_get_data),
    ESP_ELFSYM_EXPORT(pumpkin_get_default_osversion),
    ESP_ELFSYM_EXPORT(pumpkin_get_density),
    ESP_ELFSYM_EXPORT(pumpkin_get_depth),
    ESP_ELFSYM_EXPORT(pumpkin_get_encoding),
    ESP_ELFSYM_EXPORT(pumpkin_get_exception),
    ESP_ELFSYM_EXPORT(pumpkin_get_font),
    ESP_ELFSYM_EXPORT(pumpkin_get_id_option),
    ESP_ELFSYM_EXPORT(pumpkin_get_integer_option),
    ESP_ELFSYM_EXPORT(pumpkin_get_lasterr),
    ESP_ELFSYM_EXPORT(pumpkin_get_local_storage),
    ESP_ELFSYM_EXPORT(pumpkin_get_mode),
    ESP_ELFSYM_EXPORT(pumpkin_get_native_keys),
    ESP_ELFSYM_EXPORT(pumpkin_get_osversion),
    ESP_ELFSYM_EXPORT(pumpkin_get_param_size),
    ESP_ELFSYM_EXPORT(pumpkin_get_plugin),
    ESP_ELFSYM_EXPORT(pumpkin_get_preference),
    ESP_ELFSYM_EXPORT(pumpkin_get_serial),
    ESP_ELFSYM_EXPORT(pumpkin_get_serial_by_creator),
    ESP_ELFSYM_EXPORT(pumpkin_get_spawner),
    ESP_ELFSYM_EXPORT(pumpkin_get_string_option),
    ESP_ELFSYM_EXPORT(pumpkin_get_subdata),
    ESP_ELFSYM_EXPORT(pumpkin_get_taskid),
    ESP_ELFSYM_EXPORT(pumpkin_get_window),
    ESP_ELFSYM_EXPORT(pumpkin_getchar),
    ESP_ELFSYM_EXPORT(pumpkin_gets),
    ESP_ELFSYM_EXPORT(pumpkin_getstr),
    ESP_ELFSYM_EXPORT(pumpkin_gettable),
    ESP_ELFSYM_EXPORT(pumpkin_global_finish),
    ESP_ELFSYM_EXPORT(pumpkin_global_init),
    ESP_ELFSYM_EXPORT(pumpkin_haschar),
    ESP_ELFSYM_EXPORT(pumpkin_heap_alloc),
    ESP_ELFSYM_EXPORT(pumpkin_heap_base),
    ESP_ELFSYM_EXPORT(pumpkin_heap_dump),
    ESP_ELFSYM_EXPORT(pumpkin_heap_dup),
    ESP_ELFSYM_EXPORT(pumpkin_heap_free),
    ESP_ELFSYM_EXPORT(pumpkin_heap_realloc),
    ESP_ELFSYM_EXPORT(pumpkin_heap_size),
    ESP_ELFSYM_EXPORT(pumpkin_http_abort),
    ESP_ELFSYM_EXPORT(pumpkin_http_get),
    ESP_ELFSYM_EXPORT(pumpkin_httpd_create),
    ESP_ELFSYM_EXPORT(pumpkin_httpd_destroy),
    ESP_ELFSYM_EXPORT(pumpkin_httpd_status),
    ESP_ELFSYM_EXPORT(pumpkin_id2s),
    ESP_ELFSYM_EXPORT(pumpkin_info_serial),
    ESP_ELFSYM_EXPORT(pumpkin_init_misc),
    ESP_ELFSYM_EXPORT(pumpkin_is_launched),
    ESP_ELFSYM_EXPORT(pumpkin_is_m68k),
    ESP_ELFSYM_EXPORT(pumpkin_is_paused),
    ESP_ELFSYM_EXPORT(pumpkin_is_spawner),
    ESP_ELFSYM_EXPORT(pumpkin_keymask),
    ESP_ELFSYM_EXPORT(pumpkin_kill),
    ESP_ELFSYM_EXPORT(pumpkin_launch),
    ESP_ELFSYM_EXPORT(pumpkin_launcher),
    ESP_ELFSYM_EXPORT(pumpkin_load_plugins),
    ESP_ELFSYM_EXPORT(pumpkin_local_refresh),
    ESP_ELFSYM_EXPORT(pumpkin_map_char),
    ESP_ELFSYM_EXPORT(pumpkin_must_finish),
    ESP_ELFSYM_EXPORT(pumpkin_next_char),
    ESP_ELFSYM_EXPORT(pumpkin_num_serial),
    ESP_ELFSYM_EXPORT(pumpkin_open_serial),
    ESP_ELFSYM_EXPORT(pumpkin_pause),
    ESP_ELFSYM_EXPORT(pumpkin_printf),
    ESP_ELFSYM_EXPORT(pumpkin_ps),
    ESP_ELFSYM_EXPORT(pumpkin_putchar),
    ESP_ELFSYM_EXPORT(pumpkin_puts),
    ESP_ELFSYM_EXPORT(pumpkin_refresh_desktop),
    ESP_ELFSYM_EXPORT(pumpkin_reg_get),
    ESP_ELFSYM_EXPORT(pumpkin_reg_set),
    ESP_ELFSYM_EXPORT(pumpkin_s2id),
    ESP_ELFSYM_EXPORT(pumpkin_save_bitmap),
    ESP_ELFSYM_EXPORT(pumpkin_save_bmp),
    ESP_ELFSYM_EXPORT(pumpkin_save_surface),
    ESP_ELFSYM_EXPORT(pumpkin_screen_copy),
    ESP_ELFSYM_EXPORT(pumpkin_screen_dirty),
    ESP_ELFSYM_EXPORT(pumpkin_screen_lock),
    ESP_ELFSYM_EXPORT(pumpkin_screen_unlock),
    ESP_ELFSYM_EXPORT(pumpkin_script_call),
    ESP_ELFSYM_EXPORT(pumpkin_script_create),
    ESP_ELFSYM_EXPORT(pumpkin_script_create_obj),
    ESP_ELFSYM_EXPORT(pumpkin_script_destroy),
    ESP_ELFSYM_EXPORT(pumpkin_script_engine_id),
    ESP_ELFSYM_EXPORT(pumpkin_script_get_last_error),
    ESP_ELFSYM_EXPORT(pumpkin_script_global_function),
    ESP_ELFSYM_EXPORT(pumpkin_script_global_function_data),
    ESP_ELFSYM_EXPORT(pumpkin_script_global_iconst),
    ESP_ELFSYM_EXPORT(pumpkin_script_global_iconst_value),
    ESP_ELFSYM_EXPORT(pumpkin_script_global_pointer_value),
    ESP_ELFSYM_EXPORT(pumpkin_script_init),
    ESP_ELFSYM_EXPORT(pumpkin_script_obj_boolean),
    ESP_ELFSYM_EXPORT(pumpkin_script_obj_function),
    ESP_ELFSYM_EXPORT(pumpkin_script_obj_iconst),
    ESP_ELFSYM_EXPORT(pumpkin_script_obj_sconst),
    ESP_ELFSYM_EXPORT(pumpkin_script_run_file),
    ESP_ELFSYM_EXPORT(pumpkin_script_run_string),
    ESP_ELFSYM_EXPORT(pumpkin_send_deploy),
    ESP_ELFSYM_EXPORT(pumpkin_set_abgr),
    ESP_ELFSYM_EXPORT(pumpkin_set_battery),
    ESP_ELFSYM_EXPORT(pumpkin_set_data),
    ESP_ELFSYM_EXPORT(pumpkin_set_density),
    ESP_ELFSYM_EXPORT(pumpkin_set_depth),
    ESP_ELFSYM_EXPORT(pumpkin_set_finish),
    ESP_ELFSYM_EXPORT(pumpkin_set_fullrefresh),
    ESP_ELFSYM_EXPORT(pumpkin_set_lasterr),
    ESP_ELFSYM_EXPORT(pumpkin_set_local_storage),
    ESP_ELFSYM_EXPORT(pumpkin_set_lockable),
    ESP_ELFSYM_EXPORT(pumpkin_set_m68k),
    ESP_ELFSYM_EXPORT(pumpkin_set_mode),
    ESP_ELFSYM_EXPORT(pumpkin_set_mono),
    ESP_ELFSYM_EXPORT(pumpkin_set_native_keys),
    ESP_ELFSYM_EXPORT(pumpkin_set_obj),
    ESP_ELFSYM_EXPORT(pumpkin_set_osversion),
    ESP_ELFSYM_EXPORT(pumpkin_set_preference),
    ESP_ELFSYM_EXPORT(pumpkin_set_secure),
    ESP_ELFSYM_EXPORT(pumpkin_set_size),
    ESP_ELFSYM_EXPORT(pumpkin_set_spawner),
    ESP_ELFSYM_EXPORT(pumpkin_set_subdata),
    ESP_ELFSYM_EXPORT(pumpkin_set_taskbar),
    ESP_ELFSYM_EXPORT(pumpkin_set_window),
    ESP_ELFSYM_EXPORT(pumpkin_setcolor),
    ESP_ELFSYM_EXPORT(pumpkin_setio),
    ESP_ELFSYM_EXPORT(pumpkin_shader),
    ESP_ELFSYM_EXPORT(pumpkin_status),
    ESP_ELFSYM_EXPORT(pumpkin_sys_event),
    ESP_ELFSYM_EXPORT(pumpkin_taskbar_add),
    ESP_ELFSYM_EXPORT(pumpkin_taskbar_add_widget),
    ESP_ELFSYM_EXPORT(pumpkin_taskbar_create),
    ESP_ELFSYM_EXPORT(pumpkin_taskbar_destroy),
    ESP_ELFSYM_EXPORT(pumpkin_taskbar_remove),
    ESP_ELFSYM_EXPORT(pumpkin_taskbar_remove_widget),
    ESP_ELFSYM_EXPORT(pumpkin_taskbar_ui),
    ESP_ELFSYM_EXPORT(pumpkin_taskbar_update),
    ESP_ELFSYM_EXPORT(pumpkin_test_exception),
    ESP_ELFSYM_EXPORT(pumpkin_trace),
    ESP_ELFSYM_EXPORT(pumpkin_vprintf),
    ESP_ELFSYM_EXPORT(pumpkin_word_serial),
    ESP_ELFSYM_EXPORT(pumpkin_write),
    ESP_ELFSYM_EXPORT(pumpkin_system_call),
    ESP_ELFSYM_EXPORT(pumpkin_deploy_files_session),
    ESP_ELFSYM_EXPORT(pumpkin_deploy_from_image),
    ESP_ELFSYM_EXPORT(pumpkin_registry_create),
    ESP_ELFSYM_EXPORT(pumpkin_remove_locks),
    ESP_ELFSYM_EXPORT(DmArchiveRecord),
    ESP_ELFSYM_EXPORT(DmAttachRecord),
    ESP_ELFSYM_EXPORT(DmAttachResource),
    ESP_ELFSYM_EXPORT(DmCloseDatabase),
    ESP_ELFSYM_EXPORT(DmCreateDatabase),
    ESP_ELFSYM_EXPORT(DmCreateDatabaseEx),
    ESP_ELFSYM_EXPORT(DmCreateDatabaseFromImage),
    ESP_ELFSYM_EXPORT(DmDatabaseInfo),
    ESP_ELFSYM_EXPORT(DmDatabaseProtect),
    ESP_ELFSYM_EXPORT(DmDatabaseSize),
    ESP_ELFSYM_EXPORT(DmDeleteCategory),
    ESP_ELFSYM_EXPORT(DmDeleteDatabase),
    ESP_ELFSYM_EXPORT(DmDeleteRecord),
    ESP_ELFSYM_EXPORT(DmDetachRecord),
    ESP_ELFSYM_EXPORT(DmDetachResource),
    ESP_ELFSYM_EXPORT(DmFindDatabase),
    ESP_ELFSYM_EXPORT(DmFindRecordByID),
    ESP_ELFSYM_EXPORT(DmFindResource),
    ESP_ELFSYM_EXPORT(DmFindResourceID),
    ESP_ELFSYM_EXPORT(DmFindResourceType),
    ESP_ELFSYM_EXPORT(DmFindSortPosition),
    ESP_ELFSYM_EXPORT(DmFindSortPosition68K),
    ESP_ELFSYM_EXPORT(DmFindSortPositionV10),
    ESP_ELFSYM_EXPORT(DmGet1Resource),
    ESP_ELFSYM_EXPORT(DmGetAppInfoID),
    ESP_ELFSYM_EXPORT(DmGetDatabase),
    ESP_ELFSYM_EXPORT(DmGetDatabaseLockState),
    ESP_ELFSYM_EXPORT(DmGetLastErr),
    ESP_ELFSYM_EXPORT(DmGetNextDatabaseByTypeCreator),
    ESP_ELFSYM_EXPORT(DmGetRecord),
    ESP_ELFSYM_EXPORT(DmGetResource),
    ESP_ELFSYM_EXPORT(DmGetResourceDecoded),
    ESP_ELFSYM_EXPORT(DmGetResourceIndex),
    ESP_ELFSYM_EXPORT(DmInit),
    ESP_ELFSYM_EXPORT(DmInsertionSort),
    ESP_ELFSYM_EXPORT(DmInsertionSort68K),
    ESP_ELFSYM_EXPORT(DmMoveCategory),
    ESP_ELFSYM_EXPORT(DmMoveRecord),
    ESP_ELFSYM_EXPORT(DmNewHandle),
    ESP_ELFSYM_EXPORT(DmNewRecord),
    ESP_ELFSYM_EXPORT(DmNewRecordEx),
    ESP_ELFSYM_EXPORT(DmNewResource),
    ESP_ELFSYM_EXPORT(DmNewResourceEx),
    ESP_ELFSYM_EXPORT(DmNextOpenDatabase),
    ESP_ELFSYM_EXPORT(DmNextOpenResDatabase),
    ESP_ELFSYM_EXPORT(DmNumDatabases),
    ESP_ELFSYM_EXPORT(DmNumRecords),
    ESP_ELFSYM_EXPORT(DmNumRecordsInCategory),
    ESP_ELFSYM_EXPORT(DmNumResources),
    ESP_ELFSYM_EXPORT(DmOpenDBNoOverlay),
    ESP_ELFSYM_EXPORT(DmOpenDatabase),
    ESP_ELFSYM_EXPORT(DmOpenDatabaseByTypeCreator),
    ESP_ELFSYM_EXPORT(DmOpenDatabaseInfo),
    ESP_ELFSYM_EXPORT(DmPositionInCategory),
    ESP_ELFSYM_EXPORT(DmQueryNextInCategory),
    ESP_ELFSYM_EXPORT(DmQueryRecord),
    ESP_ELFSYM_EXPORT(DmQuickSort),
    ESP_ELFSYM_EXPORT(DmQuickSort68K),
    ESP_ELFSYM_EXPORT(DmRecordInfo),
    ESP_ELFSYM_EXPORT(DmReleaseRecord),
    ESP_ELFSYM_EXPORT(DmReleaseResource),
    ESP_ELFSYM_EXPORT(DmRemoveRecord),
    ESP_ELFSYM_EXPORT(DmRemoveResource),
    ESP_ELFSYM_EXPORT(DmRemoveSecretRecords),
    ESP_ELFSYM_EXPORT(DmResetRecordStates),
    ESP_ELFSYM_EXPORT(DmResizeRecord),
    ESP_ELFSYM_EXPORT(DmResizeResource),
    ESP_ELFSYM_EXPORT(DmResourceInfo),
    ESP_ELFSYM_EXPORT(DmResourceLoadLib),
    ESP_ELFSYM_EXPORT(DmResourceType),
    ESP_ELFSYM_EXPORT(DmSearchRecord),
    ESP_ELFSYM_EXPORT(DmSearchResource),
    ESP_ELFSYM_EXPORT(DmSeekRecordInCategory),
    ESP_ELFSYM_EXPORT(DmSet),
    ESP_ELFSYM_EXPORT(DmSetDatabaseInfo),
    ESP_ELFSYM_EXPORT(DmSetDirty),
    ESP_ELFSYM_EXPORT(DmSetRecordInfo),
    ESP_ELFSYM_EXPORT(DmSetResourceInfo),
    ESP_ELFSYM_EXPORT(DmStrCopy),
    ESP_ELFSYM_EXPORT(DmSync),
    ESP_ELFSYM_EXPORT(DmSyncDatabase),
    ESP_ELFSYM_EXPORT(DmWrite),
    ESP_ELFSYM_EXPORT(DmWriteCheck),
    ESP_ELFSYM_EXPORT(DmWriteOrCheck),
    ESP_ELFSYM_EXPORT(FntSetAppearance),
    ESP_ELFSYM_EXPORT(MemCardFormat),
    ESP_ELFSYM_EXPORT(MemCardInfo),
    ESP_ELFSYM_EXPORT(MemChunkFree),
    ESP_ELFSYM_EXPORT(MemChunkNew),
    ESP_ELFSYM_EXPORT(MemCmp),
    ESP_ELFSYM_EXPORT(MemDebugMode),
    ESP_ELFSYM_EXPORT(MemHandleCardNo),
    ESP_ELFSYM_EXPORT(MemHandleDataStorage),
    ESP_ELFSYM_EXPORT(MemHandleFlags),
    ESP_ELFSYM_EXPORT(MemHandleFree),
    ESP_ELFSYM_EXPORT(MemHandleHeapID),
    ESP_ELFSYM_EXPORT(MemHandleLock),
    ESP_ELFSYM_EXPORT(MemHandleLockCount),
    ESP_ELFSYM_EXPORT(MemHandleLockEx),
    ESP_ELFSYM_EXPORT(MemHandleNew),
    ESP_ELFSYM_EXPORT(MemHandleOwner),
    ESP_ELFSYM_EXPORT(MemHandleResetLock),
    ESP_ELFSYM_EXPORT(MemHandleResize),
    ESP_ELFSYM_EXPORT(MemHandleSetOwner),
    ESP_ELFSYM_EXPORT(MemHandleSize),
    ESP_ELFSYM_EXPORT(MemHandleToLocalID),
    ESP_ELFSYM_EXPORT(MemHandleUnlock),
    ESP_ELFSYM_EXPORT(MemHandleUnlockEx),
    ESP_ELFSYM_EXPORT(MemHeapCheck),
    ESP_ELFSYM_EXPORT(MemHeapCompact),
    ESP_ELFSYM_EXPORT(MemHeapDynamic),
    ESP_ELFSYM_EXPORT(MemHeapFlags),
    ESP_ELFSYM_EXPORT(MemHeapFreeByOwnerID),
    ESP_ELFSYM_EXPORT(MemHeapFreeBytes),
    ESP_ELFSYM_EXPORT(MemHeapID),
    ESP_ELFSYM_EXPORT(MemHeapInit),
    ESP_ELFSYM_EXPORT(MemHeapScramble),
    ESP_ELFSYM_EXPORT(MemHeapSize),
    ESP_ELFSYM_EXPORT(MemInit),
    ESP_ELFSYM_EXPORT(MemInitHeapTable),
    ESP_ELFSYM_EXPORT(MemKernelInit),
    ESP_ELFSYM_EXPORT(MemLocalIDKind),
    ESP_ELFSYM_EXPORT(MemLocalIDToGlobal),
    ESP_ELFSYM_EXPORT(MemLocalIDToHandle),
    ESP_ELFSYM_EXPORT(MemLocalIDToLockedPtr),
    ESP_ELFSYM_EXPORT(MemLocalIDToPtr),
    ESP_ELFSYM_EXPORT(MemMove),
    ESP_ELFSYM_EXPORT(MemNumCards),
    ESP_ELFSYM_EXPORT(MemNumHeaps),
    ESP_ELFSYM_EXPORT(MemNumRAMHeaps),
    ESP_ELFSYM_EXPORT(MemPtrCardNo),
    ESP_ELFSYM_EXPORT(MemPtrDataStorage),
    ESP_ELFSYM_EXPORT(MemPtrFlags),
    ESP_ELFSYM_EXPORT(MemPtrHeapID),
    ESP_ELFSYM_EXPORT(MemPtrNew),
    ESP_ELFSYM_EXPORT(MemPtrOwner),
    ESP_ELFSYM_EXPORT(MemPtrRecoverHandle),
    ESP_ELFSYM_EXPORT(MemPtrResetLock),
    ESP_ELFSYM_EXPORT(MemPtrResize),
    ESP_ELFSYM_EXPORT(MemPtrSetOwner),
    ESP_ELFSYM_EXPORT(MemPtrSize),
    ESP_ELFSYM_EXPORT(MemPtrToLocalID),
    ESP_ELFSYM_EXPORT(MemPtrUnlock),
    ESP_ELFSYM_EXPORT(MemSemaphoreRelease),
    ESP_ELFSYM_EXPORT(MemSemaphoreReserve),
    ESP_ELFSYM_EXPORT(MemSet),
    ESP_ELFSYM_EXPORT(MemSetDebugMode),
    ESP_ELFSYM_EXPORT(MemStoreInfo),
    ESP_ELFSYM_EXPORT(MemStoreSetInfo),
    ESP_ELFSYM_EXPORT(StoFileRead),
    ESP_ELFSYM_EXPORT(StoFileSeek),
    ESP_ELFSYM_EXPORT(StoFileWrite),
    ESP_ELFSYM_EXPORT(StoFinish),
    ESP_ELFSYM_EXPORT(StoHeapWalk),
    ESP_ELFSYM_EXPORT(StoInit),
    ESP_ELFSYM_EXPORT(StoNewDecodedResource),
    ESP_ELFSYM_EXPORT(StoRefresh),
    ESP_ELFSYM_EXPORT(SysCreateDataBaseList),
    ESP_ELFSYM_EXPORT(SysCreateDataBaseList68K),
    ESP_ELFSYM_EXPORT(VFSFileDBInfo),
    ESP_ELFSYM_EXPORT(pumpkin_script_finish_env),
    ESP_ELFSYM_EXPORT(pumpkin_script_init_env),
    ESP_ELFSYM_EXPORT(pumpkin_script_main),
    ESP_ELFSYM_EXPORT(AppSeedFill),
    ESP_ELFSYM_EXPORT(AbtShowAbout),
    ESP_ELFSYM_EXPORT(AbtShowAboutEx),
    ESP_ELFSYM_EXPORT(AbtShowAboutPumpkin),
    ESP_ELFSYM_EXPORT(AddrDBSort),
    ESP_ELFSYM_EXPORT(AddrJDBSort),
    ESP_ELFSYM_EXPORT(AlmAlarmCallback),
    ESP_ELFSYM_EXPORT(AlmCancelAll),
    ESP_ELFSYM_EXPORT(AlmDisplayAlarm),
    ESP_ELFSYM_EXPORT(AlmEnableNotification),
    ESP_ELFSYM_EXPORT(AlmGetAlarm),
    ESP_ELFSYM_EXPORT(AlmInit),
    ESP_ELFSYM_EXPORT(AlmSetAlarm),
    ESP_ELFSYM_EXPORT(AlmTimeChange),
    ESP_ELFSYM_EXPORT(AttnDoSpecialEffects),
    ESP_ELFSYM_EXPORT(AttnForgetIt),
    ESP_ELFSYM_EXPORT(AttnGetAttention),
    ESP_ELFSYM_EXPORT(AttnGetCounts),
    ESP_ELFSYM_EXPORT(AttnIndicatorEnable),
    ESP_ELFSYM_EXPORT(AttnIndicatorEnabled),
    ESP_ELFSYM_EXPORT(AttnIterate),
    ESP_ELFSYM_EXPORT(AttnListOpen),
    ESP_ELFSYM_EXPORT(AttnUpdate),
    ESP_ELFSYM_EXPORT(BmpBitsSize),
    ESP_ELFSYM_EXPORT(BmpColortableSize),
    ESP_ELFSYM_EXPORT(BmpCompress),
    ESP_ELFSYM_EXPORT(BmpConvertFrom16Bits),
    ESP_ELFSYM_EXPORT(BmpConvertFrom1Bit),
    ESP_ELFSYM_EXPORT(BmpConvertFrom24Bits),
    ESP_ELFSYM_EXPORT(BmpConvertFrom2Bits),
    ESP_ELFSYM_EXPORT(BmpConvertFrom4Bits),
    ESP_ELFSYM_EXPORT(BmpConvertFrom8Bits),
    ESP_ELFSYM_EXPORT(BmpCopyBit),
    ESP_ELFSYM_EXPORT(BmpCreate),
    ESP_ELFSYM_EXPORT(BmpCreate3),
    ESP_ELFSYM_EXPORT(BmpCreateBitmapV3),
    ESP_ELFSYM_EXPORT(BmpCreateSurface),
    ESP_ELFSYM_EXPORT(BmpCreateSurfaceBitmap),
    ESP_ELFSYM_EXPORT(BmpDecompressBitmap),
    ESP_ELFSYM_EXPORT(BmpDecompressBitmapChain),
    ESP_ELFSYM_EXPORT(BmpDelete),
    ESP_ELFSYM_EXPORT(BmpDrawSurface),
    ESP_ELFSYM_EXPORT(BmpExportFont),
    ESP_ELFSYM_EXPORT(BmpFlip),
    ESP_ELFSYM_EXPORT(BmpGetBestBitmap),
    ESP_ELFSYM_EXPORT(BmpGetBestBitmapEx),
    ESP_ELFSYM_EXPORT(BmpGetBitDepth),
    ESP_ELFSYM_EXPORT(BmpGetBits),
    ESP_ELFSYM_EXPORT(BmpGetColortable),
    ESP_ELFSYM_EXPORT(BmpGetCompressionType),
    ESP_ELFSYM_EXPORT(BmpGetDensity),
    ESP_ELFSYM_EXPORT(BmpGetDescr),
    ESP_ELFSYM_EXPORT(BmpGetDimensions),
    ESP_ELFSYM_EXPORT(BmpGetLittleEndianBits),
    ESP_ELFSYM_EXPORT(BmpGetNextBitmap),
    ESP_ELFSYM_EXPORT(BmpGetNextBitmapAnyDensity),
    ESP_ELFSYM_EXPORT(BmpGetNoDither),
    ESP_ELFSYM_EXPORT(BmpGetPixel),
    ESP_ELFSYM_EXPORT(BmpGetPixelRGB),
    ESP_ELFSYM_EXPORT(BmpGetPixelValue),
    ESP_ELFSYM_EXPORT(BmpGetSetCommonField),
    ESP_ELFSYM_EXPORT(BmpGetSizes),
    ESP_ELFSYM_EXPORT(BmpGetTransparentValue),
    ESP_ELFSYM_EXPORT(BmpGetVersion),
    ESP_ELFSYM_EXPORT(BmpIndexToRGB),
    ESP_ELFSYM_EXPORT(BmpLittleEndian),
    ESP_ELFSYM_EXPORT(BmpPutBit),
    ESP_ELFSYM_EXPORT(BmpRGBToIndex),
    ESP_ELFSYM_EXPORT(BmpRotate),
    ESP_ELFSYM_EXPORT(BmpSetBitDepth),
    ESP_ELFSYM_EXPORT(BmpSetDensity),
    ESP_ELFSYM_EXPORT(BmpSetLittleEndianBits),
    ESP_ELFSYM_EXPORT(BmpSetPixel),
    ESP_ELFSYM_EXPORT(BmpSetTransparentValue),
    ESP_ELFSYM_EXPORT(BmpSize),
    ESP_ELFSYM_EXPORT(BmpV0GetSetField),
    ESP_ELFSYM_EXPORT(BmpV1GetSetField),
    ESP_ELFSYM_EXPORT(BmpV2GetSetField),
    ESP_ELFSYM_EXPORT(BmpV3GetSetField),
    ESP_ELFSYM_EXPORT(CharAttrFinishModule),
    ESP_ELFSYM_EXPORT(CharAttrInitModule),
    ESP_ELFSYM_EXPORT(GetCharCaselessValue),
    ESP_ELFSYM_EXPORT(GetCharSortValue),
    ESP_ELFSYM_EXPORT(CtbCompare),
    ESP_ELFSYM_EXPORT(CtbGetEntry),
    ESP_ELFSYM_EXPORT(CtbGetNumEntries),
    ESP_ELFSYM_EXPORT(CtbSetEntry),
    ESP_ELFSYM_EXPORT(CtbSetNumEntries),
    ESP_ELFSYM_EXPORT(BtLibAddrAToBtd),
    ESP_ELFSYM_EXPORT(BtLibCancelInquiry),
    ESP_ELFSYM_EXPORT(BtLibClose),
    ESP_ELFSYM_EXPORT(BtLibGetGeneralPreference),
    ESP_ELFSYM_EXPORT(BtLibGetSelectedDevices),
    ESP_ELFSYM_EXPORT(BtLibHandleEvent),
    ESP_ELFSYM_EXPORT(BtLibHandleTransportEvent),
    ESP_ELFSYM_EXPORT(BtLibLinkConnect),
    ESP_ELFSYM_EXPORT(BtLibLinkDisconnect),
    ESP_ELFSYM_EXPORT(BtLibLinkGetState),
    ESP_ELFSYM_EXPORT(BtLibLinkSetState),
    ESP_ELFSYM_EXPORT(BtLibOpen),
    ESP_ELFSYM_EXPORT(BtLibPiconetCreate),
    ESP_ELFSYM_EXPORT(BtLibPiconetDestroy),
    ESP_ELFSYM_EXPORT(BtLibPiconetLockInbound),
    ESP_ELFSYM_EXPORT(BtLibPiconetUnlockInbound),
    ESP_ELFSYM_EXPORT(BtLibRegisterManagementNotification),
    ESP_ELFSYM_EXPORT(BtLibSdpCompareUuids),
    ESP_ELFSYM_EXPORT(BtLibSdpServiceRecordCreate),
    ESP_ELFSYM_EXPORT(BtLibSdpServiceRecordDestroy),
    ESP_ELFSYM_EXPORT(BtLibSdpServiceRecordStartAdvertising),
    ESP_ELFSYM_EXPORT(BtLibSdpServiceRecordStopAdvertising),
    ESP_ELFSYM_EXPORT(BtLibSecurityFindTrustedDeviceRecord),
    ESP_ELFSYM_EXPORT(BtLibSecurityNumTrustedDeviceRecords),
    ESP_ELFSYM_EXPORT(BtLibSecurityRemoveTrustedDeviceRecord),
    ESP_ELFSYM_EXPORT(BtLibServiceClose),
    ESP_ELFSYM_EXPORT(BtLibServiceIndicateSessionStart),
    ESP_ELFSYM_EXPORT(BtLibServiceOpen),
    ESP_ELFSYM_EXPORT(BtLibServicePlaySound),
    ESP_ELFSYM_EXPORT(BtLibSetGeneralPreference),
    ESP_ELFSYM_EXPORT(BtLibSleep),
    ESP_ELFSYM_EXPORT(BtLibSocketAdvanceCredit),
    ESP_ELFSYM_EXPORT(BtLibSocketClose),
    ESP_ELFSYM_EXPORT(BtLibSocketConnect),
    ESP_ELFSYM_EXPORT(BtLibSocketListen),
    ESP_ELFSYM_EXPORT(BtLibSocketRespondToConnection),
    ESP_ELFSYM_EXPORT(BtLibSocketSend),
    ESP_ELFSYM_EXPORT(BtLibStartInquiry),
    ESP_ELFSYM_EXPORT(BtLibUnregisterManagementNotification),
    ESP_ELFSYM_EXPORT(BtLibWake),
    ESP_ELFSYM_EXPORT(CategoryCreateList),
    ESP_ELFSYM_EXPORT(CategoryCreateListV10),
    ESP_ELFSYM_EXPORT(CategoryEdit),
    ESP_ELFSYM_EXPORT(CategoryEditV10),
    ESP_ELFSYM_EXPORT(CategoryEditV20),
    ESP_ELFSYM_EXPORT(CategoryFind),
    ESP_ELFSYM_EXPORT(CategoryFreeList),
    ESP_ELFSYM_EXPORT(CategoryFreeListV10),
    ESP_ELFSYM_EXPORT(CategoryGetName),
    ESP_ELFSYM_EXPORT(CategoryGetNext),
    ESP_ELFSYM_EXPORT(CategoryInitialize),
    ESP_ELFSYM_EXPORT(CategorySelect),
    ESP_ELFSYM_EXPORT(CategorySelectV10),
    ESP_ELFSYM_EXPORT(CategorySetName),
    ESP_ELFSYM_EXPORT(CategorySetTriggerLabel),
    ESP_ELFSYM_EXPORT(CategoryTruncateName),
    ESP_ELFSYM_EXPORT(ClipboardAddItem),
    ESP_ELFSYM_EXPORT(ClipboardAppendItem),
    ESP_ELFSYM_EXPORT(ClipboardGetItem),
    ESP_ELFSYM_EXPORT(ClpFinishModule),
    ESP_ELFSYM_EXPORT(ClpInitModule),
    ESP_ELFSYM_EXPORT(CncAddProfile),
    ESP_ELFSYM_EXPORT(CncDeleteProfile),
    ESP_ELFSYM_EXPORT(CncGetProfileInfo),
    ESP_ELFSYM_EXPORT(CncGetProfileList),
    ESP_ELFSYM_EXPORT(CncProfileCloseDB),
    ESP_ELFSYM_EXPORT(CncProfileCount),
    ESP_ELFSYM_EXPORT(CncProfileCreate),
    ESP_ELFSYM_EXPORT(CncProfileDelete),
    ESP_ELFSYM_EXPORT(CncProfileGetCurrent),
    ESP_ELFSYM_EXPORT(CncProfileGetIDFromIndex),
    ESP_ELFSYM_EXPORT(CncProfileGetIDFromName),
    ESP_ELFSYM_EXPORT(CncProfileGetIndex),
    ESP_ELFSYM_EXPORT(CncProfileOpenDB),
    ESP_ELFSYM_EXPORT(CncProfileSetCurrent),
    ESP_ELFSYM_EXPORT(CncProfileSettingGet),
    ESP_ELFSYM_EXPORT(CncProfileSettingSet),
    ESP_ELFSYM_EXPORT(ConGetS),
    ESP_ELFSYM_EXPORT(ConPutS),
    ESP_ELFSYM_EXPORT(CtlDrawControl),
    ESP_ELFSYM_EXPORT(CtlEnabled),
    ESP_ELFSYM_EXPORT(CtlEraseControl),
    ESP_ELFSYM_EXPORT(CtlGetGraphics),
    ESP_ELFSYM_EXPORT(CtlGetLabel),
    ESP_ELFSYM_EXPORT(CtlGetSliderValues),
    ESP_ELFSYM_EXPORT(CtlGetValue),
    ESP_ELFSYM_EXPORT(CtlHandleEvent),
    ESP_ELFSYM_EXPORT(CtlHideControl),
    ESP_ELFSYM_EXPORT(CtlHitControl),
    ESP_ELFSYM_EXPORT(CtlNewControl),
    ESP_ELFSYM_EXPORT(CtlNewGraphicControl),
    ESP_ELFSYM_EXPORT(CtlNewSliderControl),
    ESP_ELFSYM_EXPORT(CtlSetEnabled),
    ESP_ELFSYM_EXPORT(CtlSetGraphics),
    ESP_ELFSYM_EXPORT(CtlSetLabel),
    ESP_ELFSYM_EXPORT(CtlSetSliderValues),
    ESP_ELFSYM_EXPORT(CtlSetUsable),
    ESP_ELFSYM_EXPORT(CtlSetValue),
    ESP_ELFSYM_EXPORT(CtlShowControl),
    ESP_ELFSYM_EXPORT(CtlUpdateCheckboxGroup),
    ESP_ELFSYM_EXPORT(CtlUpdateGroup),
    ESP_ELFSYM_EXPORT(CtlValidatePointer),
    ESP_ELFSYM_EXPORT(CPMLibAddRandomSeed),
    ESP_ELFSYM_EXPORT(CPMLibClose),
    ESP_ELFSYM_EXPORT(CPMLibDecrypt),
    ESP_ELFSYM_EXPORT(CPMLibDecryptFinal),
    ESP_ELFSYM_EXPORT(CPMLibDecryptInit),
    ESP_ELFSYM_EXPORT(CPMLibDecryptUpdate),
    ESP_ELFSYM_EXPORT(CPMLibEncrypt),
    ESP_ELFSYM_EXPORT(CPMLibEncryptFinal),
    ESP_ELFSYM_EXPORT(CPMLibEncryptInit),
    ESP_ELFSYM_EXPORT(CPMLibEncryptUpdate),
    ESP_ELFSYM_EXPORT(CPMLibEnumerateProviders),
    ESP_ELFSYM_EXPORT(CPMLibExportCipherInfo),
    ESP_ELFSYM_EXPORT(CPMLibExportHashInfo),
    ESP_ELFSYM_EXPORT(CPMLibExportKeyInfo),
    ESP_ELFSYM_EXPORT(CPMLibExportVerifyInfo),
    ESP_ELFSYM_EXPORT(CPMLibGenerateKey),
    ESP_ELFSYM_EXPORT(CPMLibGenerateRandomBytes),
    ESP_ELFSYM_EXPORT(CPMLibGetInfo),
    ESP_ELFSYM_EXPORT(CPMLibGetProviderInfo),
    ESP_ELFSYM_EXPORT(CPMLibHash),
    ESP_ELFSYM_EXPORT(CPMLibHashFinal),
    ESP_ELFSYM_EXPORT(CPMLibHashInit),
    ESP_ELFSYM_EXPORT(CPMLibHashUpdate),
    ESP_ELFSYM_EXPORT(CPMLibImportCipherInfo),
    ESP_ELFSYM_EXPORT(CPMLibImportHashInfo),
    ESP_ELFSYM_EXPORT(CPMLibImportKeyInfo),
    ESP_ELFSYM_EXPORT(CPMLibImportVerifyInfo),
    ESP_ELFSYM_EXPORT(CPMLibOpen),
    ESP_ELFSYM_EXPORT(CPMLibReleaseCipherInfo),
    ESP_ELFSYM_EXPORT(CPMLibReleaseHashInfo),
    ESP_ELFSYM_EXPORT(CPMLibReleaseKeyInfo),
    ESP_ELFSYM_EXPORT(CPMLibReleaseVerifyInfo),
    ESP_ELFSYM_EXPORT(CPMLibSetDebugLevel),
    ESP_ELFSYM_EXPORT(CPMLibSetDefaultProvider),
    ESP_ELFSYM_EXPORT(CPMLibSleep),
    ESP_ELFSYM_EXPORT(CPMLibTest),
    ESP_ELFSYM_EXPORT(CPMLibVerify),
    ESP_ELFSYM_EXPORT(CPMLibVerifyFinal),
    ESP_ELFSYM_EXPORT(CPMLibVerifyInit),
    ESP_ELFSYM_EXPORT(CPMLibVerifyUpdate),
    ESP_ELFSYM_EXPORT(CPMLibWake),
    ESP_ELFSYM_EXPORT(Crc16CalcBigBlock),
    ESP_ELFSYM_EXPORT(Crc16CalcBlock),
    ESP_ELFSYM_EXPORT(DateAdjust),
    ESP_ELFSYM_EXPORT(DateDaysToDate),
    ESP_ELFSYM_EXPORT(DateSecondsToDate),
    ESP_ELFSYM_EXPORT(DateTemplateToAscii),
    ESP_ELFSYM_EXPORT(DateToAscii),
    ESP_ELFSYM_EXPORT(DateToDOWDMFormat),
    ESP_ELFSYM_EXPORT(DateToDays),
    ESP_ELFSYM_EXPORT(DayOfMonth),
    ESP_ELFSYM_EXPORT(DayOfWeek),
    ESP_ELFSYM_EXPORT(DaysInMonth),
    ESP_ELFSYM_EXPORT(TimAdjust),
    ESP_ELFSYM_EXPORT(TimDateTimeToSeconds),
    ESP_ELFSYM_EXPORT(TimSecondsToDateTime),
    ESP_ELFSYM_EXPORT(TimTimeZoneToUTC),
    ESP_ELFSYM_EXPORT(TimUTCToTimeZone),
    ESP_ELFSYM_EXPORT(TimeToAscii),
    ESP_ELFSYM_EXPORT(TimeZoneToAscii),
    ESP_ELFSYM_EXPORT(DayDrawDaySelector),
    ESP_ELFSYM_EXPORT(DayDrawDays),
    ESP_ELFSYM_EXPORT(DayHandleEvent),
    ESP_ELFSYM_EXPORT(DbgBreak),
    ESP_ELFSYM_EXPORT(DbgCommSettings),
    ESP_ELFSYM_EXPORT(DbgGetMessage),
    ESP_ELFSYM_EXPORT(DbgInit),
    ESP_ELFSYM_EXPORT(DbgMessage),
    ESP_ELFSYM_EXPORT(DbgSrcBreak),
    ESP_ELFSYM_EXPORT(DbgSrcMessage),
    ESP_ELFSYM_EXPORT(DlkControl),
    ESP_ELFSYM_EXPORT(DlkDispatchRequest),
    ESP_ELFSYM_EXPORT(DlkGetSyncInfo),
    ESP_ELFSYM_EXPORT(DlkSetLogEntry),
    ESP_ELFSYM_EXPORT(DlkStartServer),
    ESP_ELFSYM_EXPORT(EncDES),
    ESP_ELFSYM_EXPORT(EncDigestMD4),
    ESP_ELFSYM_EXPORT(EncDigestMD5),
    ESP_ELFSYM_EXPORT(md5Finalize),
    ESP_ELFSYM_EXPORT(md5Init),
    ESP_ELFSYM_EXPORT(md5Step),
    ESP_ELFSYM_EXPORT(md5Update),
    ESP_ELFSYM_EXPORT(rotateLeft),
    ESP_ELFSYM_EXPORT(SHA1),
    ESP_ELFSYM_EXPORT(SHA1Final),
    ESP_ELFSYM_EXPORT(SHA1Init),
    ESP_ELFSYM_EXPORT(SHA1Transform),
    ESP_ELFSYM_EXPORT(SHA1Update),
    ESP_ELFSYM_EXPORT(ErrAlertCustom),
    ESP_ELFSYM_EXPORT(ErrDisplayFileLineMsg),
    ESP_ELFSYM_EXPORT(ErrDisplayFileLineMsgEx),
    ESP_ELFSYM_EXPORT(ErrExceptionList),
    ESP_ELFSYM_EXPORT(ErrThrow),
    ESP_ELFSYM_EXPORT(EvtAddEventToQueue),
    ESP_ELFSYM_EXPORT(EvtAddUniqueEventToQueue),
    ESP_ELFSYM_EXPORT(EvtCopyEvent),
    ESP_ELFSYM_EXPORT(EvtEmptyQueue),
    ESP_ELFSYM_EXPORT(EvtEnqueueKey),
    ESP_ELFSYM_EXPORT(EvtEventAvail),
    ESP_ELFSYM_EXPORT(EvtFinishModule),
    ESP_ELFSYM_EXPORT(EvtFlushPenQueue),
    ESP_ELFSYM_EXPORT(EvtGetEvent),
    ESP_ELFSYM_EXPORT(EvtGetEventName),
    ESP_ELFSYM_EXPORT(EvtGetEventUs),
    ESP_ELFSYM_EXPORT(EvtGetPen),
    ESP_ELFSYM_EXPORT(EvtGetPenEx),
    ESP_ELFSYM_EXPORT(EvtInitModule),
    ESP_ELFSYM_EXPORT(EvtKeyEventAvail),
    ESP_ELFSYM_EXPORT(EvtPrintEvent),
    ESP_ELFSYM_EXPORT(EvtPumpEvents),
    ESP_ELFSYM_EXPORT(EvtReturnPenMove),
    ESP_ELFSYM_EXPORT(EvtSetNullEventTick),
    ESP_ELFSYM_EXPORT(EvtSysEventAvail),
    ESP_ELFSYM_EXPORT(ExgLibAccept),
    ESP_ELFSYM_EXPORT(ExgLibClose),
    ESP_ELFSYM_EXPORT(ExgLibConnect),
    ESP_ELFSYM_EXPORT(ExgLibControl),
    ESP_ELFSYM_EXPORT(ExgLibDisconnect),
    ESP_ELFSYM_EXPORT(ExgLibGet),
    ESP_ELFSYM_EXPORT(ExgLibHandleEvent),
    ESP_ELFSYM_EXPORT(ExgLibOpen),
    ESP_ELFSYM_EXPORT(ExgLibPut),
    ESP_ELFSYM_EXPORT(ExgLibReceive),
    ESP_ELFSYM_EXPORT(ExgLibRequest),
    ESP_ELFSYM_EXPORT(ExgLibSend),
    ESP_ELFSYM_EXPORT(ExgLibSleep),
    ESP_ELFSYM_EXPORT(ExgLibWake),
    ESP_ELFSYM_EXPORT(ExgAccept),
    ESP_ELFSYM_EXPORT(ExgConnect),
    ESP_ELFSYM_EXPORT(ExgControl),
    ESP_ELFSYM_EXPORT(ExgDBRead),
    ESP_ELFSYM_EXPORT(ExgDBWrite),
    ESP_ELFSYM_EXPORT(ExgDisconnect),
    ESP_ELFSYM_EXPORT(ExgDoDialog),
    ESP_ELFSYM_EXPORT(ExgGet),
    ESP_ELFSYM_EXPORT(ExgGetDefaultApplication),
    ESP_ELFSYM_EXPORT(ExgGetRegisteredApplications),
    ESP_ELFSYM_EXPORT(ExgGetRegisteredTypes),
    ESP_ELFSYM_EXPORT(ExgGetTargetApplication),
    ESP_ELFSYM_EXPORT(ExgInit),
    ESP_ELFSYM_EXPORT(ExgNotifyGoto),
    ESP_ELFSYM_EXPORT(ExgNotifyPreview),
    ESP_ELFSYM_EXPORT(ExgNotifyReceive),
    ESP_ELFSYM_EXPORT(ExgNotifyReceiveV35),
    ESP_ELFSYM_EXPORT(ExgPut),
    ESP_ELFSYM_EXPORT(ExgReceive),
    ESP_ELFSYM_EXPORT(ExgRegisterData),
    ESP_ELFSYM_EXPORT(ExgRegisterDatatype),
    ESP_ELFSYM_EXPORT(ExgRequest),
    ESP_ELFSYM_EXPORT(ExgSend),
    ESP_ELFSYM_EXPORT(ExgSetDefaultApplication),
    ESP_ELFSYM_EXPORT(ExpCardGetSerialPort),
    ESP_ELFSYM_EXPORT(ExpCardInfo),
    ESP_ELFSYM_EXPORT(ExpCardInserted),
    ESP_ELFSYM_EXPORT(ExpCardPresent),
    ESP_ELFSYM_EXPORT(ExpCardRemoved),
    ESP_ELFSYM_EXPORT(ExpInit),
    ESP_ELFSYM_EXPORT(ExpSlotDriverInstall),
    ESP_ELFSYM_EXPORT(ExpSlotDriverRemove),
    ESP_ELFSYM_EXPORT(ExpSlotEnumerate),
    ESP_ELFSYM_EXPORT(ExpSlotLibFind),
    ESP_ELFSYM_EXPORT(ExpSlotRegister),
    ESP_ELFSYM_EXPORT(ExpSlotUnregister),
    ESP_ELFSYM_EXPORT(SysFatalAlert),
    ESP_ELFSYM_EXPORT(SysFatalAlertFinish),
    ESP_ELFSYM_EXPORT(SysFatalAlertInit),
    ESP_ELFSYM_EXPORT(FtrFinishModule),
    ESP_ELFSYM_EXPORT(FtrGet),
    ESP_ELFSYM_EXPORT(FtrGetByIndex),
    ESP_ELFSYM_EXPORT(FtrGetPtr),
    ESP_ELFSYM_EXPORT(FtrInit),
    ESP_ELFSYM_EXPORT(FtrInitModule),
    ESP_ELFSYM_EXPORT(FtrPtrFree),
    ESP_ELFSYM_EXPORT(FtrPtrNew),
    ESP_ELFSYM_EXPORT(FtrPtrResize),
    ESP_ELFSYM_EXPORT(FtrSet),
    ESP_ELFSYM_EXPORT(FtrUnregister),
    ESP_ELFSYM_EXPORT(FldCalcFieldHeight),
    ESP_ELFSYM_EXPORT(FldCompactText),
    ESP_ELFSYM_EXPORT(FldCopy),
    ESP_ELFSYM_EXPORT(FldCut),
    ESP_ELFSYM_EXPORT(FldDelete),
    ESP_ELFSYM_EXPORT(FldDirty),
    ESP_ELFSYM_EXPORT(FldDrawField),
    ESP_ELFSYM_EXPORT(FldEraseField),
    ESP_ELFSYM_EXPORT(FldFinishModule),
    ESP_ELFSYM_EXPORT(FldFreeMemory),
    ESP_ELFSYM_EXPORT(FldGetActiveField),
    ESP_ELFSYM_EXPORT(FldGetAttributes),
    ESP_ELFSYM_EXPORT(FldGetBounds),
    ESP_ELFSYM_EXPORT(FldGetFont),
    ESP_ELFSYM_EXPORT(FldGetInsPtPosition),
    ESP_ELFSYM_EXPORT(FldGetMaxChars),
    ESP_ELFSYM_EXPORT(FldGetNumberOfBlankLines),
    ESP_ELFSYM_EXPORT(FldGetScrollPosition),
    ESP_ELFSYM_EXPORT(FldGetScrollValues),
    ESP_ELFSYM_EXPORT(FldGetSelection),
    ESP_ELFSYM_EXPORT(FldGetTextAllocatedSize),
    ESP_ELFSYM_EXPORT(FldGetTextHandle),
    ESP_ELFSYM_EXPORT(FldGetTextHeight),
    ESP_ELFSYM_EXPORT(FldGetTextLength),
    ESP_ELFSYM_EXPORT(FldGetTextPtr),
    ESP_ELFSYM_EXPORT(FldGetVisibleLines),
    ESP_ELFSYM_EXPORT(FldGrabFocus),
    ESP_ELFSYM_EXPORT(FldHandleEvent),
    ESP_ELFSYM_EXPORT(FldInitModule),
    ESP_ELFSYM_EXPORT(FldInsert),
    ESP_ELFSYM_EXPORT(FldMakeFullyVisible),
    ESP_ELFSYM_EXPORT(FldNewField),
    ESP_ELFSYM_EXPORT(FldPaste),
    ESP_ELFSYM_EXPORT(FldRecalculateField),
    ESP_ELFSYM_EXPORT(FldReinitModule),
    ESP_ELFSYM_EXPORT(FldReleaseFocus),
    ESP_ELFSYM_EXPORT(FldReplaceText),
    ESP_ELFSYM_EXPORT(FldScrollField),
    ESP_ELFSYM_EXPORT(FldScrollable),
    ESP_ELFSYM_EXPORT(FldSendChangeNotification),
    ESP_ELFSYM_EXPORT(FldSendHeightChangeNotification),
    ESP_ELFSYM_EXPORT(FldSetActiveField),
    ESP_ELFSYM_EXPORT(FldSetAttributes),
    ESP_ELFSYM_EXPORT(FldSetBounds),
    ESP_ELFSYM_EXPORT(FldSetDirty),
    ESP_ELFSYM_EXPORT(FldSetFont),
    ESP_ELFSYM_EXPORT(FldSetInsPtPosition),
    ESP_ELFSYM_EXPORT(FldSetInsertionPoint),
    ESP_ELFSYM_EXPORT(FldSetMaxChars),
    ESP_ELFSYM_EXPORT(FldSetMaxVisibleLines),
    ESP_ELFSYM_EXPORT(FldSetScrollPosition),
    ESP_ELFSYM_EXPORT(FldSetSelection),
    ESP_ELFSYM_EXPORT(FldSetText),
    ESP_ELFSYM_EXPORT(FldSetTextAllocatedSize),
    ESP_ELFSYM_EXPORT(FldSetTextHandle),
    ESP_ELFSYM_EXPORT(FldSetTextPtr),
    ESP_ELFSYM_EXPORT(FldSetUsable),
    ESP_ELFSYM_EXPORT(FldUndo),
    ESP_ELFSYM_EXPORT(FldWordWrap),
    ESP_ELFSYM_EXPORT(FileClose),
    ESP_ELFSYM_EXPORT(FileControl),
    ESP_ELFSYM_EXPORT(FileDelete),
    ESP_ELFSYM_EXPORT(FileOpen),
    ESP_ELFSYM_EXPORT(FileReadLow),
    ESP_ELFSYM_EXPORT(FileSeek),
    ESP_ELFSYM_EXPORT(FileTell),
    ESP_ELFSYM_EXPORT(FileTruncate),
    ESP_ELFSYM_EXPORT(FileWrite),
    ESP_ELFSYM_EXPORT(Find),
    ESP_ELFSYM_EXPORT(FindDrawHeader),
    ESP_ELFSYM_EXPORT(FindGetLineBounds),
    ESP_ELFSYM_EXPORT(FindSaveMatch),
    ESP_ELFSYM_EXPORT(FindStrInStr),
    ESP_ELFSYM_EXPORT(ECFixedDiv),
    ESP_ELFSYM_EXPORT(ECFixedMul),
    ESP_ELFSYM_EXPORT(FlpAToF),
    ESP_ELFSYM_EXPORT(FlpBase10Info),
    ESP_ELFSYM_EXPORT(FlpBufferAToF),
    ESP_ELFSYM_EXPORT(FlpCorrectedAdd),
    ESP_ELFSYM_EXPORT(FlpCorrectedSub),
    ESP_ELFSYM_EXPORT(FlpFToA),
    ESP_ELFSYM_EXPORT(FlpSelectorErrPrv),
    ESP_ELFSYM_EXPORT(FlpVersion),
    ESP_ELFSYM_EXPORT(_d_add),
    ESP_ELFSYM_EXPORT(_d_div),
    ESP_ELFSYM_EXPORT(_d_dtof),
    ESP_ELFSYM_EXPORT(_d_dtoi),
    ESP_ELFSYM_EXPORT(_d_dtou),
    ESP_ELFSYM_EXPORT(_d_feq),
    ESP_ELFSYM_EXPORT(_d_fge),
    ESP_ELFSYM_EXPORT(_d_fgt),
    ESP_ELFSYM_EXPORT(_d_fle),
    ESP_ELFSYM_EXPORT(_d_flt),
    ESP_ELFSYM_EXPORT(_d_fne),
    ESP_ELFSYM_EXPORT(_d_itod),
    ESP_ELFSYM_EXPORT(_d_mul),
    ESP_ELFSYM_EXPORT(_d_neg),
    ESP_ELFSYM_EXPORT(_d_sub),
    ESP_ELFSYM_EXPORT(_d_utod),
    ESP_ELFSYM_EXPORT(_f_add),
    ESP_ELFSYM_EXPORT(_f_div),
    ESP_ELFSYM_EXPORT(_f_feq),
    ESP_ELFSYM_EXPORT(_f_fge),
    ESP_ELFSYM_EXPORT(_f_fgt),
    ESP_ELFSYM_EXPORT(_f_fle),
    ESP_ELFSYM_EXPORT(_f_flt),
    ESP_ELFSYM_EXPORT(_f_fne),
    ESP_ELFSYM_EXPORT(_f_ftod),
    ESP_ELFSYM_EXPORT(_f_ftoi),
    ESP_ELFSYM_EXPORT(_f_ftou),
    ESP_ELFSYM_EXPORT(_f_itof),
    ESP_ELFSYM_EXPORT(_f_mul),
    ESP_ELFSYM_EXPORT(_f_neg),
    ESP_ELFSYM_EXPORT(_f_sub),
    ESP_ELFSYM_EXPORT(_f_utof),
    ESP_ELFSYM_EXPORT(_fp_get_fpscr),
    ESP_ELFSYM_EXPORT(_fp_round),
    ESP_ELFSYM_EXPORT(_fp_set_fpscr),
    ESP_ELFSYM_EXPORT(FntAverageCharWidth),
    ESP_ELFSYM_EXPORT(FntBaseLine),
    ESP_ELFSYM_EXPORT(FntCharHeight),
    ESP_ELFSYM_EXPORT(FntCharWidth),
    ESP_ELFSYM_EXPORT(FntCharsInWidth),
    ESP_ELFSYM_EXPORT(FntCharsWidth),
    ESP_ELFSYM_EXPORT(FntCopyFont),
    ESP_ELFSYM_EXPORT(FntDefineFont),
    ESP_ELFSYM_EXPORT(FntDescenderHeight),
    ESP_ELFSYM_EXPORT(FntDrawChar),
    ESP_ELFSYM_EXPORT(FntFinishModule),
    ESP_ELFSYM_EXPORT(FntFontCharWidth),
    ESP_ELFSYM_EXPORT(FntFreeFont),
    ESP_ELFSYM_EXPORT(FntGetDensity),
    ESP_ELFSYM_EXPORT(FntGetDensityCount),
    ESP_ELFSYM_EXPORT(FntGetFont),
    ESP_ELFSYM_EXPORT(FntGetFontPtr),
    ESP_ELFSYM_EXPORT(FntGetScrollValues),
    ESP_ELFSYM_EXPORT(FntGetVersion),
    ESP_ELFSYM_EXPORT(FntInitModule),
    ESP_ELFSYM_EXPORT(FntLineHeight),
    ESP_ELFSYM_EXPORT(FntLineWidth),
    ESP_ELFSYM_EXPORT(FntLoadFont),
    ESP_ELFSYM_EXPORT(FntReinitModule),
    ESP_ELFSYM_EXPORT(FntSaveFont),
    ESP_ELFSYM_EXPORT(FntSaveFontEx),
    ESP_ELFSYM_EXPORT(FntSaveFonts),
    ESP_ELFSYM_EXPORT(FntSetFont),
    ESP_ELFSYM_EXPORT(FntWCharWidth),
    ESP_ELFSYM_EXPORT(FntWidthToOffset),
    ESP_ELFSYM_EXPORT(FntWordWrap),
    ESP_ELFSYM_EXPORT(FntWordWrapReverseNLines),
    ESP_ELFSYM_EXPORT(pumpkin_create_font),
    ESP_ELFSYM_EXPORT(pumpkin_create_fontv2),
    ESP_ELFSYM_EXPORT(pumpkin_destroy_font),
    ESP_ELFSYM_EXPORT(pumpkin_destroy_fontv2),
    ESP_ELFSYM_EXPORT(FontSelect),
    ESP_ELFSYM_EXPORT(FrmActiveState),
    ESP_ELFSYM_EXPORT(FrmAddSpaceForObject),
    ESP_ELFSYM_EXPORT(FrmAlert),
    ESP_ELFSYM_EXPORT(FrmCenterDialogs),
    ESP_ELFSYM_EXPORT(FrmCloseAllForms),
    ESP_ELFSYM_EXPORT(FrmCopyLabel),
    ESP_ELFSYM_EXPORT(FrmCopyTitle),
    ESP_ELFSYM_EXPORT(FrmCustomAlert),
    ESP_ELFSYM_EXPORT(FrmCustomResponseAlert),
    ESP_ELFSYM_EXPORT(FrmDeleteForm),
    ESP_ELFSYM_EXPORT(FrmDispatchEvent),
    ESP_ELFSYM_EXPORT(FrmDoDialog),
    ESP_ELFSYM_EXPORT(FrmDoDialogEx),
    ESP_ELFSYM_EXPORT(FrmDrawEmptyDialog),
    ESP_ELFSYM_EXPORT(FrmDrawForm),
    ESP_ELFSYM_EXPORT(FrmDrawObject),
    ESP_ELFSYM_EXPORT(FrmEraseForm),
    ESP_ELFSYM_EXPORT(FrmEraseObject),
    ESP_ELFSYM_EXPORT(FrmFinishModule),
    ESP_ELFSYM_EXPORT(FrmGetActiveField),
    ESP_ELFSYM_EXPORT(FrmGetActiveForm),
    ESP_ELFSYM_EXPORT(FrmGetActiveFormID),
    ESP_ELFSYM_EXPORT(FrmGetCenterDialogs),
    ESP_ELFSYM_EXPORT(FrmGetControlGroupSelection),
    ESP_ELFSYM_EXPORT(FrmGetControlValue),
    ESP_ELFSYM_EXPORT(FrmGetDIAPolicyAttr),
    ESP_ELFSYM_EXPORT(FrmGetFirstForm),
    ESP_ELFSYM_EXPORT(FrmGetFocus),
    ESP_ELFSYM_EXPORT(FrmGetFormBounds),
    ESP_ELFSYM_EXPORT(FrmGetFormId),
    ESP_ELFSYM_EXPORT(FrmGetFormPtr),
    ESP_ELFSYM_EXPORT(FrmGetGadgetData),
    ESP_ELFSYM_EXPORT(FrmGetLabel),
    ESP_ELFSYM_EXPORT(FrmGetNumberOfObjects),
    ESP_ELFSYM_EXPORT(FrmGetObjectBounds),
    ESP_ELFSYM_EXPORT(FrmGetObjectId),
    ESP_ELFSYM_EXPORT(FrmGetObjectIndex),
    ESP_ELFSYM_EXPORT(FrmGetObjectIndexFromPtr),
    ESP_ELFSYM_EXPORT(FrmGetObjectPosition),
    ESP_ELFSYM_EXPORT(FrmGetObjectPtr),
    ESP_ELFSYM_EXPORT(FrmGetObjectType),
    ESP_ELFSYM_EXPORT(FrmGetTitle),
    ESP_ELFSYM_EXPORT(FrmGetUsable),
    ESP_ELFSYM_EXPORT(FrmGetUserModifiedState),
    ESP_ELFSYM_EXPORT(FrmGetVisible),
    ESP_ELFSYM_EXPORT(FrmGetWindowHandle),
    ESP_ELFSYM_EXPORT(FrmGotoForm),
    ESP_ELFSYM_EXPORT(FrmHandleEvent),
    ESP_ELFSYM_EXPORT(FrmHelp),
    ESP_ELFSYM_EXPORT(FrmHideObject),
    ESP_ELFSYM_EXPORT(FrmInitForm),
    ESP_ELFSYM_EXPORT(FrmInitModule),
    ESP_ELFSYM_EXPORT(FrmNavObjectTakeFocus),
    ESP_ELFSYM_EXPORT(FrmNewBitmap),
    ESP_ELFSYM_EXPORT(FrmNewForm),
    ESP_ELFSYM_EXPORT(FrmNewGadget),
    ESP_ELFSYM_EXPORT(FrmNewGsi),
    ESP_ELFSYM_EXPORT(FrmNewLabel),
    ESP_ELFSYM_EXPORT(FrmObjectBottomAlign),
    ESP_ELFSYM_EXPORT(FrmObjectRightAlign),
    ESP_ELFSYM_EXPORT(FrmPointInTitle),
    ESP_ELFSYM_EXPORT(FrmPopupForm),
    ESP_ELFSYM_EXPORT(FrmReinitModule),
    ESP_ELFSYM_EXPORT(FrmRemoveObject),
    ESP_ELFSYM_EXPORT(FrmReturnToForm),
    ESP_ELFSYM_EXPORT(FrmSaveAllForms),
    ESP_ELFSYM_EXPORT(FrmSetActiveForm),
    ESP_ELFSYM_EXPORT(FrmSetCategoryLabel),
    ESP_ELFSYM_EXPORT(FrmSetColorTrigger),
    ESP_ELFSYM_EXPORT(FrmSetControlGroupSelection),
    ESP_ELFSYM_EXPORT(FrmSetControlValue),
    ESP_ELFSYM_EXPORT(FrmSetDIAPolicyAttr),
    ESP_ELFSYM_EXPORT(FrmSetEventHandler),
    ESP_ELFSYM_EXPORT(FrmSetFocus),
    ESP_ELFSYM_EXPORT(FrmSetFormBounds),
    ESP_ELFSYM_EXPORT(FrmSetGadgetData),
    ESP_ELFSYM_EXPORT(FrmSetGadgetHandler),
    ESP_ELFSYM_EXPORT(FrmSetMenu),
    ESP_ELFSYM_EXPORT(FrmSetNotUserModified),
    ESP_ELFSYM_EXPORT(FrmSetObjectBounds),
    ESP_ELFSYM_EXPORT(FrmSetObjectPosition),
    ESP_ELFSYM_EXPORT(FrmSetObjectPtr),
    ESP_ELFSYM_EXPORT(FrmSetTitle),
    ESP_ELFSYM_EXPORT(FrmSetUsable),
    ESP_ELFSYM_EXPORT(FrmSetVisible),
    ESP_ELFSYM_EXPORT(FrmShowObject),
    ESP_ELFSYM_EXPORT(FrmTrackPenUp),
    ESP_ELFSYM_EXPORT(FrmUpdateForm),
    ESP_ELFSYM_EXPORT(FrmUpdateScrollers),
    ESP_ELFSYM_EXPORT(FrmValidatePtr),
    ESP_ELFSYM_EXPORT(FrmVisible),
    ESP_ELFSYM_EXPORT(pumpkin_create_alert),
    ESP_ELFSYM_EXPORT(pumpkin_create_form),
    ESP_ELFSYM_EXPORT(pumpkin_destroy_alert),
    ESP_ELFSYM_EXPORT(pumpkin_destroy_form),
    ESP_ELFSYM_EXPORT(pumpkin_fix_popups),
    ESP_ELFSYM_EXPORT(FSDirCreate),
    ESP_ELFSYM_EXPORT(FSDirEntryEnumerate),
    ESP_ELFSYM_EXPORT(FSFileClose),
    ESP_ELFSYM_EXPORT(FSFileCreate),
    ESP_ELFSYM_EXPORT(FSFileDelete),
    ESP_ELFSYM_EXPORT(FSFileEOF),
    ESP_ELFSYM_EXPORT(FSFileGetAttributes),
    ESP_ELFSYM_EXPORT(FSFileGetDate),
    ESP_ELFSYM_EXPORT(FSFileRename),
    ESP_ELFSYM_EXPORT(FSFileResize),
    ESP_ELFSYM_EXPORT(FSFileSeek),
    ESP_ELFSYM_EXPORT(FSFileSetAttributes),
    ESP_ELFSYM_EXPORT(FSFileSetDate),
    ESP_ELFSYM_EXPORT(FSFileSize),
    ESP_ELFSYM_EXPORT(FSFileTell),
    ESP_ELFSYM_EXPORT(FSFilesystemType),
    ESP_ELFSYM_EXPORT(FSLibAPIVersion),
    ESP_ELFSYM_EXPORT(FSLibClose),
    ESP_ELFSYM_EXPORT(FSLibOpen),
    ESP_ELFSYM_EXPORT(FSLibSleep),
    ESP_ELFSYM_EXPORT(FSLibWake),
    ESP_ELFSYM_EXPORT(FSVolumeFormat),
    ESP_ELFSYM_EXPORT(FSVolumeGetLabel),
    ESP_ELFSYM_EXPORT(FSVolumeInfo),
    ESP_ELFSYM_EXPORT(FSVolumeMount),
    ESP_ELFSYM_EXPORT(FSVolumeSetLabel),
    ESP_ELFSYM_EXPORT(FSVolumeSize),
    ESP_ELFSYM_EXPORT(FSVolumeUnmount),
    ESP_ELFSYM_EXPORT(GrfAddMacro),
    ESP_ELFSYM_EXPORT(GrfAddPoint),
    ESP_ELFSYM_EXPORT(GrfBeginStroke),
    ESP_ELFSYM_EXPORT(GrfCleanState),
    ESP_ELFSYM_EXPORT(GrfDeleteMacro),
    ESP_ELFSYM_EXPORT(GrfFieldChange),
    ESP_ELFSYM_EXPORT(GrfFilterPoints),
    ESP_ELFSYM_EXPORT(GrfFindBranch),
    ESP_ELFSYM_EXPORT(GrfFlushPoints),
    ESP_ELFSYM_EXPORT(GrfFree),
    ESP_ELFSYM_EXPORT(GrfGetAndExpandMacro),
    ESP_ELFSYM_EXPORT(GrfGetGlyphMapping),
    ESP_ELFSYM_EXPORT(GrfGetMacro),
    ESP_ELFSYM_EXPORT(GrfGetMacroName),
    ESP_ELFSYM_EXPORT(GrfGetNumPoints),
    ESP_ELFSYM_EXPORT(GrfGetPoint),
    ESP_ELFSYM_EXPORT(GrfGetState),
    ESP_ELFSYM_EXPORT(GrfInit),
    ESP_ELFSYM_EXPORT(GrfInitState),
    ESP_ELFSYM_EXPORT(GrfMatch),
    ESP_ELFSYM_EXPORT(GrfMatchGlyph),
    ESP_ELFSYM_EXPORT(GrfProcessStroke),
    ESP_ELFSYM_EXPORT(GrfSetState),
    ESP_ELFSYM_EXPORT(SysGraffitiReferenceDialog),
    ESP_ELFSYM_EXPORT(GsiEnable),
    ESP_ELFSYM_EXPORT(GsiEnabled),
    ESP_ELFSYM_EXPORT(GsiInitialize),
    ESP_ELFSYM_EXPORT(GsiSetLocation),
    ESP_ELFSYM_EXPORT(GsiSetShiftState),
    ESP_ELFSYM_EXPORT(HwrCustom),
    ESP_ELFSYM_EXPORT(HostAscTime),
    ESP_ELFSYM_EXPORT(HostCTime),
    ESP_ELFSYM_EXPORT(HostClock),
    ESP_ELFSYM_EXPORT(HostCloseDir),
    ESP_ELFSYM_EXPORT(HostDbgClearDataBreak),
    ESP_ELFSYM_EXPORT(HostDbgSetDataBreak),
    ESP_ELFSYM_EXPORT(HostErrNo),
    ESP_ELFSYM_EXPORT(HostExgLibAccept),
    ESP_ELFSYM_EXPORT(HostExgLibClose),
    ESP_ELFSYM_EXPORT(HostExgLibConnect),
    ESP_ELFSYM_EXPORT(HostExgLibControl),
    ESP_ELFSYM_EXPORT(HostExgLibDisconnect),
    ESP_ELFSYM_EXPORT(HostExgLibGet),
    ESP_ELFSYM_EXPORT(HostExgLibHandleEvent),
    ESP_ELFSYM_EXPORT(HostExgLibOpen),
    ESP_ELFSYM_EXPORT(HostExgLibPut),
    ESP_ELFSYM_EXPORT(HostExgLibReceive),
    ESP_ELFSYM_EXPORT(HostExgLibRequest),
    ESP_ELFSYM_EXPORT(HostExgLibSend),
    ESP_ELFSYM_EXPORT(HostExgLibSleep),
    ESP_ELFSYM_EXPORT(HostExgLibWake),
    ESP_ELFSYM_EXPORT(HostExportFile),
    ESP_ELFSYM_EXPORT(HostFClose),
    ESP_ELFSYM_EXPORT(HostFEOF),
    ESP_ELFSYM_EXPORT(HostFError),
    ESP_ELFSYM_EXPORT(HostFFlush),
    ESP_ELFSYM_EXPORT(HostFGetC),
    ESP_ELFSYM_EXPORT(HostFGetPos),
    ESP_ELFSYM_EXPORT(HostFGetS),
    ESP_ELFSYM_EXPORT(HostFOpen),
    ESP_ELFSYM_EXPORT(HostFPrintF),
    ESP_ELFSYM_EXPORT(HostFPutC),
    ESP_ELFSYM_EXPORT(HostFPutS),
    ESP_ELFSYM_EXPORT(HostFRead),
    ESP_ELFSYM_EXPORT(HostFReopen),
    ESP_ELFSYM_EXPORT(HostFScanF),
    ESP_ELFSYM_EXPORT(HostFSeek),
    ESP_ELFSYM_EXPORT(HostFSetPos),
    ESP_ELFSYM_EXPORT(HostFTell),
    ESP_ELFSYM_EXPORT(HostFWrite),
    ESP_ELFSYM_EXPORT(HostFree),
    ESP_ELFSYM_EXPORT(HostGMTime),
    ESP_ELFSYM_EXPORT(HostGestalt),
    ESP_ELFSYM_EXPORT(HostGetDirectory),
    ESP_ELFSYM_EXPORT(HostGetEnv),
    ESP_ELFSYM_EXPORT(HostGetFile),
    ESP_ELFSYM_EXPORT(HostGetFileAttr),
    ESP_ELFSYM_EXPORT(HostGetHostID),
    ESP_ELFSYM_EXPORT(HostGetHostPlatform),
    ESP_ELFSYM_EXPORT(HostGetHostVersion),
    ESP_ELFSYM_EXPORT(HostGetPreference),
    ESP_ELFSYM_EXPORT(HostGremlinCounter),
    ESP_ELFSYM_EXPORT(HostGremlinIsRunning),
    ESP_ELFSYM_EXPORT(HostGremlinLimit),
    ESP_ELFSYM_EXPORT(HostGremlinNew),
    ESP_ELFSYM_EXPORT(HostGremlinNumber),
    ESP_ELFSYM_EXPORT(HostImportFile),
    ESP_ELFSYM_EXPORT(HostImportFileWithID),
    ESP_ELFSYM_EXPORT(HostIsCallingTrap),
    ESP_ELFSYM_EXPORT(HostIsSelectorImplemented),
    ESP_ELFSYM_EXPORT(HostLocalTime),
    ESP_ELFSYM_EXPORT(HostLogFile),
    ESP_ELFSYM_EXPORT(HostMalloc),
    ESP_ELFSYM_EXPORT(HostMkDir),
    ESP_ELFSYM_EXPORT(HostMkTime),
    ESP_ELFSYM_EXPORT(HostOpenDir),
    ESP_ELFSYM_EXPORT(HostProfileCleanup),
    ESP_ELFSYM_EXPORT(HostProfileDetailFn),
    ESP_ELFSYM_EXPORT(HostProfileDump),
    ESP_ELFSYM_EXPORT(HostProfileGetCycles),
    ESP_ELFSYM_EXPORT(HostProfileInit),
    ESP_ELFSYM_EXPORT(HostProfileStart),
    ESP_ELFSYM_EXPORT(HostProfileStop),
    ESP_ELFSYM_EXPORT(HostPutFile),
    ESP_ELFSYM_EXPORT(HostReadDir),
    ESP_ELFSYM_EXPORT(HostRealloc),
    ESP_ELFSYM_EXPORT(HostRemove),
    ESP_ELFSYM_EXPORT(HostRename),
    ESP_ELFSYM_EXPORT(HostRmDir),
    ESP_ELFSYM_EXPORT(HostSaveScreen),
    ESP_ELFSYM_EXPORT(HostSessionClose),
    ESP_ELFSYM_EXPORT(HostSessionCreate),
    ESP_ELFSYM_EXPORT(HostSessionOpen),
    ESP_ELFSYM_EXPORT(HostSessionQuit),
    ESP_ELFSYM_EXPORT(HostSessionSave),
    ESP_ELFSYM_EXPORT(HostSetFileAttr),
    ESP_ELFSYM_EXPORT(HostSetLogFileSize),
    ESP_ELFSYM_EXPORT(HostSetPreference),
    ESP_ELFSYM_EXPORT(HostSignalResume),
    ESP_ELFSYM_EXPORT(HostSignalSend),
    ESP_ELFSYM_EXPORT(HostSignalWait),
    ESP_ELFSYM_EXPORT(HostSlotHasCard),
    ESP_ELFSYM_EXPORT(HostSlotMax),
    ESP_ELFSYM_EXPORT(HostSlotRoot),
    ESP_ELFSYM_EXPORT(HostStat),
    ESP_ELFSYM_EXPORT(HostStrFTime),
    ESP_ELFSYM_EXPORT(HostTime),
    ESP_ELFSYM_EXPORT(HostTmpFile),
    ESP_ELFSYM_EXPORT(HostTmpNam),
    ESP_ELFSYM_EXPORT(HostTraceClose),
    ESP_ELFSYM_EXPORT(HostTraceInit),
    ESP_ELFSYM_EXPORT(HostTraceOutputB),
    ESP_ELFSYM_EXPORT(HostTraceOutputT),
    ESP_ELFSYM_EXPORT(HostTraceOutputTL),
    ESP_ELFSYM_EXPORT(HostTraceOutputVT),
    ESP_ELFSYM_EXPORT(HostTraceOutputVTL),
    ESP_ELFSYM_EXPORT(HostTruncate),
    ESP_ELFSYM_EXPORT(HostUTime),
    ESP_ELFSYM_EXPORT(ImcStringIsAscii),
    ESP_ELFSYM_EXPORT(INetLibCachePurge),
    ESP_ELFSYM_EXPORT(INetLibCheckAntennaState),
    ESP_ELFSYM_EXPORT(INetLibClose),
    ESP_ELFSYM_EXPORT(INetLibConfigDelete),
    ESP_ELFSYM_EXPORT(INetLibPrepareCacheForHistory),
    ESP_ELFSYM_EXPORT(INetLibSleep),
    ESP_ELFSYM_EXPORT(INetLibSockClose),
    ESP_ELFSYM_EXPORT(INetLibURLCrack),
    ESP_ELFSYM_EXPORT(INetLibURLsCompare),
    ESP_ELFSYM_EXPORT(INetLibWake),
    ESP_ELFSYM_EXPORT(INetLibWiCmd),
    ESP_ELFSYM_EXPORT(INetLibWirelessIndicatorCmd),
    ESP_ELFSYM_EXPORT(InsPtCheckBlink),
    ESP_ELFSYM_EXPORT(InsPtEnable),
    ESP_ELFSYM_EXPORT(InsPtEnabled),
    ESP_ELFSYM_EXPORT(InsPtFinishModule),
    ESP_ELFSYM_EXPORT(InsPtGetHeight),
    ESP_ELFSYM_EXPORT(InsPtGetLocation),
    ESP_ELFSYM_EXPORT(InsPtInitModule),
    ESP_ELFSYM_EXPORT(InsPtInitialize),
    ESP_ELFSYM_EXPORT(InsPtSetHeight),
    ESP_ELFSYM_EXPORT(InsPtSetLocation),
    ESP_ELFSYM_EXPORT(IntlGetRoutineAddress),
    ESP_ELFSYM_EXPORT(IntlSetRoutineAddress),
    ESP_ELFSYM_EXPORT(IrBind),
    ESP_ELFSYM_EXPORT(IrClose),
    ESP_ELFSYM_EXPORT(IrConnectIrLap),
    ESP_ELFSYM_EXPORT(IrConnectReq),
    ESP_ELFSYM_EXPORT(IrConnectRsp),
    ESP_ELFSYM_EXPORT(IrDataReq),
    ESP_ELFSYM_EXPORT(IrDisconnectIrLap),
    ESP_ELFSYM_EXPORT(IrDiscoverReq),
    ESP_ELFSYM_EXPORT(IrHandleEvent),
    ESP_ELFSYM_EXPORT(IrIsIrLapConnected),
    ESP_ELFSYM_EXPORT(IrIsMediaBusy),
    ESP_ELFSYM_EXPORT(IrIsNoProgress),
    ESP_ELFSYM_EXPORT(IrIsRemoteBusy),
    ESP_ELFSYM_EXPORT(IrLocalBusy),
    ESP_ELFSYM_EXPORT(IrMaxRxSize),
    ESP_ELFSYM_EXPORT(IrMaxTxSize),
    ESP_ELFSYM_EXPORT(IrOpen),
    ESP_ELFSYM_EXPORT(IrSetDeviceInfo),
    ESP_ELFSYM_EXPORT(IrTestReq),
    ESP_ELFSYM_EXPORT(IrUnbind),
    ESP_ELFSYM_EXPORT(IrWaitForEvent),
    ESP_ELFSYM_EXPORT(KbdDraw),
    ESP_ELFSYM_EXPORT(KbdDrawKeyboard),
    ESP_ELFSYM_EXPORT(KbdErase),
    ESP_ELFSYM_EXPORT(KbdGetLayout),
    ESP_ELFSYM_EXPORT(KbdGetPosition),
    ESP_ELFSYM_EXPORT(KbdGetShiftState),
    ESP_ELFSYM_EXPORT(KbdGrfGetState),
    ESP_ELFSYM_EXPORT(KbdGrfSetState),
    ESP_ELFSYM_EXPORT(KbdHandleEvent),
    ESP_ELFSYM_EXPORT(KbdSetLayout),
    ESP_ELFSYM_EXPORT(KbdSetPosition),
    ESP_ELFSYM_EXPORT(KbdSetShiftState),
    ESP_ELFSYM_EXPORT(KeyboardFinishModule),
    ESP_ELFSYM_EXPORT(KeyboardInitModule),
    ESP_ELFSYM_EXPORT(KeyboardStatusFree),
    ESP_ELFSYM_EXPORT(KeyboardStatusNew),
    ESP_ELFSYM_EXPORT(SysKeyboardDialog),
    ESP_ELFSYM_EXPORT(SysKeyboardDialogV10),
    ESP_ELFSYM_EXPORT(KeyCurrentState),
    ESP_ELFSYM_EXPORT(KeyFinishModule),
    ESP_ELFSYM_EXPORT(KeyInitModule),
    ESP_ELFSYM_EXPORT(KeyRates),
    ESP_ELFSYM_EXPORT(KeySetMask),
    ESP_ELFSYM_EXPORT(SysAppLauncherDialog),
    ESP_ELFSYM_EXPORT(LstDrawList),
    ESP_ELFSYM_EXPORT(LstEraseList),
    ESP_ELFSYM_EXPORT(LstFreeListChoices),
    ESP_ELFSYM_EXPORT(LstGetNumberOfItems),
    ESP_ELFSYM_EXPORT(LstGetSelection),
    ESP_ELFSYM_EXPORT(LstGetSelectionText),
    ESP_ELFSYM_EXPORT(LstGetTopItem),
    ESP_ELFSYM_EXPORT(LstGetVisibleItems),
    ESP_ELFSYM_EXPORT(LstHandleEvent),
    ESP_ELFSYM_EXPORT(LstMakeItemVisible),
    ESP_ELFSYM_EXPORT(LstNewList),
    ESP_ELFSYM_EXPORT(LstNewListEx),
    ESP_ELFSYM_EXPORT(LstPopupList),
    ESP_ELFSYM_EXPORT(LstScrollList),
    ESP_ELFSYM_EXPORT(LstSetDrawFunction),
    ESP_ELFSYM_EXPORT(LstSetHeight),
    ESP_ELFSYM_EXPORT(LstSetListChoices),
    ESP_ELFSYM_EXPORT(LstSetPosition),
    ESP_ELFSYM_EXPORT(LstSetSelection),
    ESP_ELFSYM_EXPORT(LstSetTopItem),
    ESP_ELFSYM_EXPORT(LmGetLocaleSetting),
    ESP_ELFSYM_EXPORT(LmGetNumLocales),
    ESP_ELFSYM_EXPORT(LmLocaleToIndex),
    ESP_ELFSYM_EXPORT(LmTimeZoneToIndex),
    ESP_ELFSYM_EXPORT(LocGetNumberSeparators),
    ESP_ELFSYM_EXPORT(Lz77LibClose),
    ESP_ELFSYM_EXPORT(Lz77LibOpen),
    ESP_ELFSYM_EXPORT(Lz77LibSleep),
    ESP_ELFSYM_EXPORT(Lz77LibWake),
    ESP_ELFSYM_EXPORT(MenuAddItem),
    ESP_ELFSYM_EXPORT(MenuCmdBarAddButton),
    ESP_ELFSYM_EXPORT(MenuCmdBarDisplay),
    ESP_ELFSYM_EXPORT(MenuCmdBarGetButtonData),
    ESP_ELFSYM_EXPORT(MenuDispose),
    ESP_ELFSYM_EXPORT(MenuDrawMenu),
    ESP_ELFSYM_EXPORT(MenuEraseStatus),
    ESP_ELFSYM_EXPORT(MenuFinishModule),
    ESP_ELFSYM_EXPORT(MenuGetActiveMenu),
    ESP_ELFSYM_EXPORT(MenuHandleEvent),
    ESP_ELFSYM_EXPORT(MenuHideItem),
    ESP_ELFSYM_EXPORT(MenuInit),
    ESP_ELFSYM_EXPORT(MenuInitModule),
    ESP_ELFSYM_EXPORT(MenuReinitModule),
    ESP_ELFSYM_EXPORT(MenuSetActiveMenu),
    ESP_ELFSYM_EXPORT(MenuSetActiveMenuRscID),
    ESP_ELFSYM_EXPORT(MenuShowItem),
    ESP_ELFSYM_EXPORT(pumpkin_create_menu),
    ESP_ELFSYM_EXPORT(pumpkin_destroy_menu),
    ESP_ELFSYM_EXPORT(MdmDial),
    ESP_ELFSYM_EXPORT(MdmHangUp),
    ESP_ELFSYM_EXPORT(NetLibBitGetFixed),
    ESP_ELFSYM_EXPORT(NetLibBitGetIntV),
    ESP_ELFSYM_EXPORT(NetLibBitGetUIntV),
    ESP_ELFSYM_EXPORT(NetLibBitMove),
    ESP_ELFSYM_EXPORT(NetLibBitPutFixed),
    ESP_ELFSYM_EXPORT(NetLibBitPutIntV),
    ESP_ELFSYM_EXPORT(NetLibBitPutUIntV),
    ESP_ELFSYM_EXPORT(NetLibAddrAToIN),
    ESP_ELFSYM_EXPORT(NetLibAddrINToA),
    ESP_ELFSYM_EXPORT(NetLibClose),
    ESP_ELFSYM_EXPORT(NetLibConfigAliasGet),
    ESP_ELFSYM_EXPORT(NetLibConfigAliasSet),
    ESP_ELFSYM_EXPORT(NetLibConfigDelete),
    ESP_ELFSYM_EXPORT(NetLibConfigIndexFromName),
    ESP_ELFSYM_EXPORT(NetLibConfigList),
    ESP_ELFSYM_EXPORT(NetLibConfigMakeActive),
    ESP_ELFSYM_EXPORT(NetLibConfigRename),
    ESP_ELFSYM_EXPORT(NetLibConfigSaveAs),
    ESP_ELFSYM_EXPORT(NetLibConnectionRefresh),
    ESP_ELFSYM_EXPORT(NetLibDmReceive),
    ESP_ELFSYM_EXPORT(NetLibFinishCloseWait),
    ESP_ELFSYM_EXPORT(NetLibGetHostByAddr),
    ESP_ELFSYM_EXPORT(NetLibGetHostByName),
    ESP_ELFSYM_EXPORT(NetLibGetMailExchangeByName),
    ESP_ELFSYM_EXPORT(NetLibGetServByName),
    ESP_ELFSYM_EXPORT(NetLibHandlePowerOff),
    ESP_ELFSYM_EXPORT(NetLibIFAttach),
    ESP_ELFSYM_EXPORT(NetLibIFDetach),
    ESP_ELFSYM_EXPORT(NetLibIFDown),
    ESP_ELFSYM_EXPORT(NetLibIFGet),
    ESP_ELFSYM_EXPORT(NetLibIFSettingGet),
    ESP_ELFSYM_EXPORT(NetLibIFSettingSet),
    ESP_ELFSYM_EXPORT(NetLibIFUp),
    ESP_ELFSYM_EXPORT(NetLibMaster),
    ESP_ELFSYM_EXPORT(NetLibOpen),
    ESP_ELFSYM_EXPORT(NetLibOpenConfig),
    ESP_ELFSYM_EXPORT(NetLibOpenCount),
    ESP_ELFSYM_EXPORT(NetLibOpenIfCloseWait),
    ESP_ELFSYM_EXPORT(NetLibReceive),
    ESP_ELFSYM_EXPORT(NetLibReceivePB),
    ESP_ELFSYM_EXPORT(NetLibSelect),
    ESP_ELFSYM_EXPORT(NetLibSend),
    ESP_ELFSYM_EXPORT(NetLibSendPB),
    ESP_ELFSYM_EXPORT(NetLibSettingGet),
    ESP_ELFSYM_EXPORT(NetLibSettingSet),
    ESP_ELFSYM_EXPORT(NetLibSleep),
    ESP_ELFSYM_EXPORT(NetLibSocketAccept),
    ESP_ELFSYM_EXPORT(NetLibSocketAddr),
    ESP_ELFSYM_EXPORT(NetLibSocketBind),
    ESP_ELFSYM_EXPORT(NetLibSocketClose),
    ESP_ELFSYM_EXPORT(NetLibSocketConnect),
    ESP_ELFSYM_EXPORT(NetLibSocketListen),
    ESP_ELFSYM_EXPORT(NetLibSocketOpen),
    ESP_ELFSYM_EXPORT(NetLibSocketOptionGet),
    ESP_ELFSYM_EXPORT(NetLibSocketOptionSet),
    ESP_ELFSYM_EXPORT(NetLibSocketShutdown),
    ESP_ELFSYM_EXPORT(NetLibTracePrintF),
    ESP_ELFSYM_EXPORT(NetLibTracePutS),
    ESP_ELFSYM_EXPORT(NetLibWake),
    ESP_ELFSYM_EXPORT(NetUReadN),
    ESP_ELFSYM_EXPORT(NetUTCPOpen),
    ESP_ELFSYM_EXPORT(NetUWriteN),
    ESP_ELFSYM_EXPORT(netFDClr),
    ESP_ELFSYM_EXPORT(netFDIsSet),
    ESP_ELFSYM_EXPORT(netFDSet),
    ESP_ELFSYM_EXPORT(netFDZero),
    ESP_ELFSYM_EXPORT(OmGetCurrentLocale),
    ESP_ELFSYM_EXPORT(OmGetIndexedLocale),
    ESP_ELFSYM_EXPORT(OmGetNextSystemLocale),
    ESP_ELFSYM_EXPORT(OmGetRoutineAddress),
    ESP_ELFSYM_EXPORT(OmGetSystemLocale),
    ESP_ELFSYM_EXPORT(OmLocaleToOverlayDBName),
    ESP_ELFSYM_EXPORT(OmOverlayDBNameToLocale),
    ESP_ELFSYM_EXPORT(OmSetSystemLocale),
    ESP_ELFSYM_EXPORT(pumpkin_create_overlay),
    ESP_ELFSYM_EXPORT(pumpkin_destroy_overlay),
    ESP_ELFSYM_EXPORT(PwdExists),
    ESP_ELFSYM_EXPORT(PwdRemove),
    ESP_ELFSYM_EXPORT(PwdSet),
    ESP_ELFSYM_EXPORT(PwdVerify),
    ESP_ELFSYM_EXPORT(PceNativeCall),
    ESP_ELFSYM_EXPORT(PdiDefineReaderDictionary),
    ESP_ELFSYM_EXPORT(PdiDefineResizing),
    ESP_ELFSYM_EXPORT(PdiDefineWriterDictionary),
    ESP_ELFSYM_EXPORT(PdiEnterObject),
    ESP_ELFSYM_EXPORT(PdiLibClose),
    ESP_ELFSYM_EXPORT(PdiLibOpen),
    ESP_ELFSYM_EXPORT(PdiReadParameter),
    ESP_ELFSYM_EXPORT(PdiReadProperty),
    ESP_ELFSYM_EXPORT(PdiReadPropertyField),
    ESP_ELFSYM_EXPORT(PdiReadPropertyName),
    ESP_ELFSYM_EXPORT(PdiReaderDelete),
    ESP_ELFSYM_EXPORT(PdiReaderNew),
    ESP_ELFSYM_EXPORT(PdiSetCharset),
    ESP_ELFSYM_EXPORT(PdiSetEncoding),
    ESP_ELFSYM_EXPORT(PdiWriteBeginObject),
    ESP_ELFSYM_EXPORT(PdiWriteParameter),
    ESP_ELFSYM_EXPORT(PdiWriteParameterStr),
    ESP_ELFSYM_EXPORT(PdiWriteProperty),
    ESP_ELFSYM_EXPORT(PdiWritePropertyBinaryValue),
    ESP_ELFSYM_EXPORT(PdiWritePropertyFields),
    ESP_ELFSYM_EXPORT(PdiWritePropertyStr),
    ESP_ELFSYM_EXPORT(PdiWritePropertyValue),
    ESP_ELFSYM_EXPORT(PdiWriterDelete),
    ESP_ELFSYM_EXPORT(PdiWriterNew),
    ESP_ELFSYM_EXPORT(PINGetInputAreaState),
    ESP_ELFSYM_EXPORT(PINGetInputTriggerState),
    ESP_ELFSYM_EXPORT(PINSetInputAreaState),
    ESP_ELFSYM_EXPORT(PINSetInputTriggerState),
    ESP_ELFSYM_EXPORT(StatGetAttribute),
    ESP_ELFSYM_EXPORT(StatHide),
    ESP_ELFSYM_EXPORT(StatShow),
    ESP_ELFSYM_EXPORT(PenCalibrate),
    ESP_ELFSYM_EXPORT(PenClose),
    ESP_ELFSYM_EXPORT(PenGetRawPen),
    ESP_ELFSYM_EXPORT(PenOpen),
    ESP_ELFSYM_EXPORT(PenRawToScreen),
    ESP_ELFSYM_EXPORT(PenResetCalibration),
    ESP_ELFSYM_EXPORT(PenScreenToRaw),
    ESP_ELFSYM_EXPORT(PenSleep),
    ESP_ELFSYM_EXPORT(PenWake),
    ESP_ELFSYM_EXPORT(PhoneNumberLookup),
    ESP_ELFSYM_EXPORT(PhoneNumberLookupCustom),
    ESP_ELFSYM_EXPORT(PrefCountryCode),
    ESP_ELFSYM_EXPORT(PrefCountryName),
    ESP_ELFSYM_EXPORT(PrefFinishModule),
    ESP_ELFSYM_EXPORT(PrefGetAppPreferences),
    ESP_ELFSYM_EXPORT(PrefGetAppPreferencesV10),
    ESP_ELFSYM_EXPORT(PrefGetPreference),
    ESP_ELFSYM_EXPORT(PrefGetPreferences),
    ESP_ELFSYM_EXPORT(PrefInitModule),
    ESP_ELFSYM_EXPORT(PrefLanguageCode),
    ESP_ELFSYM_EXPORT(PrefLanguageName),
    ESP_ELFSYM_EXPORT(PrefOpenPreferenceDB),
    ESP_ELFSYM_EXPORT(PrefOpenPreferenceDBV10),
    ESP_ELFSYM_EXPORT(PrefSetAppPreferences),
    ESP_ELFSYM_EXPORT(PrefSetAppPreferencesV10),
    ESP_ELFSYM_EXPORT(PrefSetPreference),
    ESP_ELFSYM_EXPORT(PrefSetPreferences),
    ESP_ELFSYM_EXPORT(SecSelectViewStatus),
    ESP_ELFSYM_EXPORT(SecVerifyPW),
    ESP_ELFSYM_EXPORT(PrgHandleEvent),
    ESP_ELFSYM_EXPORT(PrgStartDialog),
    ESP_ELFSYM_EXPORT(PrgStartDialogV31),
    ESP_ELFSYM_EXPORT(PrgStopDialog),
    ESP_ELFSYM_EXPORT(PrgUpdateDialog),
    ESP_ELFSYM_EXPORT(RctAbsToRect),
    ESP_ELFSYM_EXPORT(RctCopyRectangle),
    ESP_ELFSYM_EXPORT(RctGetDifference),
    ESP_ELFSYM_EXPORT(RctGetIntersection),
    ESP_ELFSYM_EXPORT(RctGetUnion),
    ESP_ELFSYM_EXPORT(RctInsetRectangle),
    ESP_ELFSYM_EXPORT(RctOffsetRectangle),
    ESP_ELFSYM_EXPORT(RctPtInRectangle),
    ESP_ELFSYM_EXPORT(RctRectToAbs),
    ESP_ELFSYM_EXPORT(RctSetRectangle),
    ESP_ELFSYM_EXPORT(SclDrawScrollBar),
    ESP_ELFSYM_EXPORT(SclGetScrollBar),
    ESP_ELFSYM_EXPORT(SclHandleEvent),
    ESP_ELFSYM_EXPORT(SclSetScrollBar),
    ESP_ELFSYM_EXPORT(SelTimeFinishModule),
    ESP_ELFSYM_EXPORT(SelTimeInitModule),
    ESP_ELFSYM_EXPORT(SelectOneTime),
    ESP_ELFSYM_EXPORT(SelectTime),
    ESP_ELFSYM_EXPORT(SelectTimeV33),
    ESP_ELFSYM_EXPORT(SelectDay),
    ESP_ELFSYM_EXPORT(SelectDayV10),
    ESP_ELFSYM_EXPORT(SelectTimeZone),
    ESP_ELFSYM_EXPORT(SlkClose),
    ESP_ELFSYM_EXPORT(SlkCloseSocket),
    ESP_ELFSYM_EXPORT(SlkFlushSocket),
    ESP_ELFSYM_EXPORT(SlkOpen),
    ESP_ELFSYM_EXPORT(SlkOpenSocket),
    ESP_ELFSYM_EXPORT(SlkProcessRPC),
    ESP_ELFSYM_EXPORT(SlkReceivePacket),
    ESP_ELFSYM_EXPORT(SlkSendPacket),
    ESP_ELFSYM_EXPORT(SlkSetSocketListener),
    ESP_ELFSYM_EXPORT(SlkSocketPortID),
    ESP_ELFSYM_EXPORT(SlkSocketSetTimeout),
    ESP_ELFSYM_EXPORT(SlkSysPktDefaultResponse),
    ESP_ELFSYM_EXPORT(SerialMgrInstall),
    ESP_ELFSYM_EXPORT(SrmClearErr),
    ESP_ELFSYM_EXPORT(SrmClose),
    ESP_ELFSYM_EXPORT(SrmControl),
    ESP_ELFSYM_EXPORT(SrmCustomControl),
    ESP_ELFSYM_EXPORT(SrmExtOpen),
    ESP_ELFSYM_EXPORT(SrmExtOpenBackground),
    ESP_ELFSYM_EXPORT(SrmFinishModule),
    ESP_ELFSYM_EXPORT(SrmGetDeviceCount),
    ESP_ELFSYM_EXPORT(SrmGetDeviceInfo),
    ESP_ELFSYM_EXPORT(SrmGetStatus),
    ESP_ELFSYM_EXPORT(SrmInitModule),
    ESP_ELFSYM_EXPORT(SrmOpen),
    ESP_ELFSYM_EXPORT(SrmOpenBackground),
    ESP_ELFSYM_EXPORT(SrmPrimeWakeupHandler),
    ESP_ELFSYM_EXPORT(SrmReceive),
    ESP_ELFSYM_EXPORT(SrmReceiveCheck),
    ESP_ELFSYM_EXPORT(SrmReceiveFlush),
    ESP_ELFSYM_EXPORT(SrmReceiveWait),
    ESP_ELFSYM_EXPORT(SrmReceiveWindowClose),
    ESP_ELFSYM_EXPORT(SrmReceiveWindowOpen),
    ESP_ELFSYM_EXPORT(SrmSend),
    ESP_ELFSYM_EXPORT(SrmSendCheck),
    ESP_ELFSYM_EXPORT(SrmSendFlush),
    ESP_ELFSYM_EXPORT(SrmSendWait),
    ESP_ELFSYM_EXPORT(SrmSetReceiveBuffer),
    ESP_ELFSYM_EXPORT(SrmSetWakeupHandler),
    ESP_ELFSYM_EXPORT(SrmSleep),
    ESP_ELFSYM_EXPORT(SrmWake),
    ESP_ELFSYM_EXPORT(SerClearErr),
    ESP_ELFSYM_EXPORT(SerClose),
    ESP_ELFSYM_EXPORT(SerControl),
    ESP_ELFSYM_EXPORT(SerGetSettings),
    ESP_ELFSYM_EXPORT(SerGetStatus),
    ESP_ELFSYM_EXPORT(SerOpen),
    ESP_ELFSYM_EXPORT(SerPrimeWakeupHandler),
    ESP_ELFSYM_EXPORT(SerReceive),
    ESP_ELFSYM_EXPORT(SerReceive10),
    ESP_ELFSYM_EXPORT(SerReceiveCheck),
    ESP_ELFSYM_EXPORT(SerReceiveFlush),
    ESP_ELFSYM_EXPORT(SerReceiveISP),
    ESP_ELFSYM_EXPORT(SerReceiveWait),
    ESP_ELFSYM_EXPORT(SerReceiveWindowClose),
    ESP_ELFSYM_EXPORT(SerReceiveWindowOpen),
    ESP_ELFSYM_EXPORT(SerSend),
    ESP_ELFSYM_EXPORT(SerSend10),
    ESP_ELFSYM_EXPORT(SerSendCheck),
    ESP_ELFSYM_EXPORT(SerSendFlush),
    ESP_ELFSYM_EXPORT(SerSendWait),
    ESP_ELFSYM_EXPORT(SerSetReceiveBuffer),
    ESP_ELFSYM_EXPORT(SerSetSettings),
    ESP_ELFSYM_EXPORT(SerSleep),
    ESP_ELFSYM_EXPORT(SerWake),
    ESP_ELFSYM_EXPORT(DrvClose),
    ESP_ELFSYM_EXPORT(DrvReadChar),
    ESP_ELFSYM_EXPORT(DrvStatus),
    ESP_ELFSYM_EXPORT(DrvWriteChar),
    ESP_ELFSYM_EXPORT(VDrvClose),
    ESP_ELFSYM_EXPORT(VDrvOpen),
    ESP_ELFSYM_EXPORT(VDrvStatus),
    ESP_ELFSYM_EXPORT(SlotCardGetSerialPort),
    ESP_ELFSYM_EXPORT(SlotCardInfo),
    ESP_ELFSYM_EXPORT(SlotCardIsFilesystemSupported),
    ESP_ELFSYM_EXPORT(SlotCardLowLevelFormat),
    ESP_ELFSYM_EXPORT(SlotCardMediaType),
    ESP_ELFSYM_EXPORT(SlotCardMetrics),
    ESP_ELFSYM_EXPORT(SlotCardPresent),
    ESP_ELFSYM_EXPORT(SlotCardRelease),
    ESP_ELFSYM_EXPORT(SlotCardReserve),
    ESP_ELFSYM_EXPORT(SlotClose),
    ESP_ELFSYM_EXPORT(SlotLibAPIVersion),
    ESP_ELFSYM_EXPORT(SlotMediaType),
    ESP_ELFSYM_EXPORT(SlotOpen),
    ESP_ELFSYM_EXPORT(SlotSleep),
    ESP_ELFSYM_EXPORT(SlotWake),
    ESP_ELFSYM_EXPORT(SndCreateMidiList),
    ESP_ELFSYM_EXPORT(SndDoCmd),
    ESP_ELFSYM_EXPORT(SndFinishModule),
    ESP_ELFSYM_EXPORT(SndGetDefaultVolume),
    ESP_ELFSYM_EXPORT(SndInit),
    ESP_ELFSYM_EXPORT(SndInitModule),
    ESP_ELFSYM_EXPORT(SndInterruptSmfIrregardless),
    ESP_ELFSYM_EXPORT(SndPlayFile),
    ESP_ELFSYM_EXPORT(SndPlayResource),
    ESP_ELFSYM_EXPORT(SndPlaySmf),
    ESP_ELFSYM_EXPORT(SndPlaySmfIrregardless),
    ESP_ELFSYM_EXPORT(SndPlaySmfResource),
    ESP_ELFSYM_EXPORT(SndPlaySmfResourceIrregardless),
    ESP_ELFSYM_EXPORT(SndPlaySystemSound),
    ESP_ELFSYM_EXPORT(SndSetDefaultVolume),
    ESP_ELFSYM_EXPORT(SndStreamCreate),
    ESP_ELFSYM_EXPORT(SndStreamCreateEx),
    ESP_ELFSYM_EXPORT(SndStreamCreateExtended),
    ESP_ELFSYM_EXPORT(SndStreamDelete),
    ESP_ELFSYM_EXPORT(SndStreamDeviceControl),
    ESP_ELFSYM_EXPORT(SndStreamGetPan),
    ESP_ELFSYM_EXPORT(SndStreamGetVolume),
    ESP_ELFSYM_EXPORT(SndStreamPause),
    ESP_ELFSYM_EXPORT(SndStreamSetPan),
    ESP_ELFSYM_EXPORT(SndStreamSetVolume),
    ESP_ELFSYM_EXPORT(SndStreamStart),
    ESP_ELFSYM_EXPORT(SndStreamStop),
    ESP_ELFSYM_EXPORT(putSample),
    ESP_ELFSYM_EXPORT(SslClose),
    ESP_ELFSYM_EXPORT(SslConsume),
    ESP_ELFSYM_EXPORT(SslContextCreate),
    ESP_ELFSYM_EXPORT(SslContextDestroy),
    ESP_ELFSYM_EXPORT(SslContextGetLong),
    ESP_ELFSYM_EXPORT(SslContextGetPtr),
    ESP_ELFSYM_EXPORT(SslContextSetLong),
    ESP_ELFSYM_EXPORT(SslContextSetPtr),
    ESP_ELFSYM_EXPORT(SslFlush),
    ESP_ELFSYM_EXPORT(SslLibClose),
    ESP_ELFSYM_EXPORT(SslLibCreate),
    ESP_ELFSYM_EXPORT(SslLibDestroy),
    ESP_ELFSYM_EXPORT(SslLibGetLong),
    ESP_ELFSYM_EXPORT(SslLibGetPtr),
    ESP_ELFSYM_EXPORT(SslLibName),
    ESP_ELFSYM_EXPORT(SslLibOpen),
    ESP_ELFSYM_EXPORT(SslLibSetLong),
    ESP_ELFSYM_EXPORT(SslLibSetPtr),
    ESP_ELFSYM_EXPORT(SslLibSleep),
    ESP_ELFSYM_EXPORT(SslLibWake),
    ESP_ELFSYM_EXPORT(SslOpen),
    ESP_ELFSYM_EXPORT(StrAToI),
    ESP_ELFSYM_EXPORT(StrCaselessCompare),
    ESP_ELFSYM_EXPORT(StrCat),
    ESP_ELFSYM_EXPORT(StrChr),
    ESP_ELFSYM_EXPORT(StrCompare),
    ESP_ELFSYM_EXPORT(StrCompareAscii),
    ESP_ELFSYM_EXPORT(StrCopy),
    ESP_ELFSYM_EXPORT(StrDelocalizeNumber),
    ESP_ELFSYM_EXPORT(StrDup),
    ESP_ELFSYM_EXPORT(StrIToA),
    ESP_ELFSYM_EXPORT(StrIToH),
    ESP_ELFSYM_EXPORT(StrLen),
    ESP_ELFSYM_EXPORT(StrLocalizeNumber),
    ESP_ELFSYM_EXPORT(StrNCaselessCompare),
    ESP_ELFSYM_EXPORT(StrNCat),
    ESP_ELFSYM_EXPORT(StrNCompare),
    ESP_ELFSYM_EXPORT(StrNCompareAscii),
    ESP_ELFSYM_EXPORT(StrNCopy),
    ESP_ELFSYM_EXPORT(StrNPrintF),
    ESP_ELFSYM_EXPORT(StrPrintF),
    ESP_ELFSYM_EXPORT(StrStr),
    ESP_ELFSYM_EXPORT(StrToLower),
    ESP_ELFSYM_EXPORT(StrVNPrintF),
    ESP_ELFSYM_EXPORT(StrVPrintF),
    ESP_ELFSYM_EXPORT(EvtDequeueKeyEvent),
    ESP_ELFSYM_EXPORT(EvtDequeuePenPoint),
    ESP_ELFSYM_EXPORT(EvtDequeuePenStrokeInfo),
    ESP_ELFSYM_EXPORT(EvtEnableGraffiti),
    ESP_ELFSYM_EXPORT(EvtEnqueuePenPoint),
    ESP_ELFSYM_EXPORT(EvtFlushKeyQueue),
    ESP_ELFSYM_EXPORT(EvtFlushNextPenStroke),
    ESP_ELFSYM_EXPORT(EvtGetPenBtnList),
    ESP_ELFSYM_EXPORT(EvtGetSilkscreenAreaList),
    ESP_ELFSYM_EXPORT(EvtGetSysEvent),
    ESP_ELFSYM_EXPORT(EvtKeyQueueEmpty),
    ESP_ELFSYM_EXPORT(EvtKeyQueueSize),
    ESP_ELFSYM_EXPORT(EvtPenQueueSize),
    ESP_ELFSYM_EXPORT(EvtProcessSoftKeyStroke),
    ESP_ELFSYM_EXPORT(EvtResetAutoOffTimer),
    ESP_ELFSYM_EXPORT(EvtSetAutoOffTimer),
    ESP_ELFSYM_EXPORT(EvtSetKeyQueuePtr),
    ESP_ELFSYM_EXPORT(EvtSetPenQueuePtr),
    ESP_ELFSYM_EXPORT(EvtSysInit),
    ESP_ELFSYM_EXPORT(EvtWakeup),
    ESP_ELFSYM_EXPORT(EvtWakeupWithoutNilEvent),
    ESP_ELFSYM_EXPORT(HwrGetROMToken),
    ESP_ELFSYM_EXPORT(SysAppExit),
    ESP_ELFSYM_EXPORT(SysAppStartup),
    ESP_ELFSYM_EXPORT(SysBatteryDialog),
    ESP_ELFSYM_EXPORT(SysBatteryInfo),
    ESP_ELFSYM_EXPORT(SysBatteryInfoV20),
    ESP_ELFSYM_EXPORT(SysBroadcastActionCode),
    ESP_ELFSYM_EXPORT(SysColdBoot),
    ESP_ELFSYM_EXPORT(SysCreatePanelList),
    ESP_ELFSYM_EXPORT(SysCurAppDatabase),
    ESP_ELFSYM_EXPORT(SysDisableInts),
    ESP_ELFSYM_EXPORT(SysDoze),
    ESP_ELFSYM_EXPORT(SysEvGroupCreate),
    ESP_ELFSYM_EXPORT(SysEvGroupRead),
    ESP_ELFSYM_EXPORT(SysEvGroupSignal),
    ESP_ELFSYM_EXPORT(SysEvGroupWait),
    ESP_ELFSYM_EXPORT(SysFinishModule),
    ESP_ELFSYM_EXPORT(SysGetOSVersionString),
    ESP_ELFSYM_EXPORT(SysGetOrientation),
    ESP_ELFSYM_EXPORT(SysGetOrientationTriggerState),
    ESP_ELFSYM_EXPORT(SysGetROMToken),
    ESP_ELFSYM_EXPORT(SysGetStackInfo),
    ESP_ELFSYM_EXPORT(SysGetTrapAddress),
    ESP_ELFSYM_EXPORT(SysHandleEvent),
    ESP_ELFSYM_EXPORT(SysInit),
    ESP_ELFSYM_EXPORT(SysInitModule),
    ESP_ELFSYM_EXPORT(SysKernelInfo),
    ESP_ELFSYM_EXPORT(SysLCDBrightness),
    ESP_ELFSYM_EXPORT(SysLCDContrast),
    ESP_ELFSYM_EXPORT(SysLaunchConsole),
    ESP_ELFSYM_EXPORT(SysLibClose),
    ESP_ELFSYM_EXPORT(SysLibFind),
    ESP_ELFSYM_EXPORT(SysLibInstall),
    ESP_ELFSYM_EXPORT(SysLibLoad),
    ESP_ELFSYM_EXPORT(SysLibOpen),
    ESP_ELFSYM_EXPORT(SysLibRemove),
    ESP_ELFSYM_EXPORT(SysLibSleep),
    ESP_ELFSYM_EXPORT(SysLibTblEntry),
    ESP_ELFSYM_EXPORT(SysLibWake),
    ESP_ELFSYM_EXPORT(SysMailboxCreate),
    ESP_ELFSYM_EXPORT(SysMailboxDelete),
    ESP_ELFSYM_EXPORT(SysMailboxFlush),
    ESP_ELFSYM_EXPORT(SysMailboxSend),
    ESP_ELFSYM_EXPORT(SysMailboxWait),
    ESP_ELFSYM_EXPORT(SysNewOwnerID),
    ESP_ELFSYM_EXPORT(SysResSemaphoreCreate),
    ESP_ELFSYM_EXPORT(SysResSemaphoreDelete),
    ESP_ELFSYM_EXPORT(SysResSemaphoreRelease),
    ESP_ELFSYM_EXPORT(SysResSemaphoreReserve),
    ESP_ELFSYM_EXPORT(SysReset),
    ESP_ELFSYM_EXPORT(SysRestoreStatus),
    ESP_ELFSYM_EXPORT(SysSemaphoreCreate),
    ESP_ELFSYM_EXPORT(SysSemaphoreDelete),
    ESP_ELFSYM_EXPORT(SysSemaphoreSet),
    ESP_ELFSYM_EXPORT(SysSemaphoreSignal),
    ESP_ELFSYM_EXPORT(SysSemaphoreWait),
    ESP_ELFSYM_EXPORT(SysSetA5),
    ESP_ELFSYM_EXPORT(SysSetAutoOffTime),
    ESP_ELFSYM_EXPORT(SysSetOrientation),
    ESP_ELFSYM_EXPORT(SysSetOrientationTriggerState),
    ESP_ELFSYM_EXPORT(SysSetPerformance),
    ESP_ELFSYM_EXPORT(SysSetTrapAddress),
    ESP_ELFSYM_EXPORT(SysSleep),
    ESP_ELFSYM_EXPORT(SysTaskCreate),
    ESP_ELFSYM_EXPORT(SysTaskDelay),
    ESP_ELFSYM_EXPORT(SysTaskDelayMs),
    ESP_ELFSYM_EXPORT(SysTaskDelete),
    ESP_ELFSYM_EXPORT(SysTaskID),
    ESP_ELFSYM_EXPORT(SysTaskResume),
    ESP_ELFSYM_EXPORT(SysTaskSetTermProc),
    ESP_ELFSYM_EXPORT(SysTaskSuspend),
    ESP_ELFSYM_EXPORT(SysTaskSwitching),
    ESP_ELFSYM_EXPORT(SysTaskTrigger),
    ESP_ELFSYM_EXPORT(SysTaskWait),
    ESP_ELFSYM_EXPORT(SysTaskWaitClr),
    ESP_ELFSYM_EXPORT(SysTaskWake),
    ESP_ELFSYM_EXPORT(SysTicksPerSecond),
    ESP_ELFSYM_EXPORT(SysTimerCreate),
    ESP_ELFSYM_EXPORT(SysTimerDelete),
    ESP_ELFSYM_EXPORT(SysTimerRead),
    ESP_ELFSYM_EXPORT(SysTimerWrite),
    ESP_ELFSYM_EXPORT(SysTranslateKernelErr),
    ESP_ELFSYM_EXPORT(SysUIAppSwitch),
    ESP_ELFSYM_EXPORT(SysUIAppSwitchCont),
    ESP_ELFSYM_EXPORT(SysUIBusy),
    ESP_ELFSYM_EXPORT(SysUILaunch),
    ESP_ELFSYM_EXPORT(SysUnimplemented),
    ESP_ELFSYM_EXPORT(TimGetTicks),
    ESP_ELFSYM_EXPORT(TimGetTicksMs),
    ESP_ELFSYM_EXPORT(HostControl),
    ESP_ELFSYM_EXPORT(SysBinarySearch),
    ESP_ELFSYM_EXPORT(SysCopyStringResource),
    ESP_ELFSYM_EXPORT(SysErrString),
    ESP_ELFSYM_EXPORT(SysFormPointerArrayToStrings),
    ESP_ELFSYM_EXPORT(SysInsertionSort),
    ESP_ELFSYM_EXPORT(SysQSort),
    ESP_ELFSYM_EXPORT(SysQSort68k),
    ESP_ELFSYM_EXPORT(SysQSortP),
    ESP_ELFSYM_EXPORT(SysRandom),
    ESP_ELFSYM_EXPORT(SysRandom32),
    ESP_ELFSYM_EXPORT(SysStringArray),
    ESP_ELFSYM_EXPORT(SysStringByIndex),
    ESP_ELFSYM_EXPORT(SysUFinishModule),
    ESP_ELFSYM_EXPORT(SysUInitModule),
    ESP_ELFSYM_EXPORT(TblDrawTable),
    ESP_ELFSYM_EXPORT(TblEditing),
    ESP_ELFSYM_EXPORT(TblEraseTable),
    ESP_ELFSYM_EXPORT(TblFindRowData),
    ESP_ELFSYM_EXPORT(TblFindRowID),
    ESP_ELFSYM_EXPORT(TblGetBounds),
    ESP_ELFSYM_EXPORT(TblGetColumnSpacing),
    ESP_ELFSYM_EXPORT(TblGetColumnWidth),
    ESP_ELFSYM_EXPORT(TblGetCurrentField),
    ESP_ELFSYM_EXPORT(TblGetItemBounds),
    ESP_ELFSYM_EXPORT(TblGetItemFont),
    ESP_ELFSYM_EXPORT(TblGetItemInt),
    ESP_ELFSYM_EXPORT(TblGetItemPtr),
    ESP_ELFSYM_EXPORT(TblGetLastUsableRow),
    ESP_ELFSYM_EXPORT(TblGetNumberOfColumns),
    ESP_ELFSYM_EXPORT(TblGetNumberOfRows),
    ESP_ELFSYM_EXPORT(TblGetRowData),
    ESP_ELFSYM_EXPORT(TblGetRowHeight),
    ESP_ELFSYM_EXPORT(TblGetRowID),
    ESP_ELFSYM_EXPORT(TblGetSelection),
    ESP_ELFSYM_EXPORT(TblGetTopRow),
    ESP_ELFSYM_EXPORT(TblGrabFocus),
    ESP_ELFSYM_EXPORT(TblHandleEvent),
    ESP_ELFSYM_EXPORT(TblHasScrollBar),
    ESP_ELFSYM_EXPORT(TblInsertRow),
    ESP_ELFSYM_EXPORT(TblMarkRowInvalid),
    ESP_ELFSYM_EXPORT(TblMarkTableInvalid),
    ESP_ELFSYM_EXPORT(TblRedrawTable),
    ESP_ELFSYM_EXPORT(TblReleaseFocus),
    ESP_ELFSYM_EXPORT(TblRemoveRow),
    ESP_ELFSYM_EXPORT(TblRowInvalid),
    ESP_ELFSYM_EXPORT(TblRowMasked),
    ESP_ELFSYM_EXPORT(TblRowSelectable),
    ESP_ELFSYM_EXPORT(TblRowUsable),
    ESP_ELFSYM_EXPORT(TblSelectItem),
    ESP_ELFSYM_EXPORT(TblSetBounds),
    ESP_ELFSYM_EXPORT(TblSetColumnEditIndicator),
    ESP_ELFSYM_EXPORT(TblSetColumnMasked),
    ESP_ELFSYM_EXPORT(TblSetColumnSpacing),
    ESP_ELFSYM_EXPORT(TblSetColumnUsable),
    ESP_ELFSYM_EXPORT(TblSetColumnWidth),
    ESP_ELFSYM_EXPORT(TblSetCustomDrawProcedure),
    ESP_ELFSYM_EXPORT(TblSetItemFont),
    ESP_ELFSYM_EXPORT(TblSetItemInt),
    ESP_ELFSYM_EXPORT(TblSetItemPtr),
    ESP_ELFSYM_EXPORT(TblSetItemStyle),
    ESP_ELFSYM_EXPORT(TblSetLoadDataProcedure),
    ESP_ELFSYM_EXPORT(TblSetRowData),
    ESP_ELFSYM_EXPORT(TblSetRowHeight),
    ESP_ELFSYM_EXPORT(TblSetRowID),
    ESP_ELFSYM_EXPORT(TblSetRowMasked),
    ESP_ELFSYM_EXPORT(TblSetRowSelectable),
    ESP_ELFSYM_EXPORT(TblSetRowStaticHeight),
    ESP_ELFSYM_EXPORT(TblSetRowUsable),
    ESP_ELFSYM_EXPORT(TblSetSaveDataProcedure),
    ESP_ELFSYM_EXPORT(TblSetSelection),
    ESP_ELFSYM_EXPORT(TblUnhighlightSelection),
    ESP_ELFSYM_EXPORT(TelUnblockNotifications),
    ESP_ELFSYM_EXPORT(TxtByteAttr),
    ESP_ELFSYM_EXPORT(TxtCaselessCompare),
    ESP_ELFSYM_EXPORT(TxtCharAttr),
    ESP_ELFSYM_EXPORT(TxtCharBounds),
    ESP_ELFSYM_EXPORT(TxtCharEncoding),
    ESP_ELFSYM_EXPORT(TxtCharIsValid),
    ESP_ELFSYM_EXPORT(TxtCharSize),
    ESP_ELFSYM_EXPORT(TxtCharWidth),
    ESP_ELFSYM_EXPORT(TxtCharXAttr),
    ESP_ELFSYM_EXPORT(TxtCompare),
    ESP_ELFSYM_EXPORT(TxtConvertEncoding),
    ESP_ELFSYM_EXPORT(TxtEncodingName),
    ESP_ELFSYM_EXPORT(TxtFindString),
    ESP_ELFSYM_EXPORT(TxtGetChar),
    ESP_ELFSYM_EXPORT(TxtGetNextChar),
    ESP_ELFSYM_EXPORT(TxtGetPreviousChar),
    ESP_ELFSYM_EXPORT(TxtGetTruncationOffset),
    ESP_ELFSYM_EXPORT(TxtGetWordWrapOffset),
    ESP_ELFSYM_EXPORT(TxtLowerChar),
    ESP_ELFSYM_EXPORT(TxtMaxEncoding),
    ESP_ELFSYM_EXPORT(TxtNameToEncoding),
    ESP_ELFSYM_EXPORT(TxtParamString),
    ESP_ELFSYM_EXPORT(TxtReplaceStr),
    ESP_ELFSYM_EXPORT(TxtSetNextChar),
    ESP_ELFSYM_EXPORT(TxtStrEncoding),
    ESP_ELFSYM_EXPORT(TxtTransliterate),
    ESP_ELFSYM_EXPORT(TxtUpperChar),
    ESP_ELFSYM_EXPORT(TxtWordBounds),
    ESP_ELFSYM_EXPORT(TsmGetFepMode),
    ESP_ELFSYM_EXPORT(TsmSetFepMode),
    ESP_ELFSYM_EXPORT(TimGetSeconds),
    ESP_ELFSYM_EXPORT(TimInit),
    ESP_ELFSYM_EXPORT(TimSetSeconds),
    ESP_ELFSYM_EXPORT(UDAControl),
    ESP_ELFSYM_EXPORT(UDAExchangeReaderNew),
    ESP_ELFSYM_EXPORT(UDAExchangeWriterNew),
    ESP_ELFSYM_EXPORT(UDAMemoryReaderNew),
    ESP_ELFSYM_EXPORT(UIColorGetDefaultTableEntryRGB),
    ESP_ELFSYM_EXPORT(UIColorGetTableEntryIndex),
    ESP_ELFSYM_EXPORT(UIColorGetTableEntryRGB),
    ESP_ELFSYM_EXPORT(UIColorPopTable),
    ESP_ELFSYM_EXPORT(UIColorPushTable),
    ESP_ELFSYM_EXPORT(UIColorRestoreTable),
    ESP_ELFSYM_EXPORT(UIColorSaveTable),
    ESP_ELFSYM_EXPORT(UIColorSetTableEntry),
    ESP_ELFSYM_EXPORT(UicFinishModule),
    ESP_ELFSYM_EXPORT(UicInitModule),
    ESP_ELFSYM_EXPORT(UIBrightnessAdjust),
    ESP_ELFSYM_EXPORT(UIContrastAdjust),
    ESP_ELFSYM_EXPORT(UIPickColor),
    ESP_ELFSYM_EXPORT(ResLoadConstant),
    ESP_ELFSYM_EXPORT(ResLoadForm),
    ESP_ELFSYM_EXPORT(ResLoadMenu),
    ESP_ELFSYM_EXPORT(ResLoadString),
    ESP_ELFSYM_EXPORT(VFSAddVolume),
    ESP_ELFSYM_EXPORT(VFSChangeDir),
    ESP_ELFSYM_EXPORT(VFSCurrentDir),
    ESP_ELFSYM_EXPORT(VFSCustomControl),
    ESP_ELFSYM_EXPORT(VFSDirCreate),
    ESP_ELFSYM_EXPORT(VFSDirEntryEnumerate),
    ESP_ELFSYM_EXPORT(VFSExportDatabaseToFile),
    ESP_ELFSYM_EXPORT(VFSExportDatabaseToFileCustom),
    ESP_ELFSYM_EXPORT(VFSExportDatabaseToFileEx),
    ESP_ELFSYM_EXPORT(VFSFileClose),
    ESP_ELFSYM_EXPORT(VFSFileCreate),
    ESP_ELFSYM_EXPORT(VFSFileDBGetRecord),
    ESP_ELFSYM_EXPORT(VFSFileDBGetResource),
    ESP_ELFSYM_EXPORT(VFSFileDelete),
    ESP_ELFSYM_EXPORT(VFSFileEOF),
    ESP_ELFSYM_EXPORT(VFSFileGetAttributes),
    ESP_ELFSYM_EXPORT(VFSFileGetDate),
    ESP_ELFSYM_EXPORT(VFSFileGets),
    ESP_ELFSYM_EXPORT(VFSFileOpen),
    ESP_ELFSYM_EXPORT(VFSFilePrintF),
    ESP_ELFSYM_EXPORT(VFSFileRead),
    ESP_ELFSYM_EXPORT(VFSFileReadData),
    ESP_ELFSYM_EXPORT(VFSFileRename),
    ESP_ELFSYM_EXPORT(VFSFileResize),
    ESP_ELFSYM_EXPORT(VFSFileSeek),
    ESP_ELFSYM_EXPORT(VFSFileSetAttributes),
    ESP_ELFSYM_EXPORT(VFSFileSetDate),
    ESP_ELFSYM_EXPORT(VFSFileSize),
    ESP_ELFSYM_EXPORT(VFSFileTell),
    ESP_ELFSYM_EXPORT(VFSFileTruncate),
    ESP_ELFSYM_EXPORT(VFSFileVPrintF),
    ESP_ELFSYM_EXPORT(VFSFileWrite),
    ESP_ELFSYM_EXPORT(VFSFinishModule),
    ESP_ELFSYM_EXPORT(VFSGetAttributes),
    ESP_ELFSYM_EXPORT(VFSGetDefaultDirectory),
    ESP_ELFSYM_EXPORT(VFSGetMount),
    ESP_ELFSYM_EXPORT(VFSImportDatabaseFromFile),
    ESP_ELFSYM_EXPORT(VFSImportDatabaseFromFileCustom),
    ESP_ELFSYM_EXPORT(VFSImportDatabaseFromFileEx),
    ESP_ELFSYM_EXPORT(VFSInit),
    ESP_ELFSYM_EXPORT(VFSInitModule),
    ESP_ELFSYM_EXPORT(VFSInstallFSLib),
    ESP_ELFSYM_EXPORT(VFSRealPath),
    ESP_ELFSYM_EXPORT(VFSRegisterDefaultDirectory),
    ESP_ELFSYM_EXPORT(VFSRemoveFSLib),
    ESP_ELFSYM_EXPORT(VFSTmpName),
    ESP_ELFSYM_EXPORT(VFSUnregisterDefaultDirectory),
    ESP_ELFSYM_EXPORT(VFSVolumeEnumerate),
    ESP_ELFSYM_EXPORT(VFSVolumeExists),
    ESP_ELFSYM_EXPORT(VFSVolumeFormat),
    ESP_ELFSYM_EXPORT(VFSVolumeGetLabel),
    ESP_ELFSYM_EXPORT(VFSVolumeInfo),
    ESP_ELFSYM_EXPORT(VFSVolumeMount),
    ESP_ELFSYM_EXPORT(VFSVolumeSetLabel),
    ESP_ELFSYM_EXPORT(VFSVolumeSize),
    ESP_ELFSYM_EXPORT(VFSVolumeUnmount),
    ESP_ELFSYM_EXPORT(VFSExplorerCreate),
    ESP_ELFSYM_EXPORT(VFSExplorerCurrentPath),
    ESP_ELFSYM_EXPORT(VFSExplorerDestroy),
    ESP_ELFSYM_EXPORT(VFSExplorerEnter),
    ESP_ELFSYM_EXPORT(VFSExplorerHandleEvent),
    ESP_ELFSYM_EXPORT(VFSExplorerPaginate),
    ESP_ELFSYM_EXPORT(VFSExplorerRefresh),
    ESP_ELFSYM_EXPORT(VFSExplorerSelectedItem),
    ESP_ELFSYM_EXPORT(FileBrowserLibClose),
    ESP_ELFSYM_EXPORT(FileBrowserLibOpen),
    ESP_ELFSYM_EXPORT(FileBrowserLibParseFileURL),
    ESP_ELFSYM_EXPORT(FileBrowserLibShowMultiselectDialog),
    ESP_ELFSYM_EXPORT(FileBrowserLibShowOpenDialog),
    ESP_ELFSYM_EXPORT(FileBrowserLibShowSaveAsDialog),
    ESP_ELFSYM_EXPORT(FileBrowserLibSleep),
    ESP_ELFSYM_EXPORT(FileBrowserLibWake),
    ESP_ELFSYM_EXPORT(EvtGetPenNative),
    ESP_ELFSYM_EXPORT(WinAddWindow),
    ESP_ELFSYM_EXPORT(WinAdjustCoords),
    ESP_ELFSYM_EXPORT(WinAdjustCoordsInv),
    ESP_ELFSYM_EXPORT(WinBlitBitmap),
    ESP_ELFSYM_EXPORT(WinClipRectangle),
    ESP_ELFSYM_EXPORT(WinConvertToDisplay),
    ESP_ELFSYM_EXPORT(WinCopyBitmap),
    ESP_ELFSYM_EXPORT(WinCopyRectangle),
    ESP_ELFSYM_EXPORT(WinCopyWindow),
    ESP_ELFSYM_EXPORT(WinCreateBitmapWindow),
    ESP_ELFSYM_EXPORT(WinCreateOffscreenWindow),
    ESP_ELFSYM_EXPORT(WinCreateSurface),
    ESP_ELFSYM_EXPORT(WinCreateWindow),
    ESP_ELFSYM_EXPORT(WinDeleteWindow),
    ESP_ELFSYM_EXPORT(WinDirectAccessHack),
    ESP_ELFSYM_EXPORT(WinDisableWindow),
    ESP_ELFSYM_EXPORT(WinDisplayToWindowPt),
    ESP_ELFSYM_EXPORT(WinDrawBitmap),
    ESP_ELFSYM_EXPORT(WinDrawChar),
    ESP_ELFSYM_EXPORT(WinDrawCharBox),
    ESP_ELFSYM_EXPORT(WinDrawChars),
    ESP_ELFSYM_EXPORT(WinDrawGrayLine),
    ESP_ELFSYM_EXPORT(WinDrawGrayRectangleFrame),
    ESP_ELFSYM_EXPORT(WinDrawInvertedChars),
    ESP_ELFSYM_EXPORT(WinDrawLine),
    ESP_ELFSYM_EXPORT(WinDrawPixel),
    ESP_ELFSYM_EXPORT(WinDrawRectangle),
    ESP_ELFSYM_EXPORT(WinDrawRectangleFrame),
    ESP_ELFSYM_EXPORT(WinDrawTruncChars),
    ESP_ELFSYM_EXPORT(WinDrawWindowFrame),
    ESP_ELFSYM_EXPORT(WinEnableWindow),
    ESP_ELFSYM_EXPORT(WinEraseChars),
    ESP_ELFSYM_EXPORT(WinEraseLine),
    ESP_ELFSYM_EXPORT(WinErasePixel),
    ESP_ELFSYM_EXPORT(WinEraseRectangle),
    ESP_ELFSYM_EXPORT(WinEraseRectangleFrame),
    ESP_ELFSYM_EXPORT(WinEraseWindow),
    ESP_ELFSYM_EXPORT(WinFillLine),
    ESP_ELFSYM_EXPORT(WinFillRectangle),
    ESP_ELFSYM_EXPORT(WinFinishModule),
    ESP_ELFSYM_EXPORT(WinGetActiveWindow),
    ESP_ELFSYM_EXPORT(WinGetBackColor),
    ESP_ELFSYM_EXPORT(WinGetBackColorU),
    ESP_ELFSYM_EXPORT(WinGetBitmap),
    ESP_ELFSYM_EXPORT(WinGetBorderRect),
    ESP_ELFSYM_EXPORT(WinGetBounds),
    ESP_ELFSYM_EXPORT(WinGetClip),
    ESP_ELFSYM_EXPORT(WinGetColorTable),
    ESP_ELFSYM_EXPORT(WinGetCoordinateSystem),
    ESP_ELFSYM_EXPORT(WinGetDescr),
    ESP_ELFSYM_EXPORT(WinGetDisplayExtent),
    ESP_ELFSYM_EXPORT(WinGetDisplayWindow),
    ESP_ELFSYM_EXPORT(WinGetDrawMode),
    ESP_ELFSYM_EXPORT(WinGetDrawWindow),
    ESP_ELFSYM_EXPORT(WinGetDrawWindowBounds),
    ESP_ELFSYM_EXPORT(WinGetFirstWindow),
    ESP_ELFSYM_EXPORT(WinGetForeColor),
    ESP_ELFSYM_EXPORT(WinGetForeColorU),
    ESP_ELFSYM_EXPORT(WinGetFramesRectangle),
    ESP_ELFSYM_EXPORT(WinGetPalette),
    ESP_ELFSYM_EXPORT(WinGetPattern),
    ESP_ELFSYM_EXPORT(WinGetPatternType),
    ESP_ELFSYM_EXPORT(WinGetPixel),
    ESP_ELFSYM_EXPORT(WinGetPixelRGB),
    ESP_ELFSYM_EXPORT(WinGetPosition),
    ESP_ELFSYM_EXPORT(WinGetRealCoordinateSystem),
    ESP_ELFSYM_EXPORT(WinGetScalingMode),
    ESP_ELFSYM_EXPORT(WinGetSupportedDensity),
    ESP_ELFSYM_EXPORT(WinGetWindowExtent),
    ESP_ELFSYM_EXPORT(WinGetWindowFrameRect),
    ESP_ELFSYM_EXPORT(WinIndexToRGB),
    ESP_ELFSYM_EXPORT(WinInitModule),
    ESP_ELFSYM_EXPORT(WinInitializeWindow),
    ESP_ELFSYM_EXPORT(WinInvertChars),
    ESP_ELFSYM_EXPORT(WinInvertLine),
    ESP_ELFSYM_EXPORT(WinInvertPixel),
    ESP_ELFSYM_EXPORT(WinInvertRect),
    ESP_ELFSYM_EXPORT(WinInvertRectangle),
    ESP_ELFSYM_EXPORT(WinInvertRectangleFrame),
    ESP_ELFSYM_EXPORT(WinLegacyGetAddr),
    ESP_ELFSYM_EXPORT(WinLegacyRead),
    ESP_ELFSYM_EXPORT(WinLegacyWrite),
    ESP_ELFSYM_EXPORT(WinModal),
    ESP_ELFSYM_EXPORT(WinMoveWindowAddr),
    ESP_ELFSYM_EXPORT(WinPaintBitmap),
    ESP_ELFSYM_EXPORT(WinPaintBitmapEx),
    ESP_ELFSYM_EXPORT(WinPaintChar),
    ESP_ELFSYM_EXPORT(WinPaintChars),
    ESP_ELFSYM_EXPORT(WinPaintLine),
    ESP_ELFSYM_EXPORT(WinPaintLines),
    ESP_ELFSYM_EXPORT(WinPaintPixel),
    ESP_ELFSYM_EXPORT(WinPaintPixels),
    ESP_ELFSYM_EXPORT(WinPaintRectangle),
    ESP_ELFSYM_EXPORT(WinPaintRectangleFrame),
    ESP_ELFSYM_EXPORT(WinPaintRoundedRectangleFrame),
    ESP_ELFSYM_EXPORT(WinPaintTiledBitmap),
    ESP_ELFSYM_EXPORT(WinPalette),
    ESP_ELFSYM_EXPORT(WinPopDrawState),
    ESP_ELFSYM_EXPORT(WinPushDrawState),
    ESP_ELFSYM_EXPORT(WinRGBToIndex),
    ESP_ELFSYM_EXPORT(WinReinitModule),
    ESP_ELFSYM_EXPORT(WinRemoveWindow),
    ESP_ELFSYM_EXPORT(WinResetClip),
    ESP_ELFSYM_EXPORT(WinRestoreBits),
    ESP_ELFSYM_EXPORT(WinRestoreRectangle),
    ESP_ELFSYM_EXPORT(WinSaveBits),
    ESP_ELFSYM_EXPORT(WinSaveRectangle),
    ESP_ELFSYM_EXPORT(WinScaleAbsRect),
    ESP_ELFSYM_EXPORT(WinScaleCoord),
    ESP_ELFSYM_EXPORT(WinScalePoint),
    ESP_ELFSYM_EXPORT(WinScaleRectangle),
    ESP_ELFSYM_EXPORT(WinScreenGetAttribute),
    ESP_ELFSYM_EXPORT(WinScreenInit),
    ESP_ELFSYM_EXPORT(WinScreenLock),
    ESP_ELFSYM_EXPORT(WinScreenMode),
    ESP_ELFSYM_EXPORT(WinScreenUnlock),
    ESP_ELFSYM_EXPORT(WinScrollRectangle),
    ESP_ELFSYM_EXPORT(WinSendWindowEvents),
    ESP_ELFSYM_EXPORT(WinSetActiveWindow),
    ESP_ELFSYM_EXPORT(WinSetAsciiText),
    ESP_ELFSYM_EXPORT(WinSetBackColor),
    ESP_ELFSYM_EXPORT(WinSetBackColorRGB),
    ESP_ELFSYM_EXPORT(WinSetBounds),
    ESP_ELFSYM_EXPORT(WinSetClip),
    ESP_ELFSYM_EXPORT(WinSetClipingBounds),
    ESP_ELFSYM_EXPORT(WinSetColors),
    ESP_ELFSYM_EXPORT(WinSetConstraintsSize),
    ESP_ELFSYM_EXPORT(WinSetCoordinateSystem),
    ESP_ELFSYM_EXPORT(WinSetDisplayExtent),
    ESP_ELFSYM_EXPORT(WinSetDrawMode),
    ESP_ELFSYM_EXPORT(WinSetDrawWindow),
    ESP_ELFSYM_EXPORT(WinSetForeColor),
    ESP_ELFSYM_EXPORT(WinSetForeColorRGB),
    ESP_ELFSYM_EXPORT(WinSetPattern),
    ESP_ELFSYM_EXPORT(WinSetPatternType),
    ESP_ELFSYM_EXPORT(WinSetScalingMode),
    ESP_ELFSYM_EXPORT(WinSetTextColor),
    ESP_ELFSYM_EXPORT(WinSetTextColorRGB),
    ESP_ELFSYM_EXPORT(WinSetUnderlineMode),
    ESP_ELFSYM_EXPORT(WinUnscaleAbsRect),
    ESP_ELFSYM_EXPORT(WinUnscaleCoord),
    ESP_ELFSYM_EXPORT(WinUnscalePoint),
    ESP_ELFSYM_EXPORT(WinUnscaleRectangle),
    ESP_ELFSYM_EXPORT(WinValidateHandle),
    ESP_ELFSYM_EXPORT(WinWindowToDisplayPt),
    ESP_ELFSYM_EXPORT(ChatClose),
    ESP_ELFSYM_EXPORT(ChatOpen),
    ESP_ELFSYM_EXPORT(ChatQuery),
    ESP_ELFSYM_EXPORT(heap_alloc),
    ESP_ELFSYM_EXPORT(heap_base),
    ESP_ELFSYM_EXPORT(heap_dump),
    ESP_ELFSYM_EXPORT(heap_finish),
    ESP_ELFSYM_EXPORT(heap_free),
    ESP_ELFSYM_EXPORT(heap_init),
    ESP_ELFSYM_EXPORT(heap_realloc),
    ESP_ELFSYM_EXPORT(heap_size),
    ESP_ELFSYM_EXPORT(grail_begin),
    ESP_ELFSYM_EXPORT(grail_end),
    ESP_ELFSYM_EXPORT(grail_reset),
    ESP_ELFSYM_EXPORT(grail_stroke),
    ESP_ELFSYM_EXPORT(WavBufferHeader),
    ESP_ELFSYM_EXPORT(WavFileHeader),
    ESP_ELFSYM_EXPORT(dia_clicked),
    ESP_ELFSYM_EXPORT(dia_color),
    ESP_ELFSYM_EXPORT(dia_draw_stroke),
    ESP_ELFSYM_EXPORT(dia_finish),
    ESP_ELFSYM_EXPORT(dia_get_graffiti_dimension),
    ESP_ELFSYM_EXPORT(dia_get_main_dimension),
    ESP_ELFSYM_EXPORT(dia_get_state),
    ESP_ELFSYM_EXPORT(dia_get_taskbar_dimension),
    ESP_ELFSYM_EXPORT(dia_get_trigger),
    ESP_ELFSYM_EXPORT(dia_init),
    ESP_ELFSYM_EXPORT(dia_refresh),
    ESP_ELFSYM_EXPORT(dia_set_graffiti_state),
    ESP_ELFSYM_EXPORT(dia_set_state),
    ESP_ELFSYM_EXPORT(dia_set_trigger),
    ESP_ELFSYM_EXPORT(dia_set_wh),
    ESP_ELFSYM_EXPORT(dia_stroke),
    ESP_ELFSYM_EXPORT(dia_update),
    ESP_ELFSYM_EXPORT(taskbar_add),
    ESP_ELFSYM_EXPORT(taskbar_add_widget),
    ESP_ELFSYM_EXPORT(taskbar_clicked),
    ESP_ELFSYM_EXPORT(taskbar_create),
    ESP_ELFSYM_EXPORT(taskbar_destroy),
    ESP_ELFSYM_EXPORT(taskbar_draw),
    ESP_ELFSYM_EXPORT(taskbar_remove),
    ESP_ELFSYM_EXPORT(taskbar_remove_widget),
    ESP_ELFSYM_EXPORT(taskbar_update),
    ESP_ELFSYM_EXPORT(taskbar_widget_clicked),
    ESP_ELFSYM_EXPORT(wman_add),
    ESP_ELFSYM_EXPORT(wman_choose_border),
    ESP_ELFSYM_EXPORT(wman_clear),
    ESP_ELFSYM_EXPORT(wman_clicked),
    ESP_ELFSYM_EXPORT(wman_draw_all),
    ESP_ELFSYM_EXPORT(wman_finish),
    ESP_ELFSYM_EXPORT(wman_init),
    ESP_ELFSYM_EXPORT(wman_move),
    ESP_ELFSYM_EXPORT(wman_raise),
    ESP_ELFSYM_EXPORT(wman_remove),
    ESP_ELFSYM_EXPORT(wman_set_background),
    ESP_ELFSYM_EXPORT(wman_set_border),
    ESP_ELFSYM_EXPORT(wman_set_image_background),
    ESP_ELFSYM_EXPORT(wman_texture),
    ESP_ELFSYM_EXPORT(wman_update),
    ESP_ELFSYM_EXPORT(wman_xy),
    ESP_ELFSYM_EXPORT(pumpkin_editor_init_io),
    ESP_ELFSYM_EXPORT(pumpkin_editor_init_term),
    ESP_ELFSYM_EXPORT(syntax_get_plugin),
    ESP_ELFSYM_EXPORT(editor_get_plugin),
    ESP_ELFSYM_EXPORT(RegDelete),
    ESP_ELFSYM_EXPORT(RegFinish),
    ESP_ELFSYM_EXPORT(RegGet),
    ESP_ELFSYM_EXPORT(RegGetById),
    ESP_ELFSYM_EXPORT(RegInit),
    ESP_ELFSYM_EXPORT(RegSet),
    ESP_ELFSYM_EXPORT(LanguageFinish),
    ESP_ELFSYM_EXPORT(LanguageInit),
    ESP_ELFSYM_EXPORT(calibrate),
    ESP_ELFSYM_EXPORT(pumpkin_unzip_file),
    ESP_ELFSYM_EXPORT(pumpkin_unzip_filename),
    ESP_ELFSYM_EXPORT(pumpkin_unzip_memory),
    ESP_ELFSYM_EXPORT(pumpkin_unzip_resource),
    ESP_ELFSYM_EXPORT(jzBuffer),
    ESP_ELFSYM_EXPORT(jzReadCentralDirectory),
    ESP_ELFSYM_EXPORT(jzReadData),
    ESP_ELFSYM_EXPORT(jzReadEndRecord),
    ESP_ELFSYM_EXPORT(jzReadLocalFileHeader),
    ESP_ELFSYM_EXPORT(jzReadLocalFileHeaderRaw),
    ESP_ELFSYM_EXPORT(puff),
    ESP_ELFSYM_EXPORT(plibc_chdir),
    ESP_ELFSYM_EXPORT(plibc_close),
    ESP_ELFSYM_EXPORT(plibc_dup),
    ESP_ELFSYM_EXPORT(plibc_dup2),
    ESP_ELFSYM_EXPORT(plibc_errno),
    ESP_ELFSYM_EXPORT(plibc_error),
    ESP_ELFSYM_EXPORT(plibc_fclose),
    ESP_ELFSYM_EXPORT(plibc_fdopen),
    ESP_ELFSYM_EXPORT(plibc_feof),
    ESP_ELFSYM_EXPORT(plibc_fflush),
    ESP_ELFSYM_EXPORT(plibc_fgetc),
    ESP_ELFSYM_EXPORT(plibc_fgets),
    ESP_ELFSYM_EXPORT(plibc_fileno),
    ESP_ELFSYM_EXPORT(plibc_finish),
    ESP_ELFSYM_EXPORT(plibc_fopen),
    ESP_ELFSYM_EXPORT(plibc_fprintf),
    ESP_ELFSYM_EXPORT(plibc_fputc),
    ESP_ELFSYM_EXPORT(plibc_fputs),
    ESP_ELFSYM_EXPORT(plibc_fread),
    ESP_ELFSYM_EXPORT(plibc_fseek),
    ESP_ELFSYM_EXPORT(plibc_ftell),
    ESP_ELFSYM_EXPORT(plibc_ftruncate),
    ESP_ELFSYM_EXPORT(plibc_fwrite),
    ESP_ELFSYM_EXPORT(plibc_getdir),
    ESP_ELFSYM_EXPORT(plibc_haschar),
    ESP_ELFSYM_EXPORT(plibc_init),
    ESP_ELFSYM_EXPORT(plibc_isatty),
    ESP_ELFSYM_EXPORT(plibc_lseek),
    ESP_ELFSYM_EXPORT(plibc_mkdir),
    ESP_ELFSYM_EXPORT(plibc_open),
    ESP_ELFSYM_EXPORT(plibc_printf),
    ESP_ELFSYM_EXPORT(plibc_read),
    ESP_ELFSYM_EXPORT(plibc_remove),
    ESP_ELFSYM_EXPORT(plibc_rename),
    ESP_ELFSYM_EXPORT(plibc_setfd),
    ESP_ELFSYM_EXPORT(plibc_stderr),
    ESP_ELFSYM_EXPORT(plibc_stdin),
    ESP_ELFSYM_EXPORT(plibc_stdout),
    ESP_ELFSYM_EXPORT(plibc_strerror),
    ESP_ELFSYM_EXPORT(plibc_tmpnam),
    ESP_ELFSYM_EXPORT(plibc_ungetc),
    ESP_ELFSYM_EXPORT(plibc_vfprintf),
    ESP_ELFSYM_EXPORT(plibc_vprintf),
    ESP_ELFSYM_EXPORT(plibc_write),
    ESP_ELFSYM_EXPORT(DOSBoxMain),
    ESP_ELFSYM_EXPORT(_ssfn_zlib_decode),
    ESP_ELFSYM_EXPORT(pumpkin_create_ssfn),
    ESP_ELFSYM_EXPORT(pumpkin_destroy_ssfn),
    ESP_ELFSYM_EXPORT(ssfn_bbox),
    ESP_ELFSYM_EXPORT(ssfn_errstr),
    ESP_ELFSYM_EXPORT(ssfn_free),
    ESP_ELFSYM_EXPORT(ssfn_load),
    ESP_ELFSYM_EXPORT(ssfn_mem),
    ESP_ELFSYM_EXPORT(ssfn_render),
    ESP_ELFSYM_EXPORT(ssfn_select),
    ESP_ELFSYM_EXPORT(ssfn_text),
    ESP_ELFSYM_EXPORT(ssfn_utf8),
    ESP_ELFSYM_EXPORT(logtrap_disasm),
    ESP_ELFSYM_EXPORT(logtrap_finish),
    ESP_ELFSYM_EXPORT(logtrap_global_finish),
    ESP_ELFSYM_EXPORT(logtrap_global_init),
    ESP_ELFSYM_EXPORT(logtrap_init),
    ESP_ELFSYM_EXPORT(logtrap_start),
    ESP_ELFSYM_EXPORT(logtrap_started),
    ESP_ELFSYM_EXPORT(logtrap_trapname),
    ESP_ELFSYM_EXPORT(BmpGlueGetBitDepth),
    ESP_ELFSYM_EXPORT(BmpGlueGetBits),
    ESP_ELFSYM_EXPORT(BmpGlueGetCompressionType),
    ESP_ELFSYM_EXPORT(BmpGlueGetDimensions),
    ESP_ELFSYM_EXPORT(BmpGlueGetNextBitmap),
    ESP_ELFSYM_EXPORT(BmpGlueGetTransparentValue),
    ESP_ELFSYM_EXPORT(BmpGlueSetTransparentValue),
    ESP_ELFSYM_EXPORT(CtlGlueGetControlStyle),
    ESP_ELFSYM_EXPORT(CtlGlueGetFont),
    ESP_ELFSYM_EXPORT(CtlGlueGetGraphics),
    ESP_ELFSYM_EXPORT(CtlGlueIsGraphical),
    ESP_ELFSYM_EXPORT(CtlGlueNewSliderControl),
    ESP_ELFSYM_EXPORT(CtlGlueSetFont),
    ESP_ELFSYM_EXPORT(CtlGlueSetFrameStyle),
    ESP_ELFSYM_EXPORT(CtlGlueSetLeftAnchor),
    ESP_ELFSYM_EXPORT(DateGlueTemplateToAscii),
    ESP_ELFSYM_EXPORT(DateGlueToDOWDMFormat),
    ESP_ELFSYM_EXPORT(FldGlueGetLineInfo),
    ESP_ELFSYM_EXPORT(FntGlueGetDefaultFontID),
    ESP_ELFSYM_EXPORT(FntGlueTruncateString),
    ESP_ELFSYM_EXPORT(FntGlueWCharWidth),
    ESP_ELFSYM_EXPORT(FntGlueWidthToOffset),
    ESP_ELFSYM_EXPORT(FrmGlueGetDefaultButtonID),
    ESP_ELFSYM_EXPORT(FrmGlueGetEventHandler),
    ESP_ELFSYM_EXPORT(FrmGlueGetHelpID),
    ESP_ELFSYM_EXPORT(FrmGlueGetLabelFont),
    ESP_ELFSYM_EXPORT(FrmGlueGetMenuBarID),
    ESP_ELFSYM_EXPORT(FrmGlueGetObjIDFromObjPtr),
    ESP_ELFSYM_EXPORT(FrmGlueGetObjectUsable),
    ESP_ELFSYM_EXPORT(FrmGlueSetDefaultButtonID),
    ESP_ELFSYM_EXPORT(FrmGlueSetHelpID),
    ESP_ELFSYM_EXPORT(FrmGlueSetLabelFont),
    ESP_ELFSYM_EXPORT(LstGlueGetDrawFunction),
    ESP_ELFSYM_EXPORT(LstGlueGetFont),
    ESP_ELFSYM_EXPORT(LstGlueGetItemsText),
    ESP_ELFSYM_EXPORT(LstGlueGetTopItem),
    ESP_ELFSYM_EXPORT(LstGlueSetFont),
    ESP_ELFSYM_EXPORT(LstGlueSetIncrementalSearch),
    ESP_ELFSYM_EXPORT(MemGluePtrNew),
    ESP_ELFSYM_EXPORT(TblGlueGetColumnMasked),
    ESP_ELFSYM_EXPORT(TblGlueGetItemPtr),
    ESP_ELFSYM_EXPORT(TblGlueGetNumberOfColumns),
    ESP_ELFSYM_EXPORT(TblGlueGetTopRow),
    ESP_ELFSYM_EXPORT(TblGlueSetSelection),
    ESP_ELFSYM_EXPORT(TxtGlueCharAttr),
    ESP_ELFSYM_EXPORT(TxtGlueCharIsValid),
    ESP_ELFSYM_EXPORT(TxtGlueCharSize),
    ESP_ELFSYM_EXPORT(TxtGlueCharWidth),
    ESP_ELFSYM_EXPORT(TxtGlueFindString),
    ESP_ELFSYM_EXPORT(TxtGlueGetNextChar),
    ESP_ELFSYM_EXPORT(TxtGlueGetPreviousChar),
    ESP_ELFSYM_EXPORT(TxtGlueLowerChar),
    ESP_ELFSYM_EXPORT(TxtGlueSetNextChar),
    ESP_ELFSYM_EXPORT(TxtGlueUpperChar),
    ESP_ELFSYM_EXPORT(WinGlueDrawTruncChars),
    ESP_ELFSYM_EXPORT(WinGlueGetFrameType),
    ESP_ELFSYM_EXPORT(WinGlueSetFrameType),
    ESP_ELFSYM_EXPORT(GPSClose),
    ESP_ELFSYM_EXPORT(GPSFinishModule),
    ESP_ELFSYM_EXPORT(GPSGetLibAPIVersion),
    ESP_ELFSYM_EXPORT(GPSGetMaxSatellites),
    ESP_ELFSYM_EXPORT(GPSGetPVT),
    ESP_ELFSYM_EXPORT(GPSGetPosition),
    ESP_ELFSYM_EXPORT(GPSGetSatellites),
    ESP_ELFSYM_EXPORT(GPSGetStatus),
    ESP_ELFSYM_EXPORT(GPSGetTime),
    ESP_ELFSYM_EXPORT(GPSGetVelocity),
    ESP_ELFSYM_EXPORT(GPSInitModule),
    ESP_ELFSYM_EXPORT(GPSOpen),
    ESP_ELFSYM_EXPORT(GPDClose),
    ESP_ELFSYM_EXPORT(GPDGetVersion),
    ESP_ELFSYM_EXPORT(GPDOpen),
    ESP_ELFSYM_EXPORT(GPDReadInstant),
    ESP_ELFSYM_EXPORT(GPDReadInstantKCSformat),
    ESP_ELFSYM_EXPORT(CallCompareFunction),
    ESP_ELFSYM_EXPORT(CallDmCompare),
    ESP_ELFSYM_EXPORT(CallFormHandler),
    ESP_ELFSYM_EXPORT(CallGadgetHandler),
    ESP_ELFSYM_EXPORT(CallListDrawItem),
    ESP_ELFSYM_EXPORT(CallNotifyProc),
    ESP_ELFSYM_EXPORT(CallPrgCallback),
    ESP_ELFSYM_EXPORT(CallSndFunc),
    ESP_ELFSYM_EXPORT(CallSndFuncArm),
    ESP_ELFSYM_EXPORT(CallSndVFunc),
    ESP_ELFSYM_EXPORT(CallSndVFuncArm),
    ESP_ELFSYM_EXPORT(CallTableDrawItem),
    ESP_ELFSYM_EXPORT(CallTableLoadData),
    ESP_ELFSYM_EXPORT(CallTableSaveData),
    ESP_ELFSYM_EXPORT(ExgDBReadARM),
    ESP_ELFSYM_EXPORT(arm_native_call_pce),
    ESP_ELFSYM_EXPORT(arm_native_call_sub),
    ESP_ELFSYM_EXPORT(cpu_pulse_reset),
    ESP_ELFSYM_EXPORT(cpu_read_byte),
    ESP_ELFSYM_EXPORT(cpu_read_long),
    ESP_ELFSYM_EXPORT(cpu_read_word),
    ESP_ELFSYM_EXPORT(cpu_write_byte),
    ESP_ELFSYM_EXPORT(cpu_write_long),
    ESP_ELFSYM_EXPORT(cpu_write_word),
    ESP_ELFSYM_EXPORT(decode_FileInfoType),
    ESP_ELFSYM_EXPORT(decode_NetConfigNameType),
    ESP_ELFSYM_EXPORT(decode_NetSocketAddrType),
    ESP_ELFSYM_EXPORT(decode_appinfo),
    ESP_ELFSYM_EXPORT(decode_datetime),
    ESP_ELFSYM_EXPORT(decode_event),
    ESP_ELFSYM_EXPORT(decode_locale),
    ESP_ELFSYM_EXPORT(decode_notify),
    ESP_ELFSYM_EXPORT(decode_point),
    ESP_ELFSYM_EXPORT(decode_rectangle),
    ESP_ELFSYM_EXPORT(decode_rgb),
    ESP_ELFSYM_EXPORT(decode_smfoptions),
    ESP_ELFSYM_EXPORT(emupalmos_check_address),
    ESP_ELFSYM_EXPORT(emupalmos_deinstall),
    ESP_ELFSYM_EXPORT(emupalmos_disasm),
    ESP_ELFSYM_EXPORT(emupalmos_finish),
    ESP_ELFSYM_EXPORT(emupalmos_finished),
    ESP_ELFSYM_EXPORT(emupalmos_init),
    ESP_ELFSYM_EXPORT(emupalmos_install),
    ESP_ELFSYM_EXPORT(emupalmos_main),
    ESP_ELFSYM_EXPORT(emupalmos_memory_hooks),
    ESP_ELFSYM_EXPORT(emupalmos_panic),
    ESP_ELFSYM_EXPORT(emupalmos_ram),
    ESP_ELFSYM_EXPORT(emupalmos_trap_in),
    ESP_ELFSYM_EXPORT(emupalmos_trap_out),
    ESP_ELFSYM_EXPORT(emupalmos_trap_sel_in),
    ESP_ELFSYM_EXPORT(encode_FileInfoType),
    ESP_ELFSYM_EXPORT(encode_NetConfigNameType),
    ESP_ELFSYM_EXPORT(encode_NetHostInfoBufType),
    ESP_ELFSYM_EXPORT(encode_NetSocketAddrType),
    ESP_ELFSYM_EXPORT(encode_VolumeInfoType),
    ESP_ELFSYM_EXPORT(encode_appinfo),
    ESP_ELFSYM_EXPORT(encode_datetime),
    ESP_ELFSYM_EXPORT(encode_deviceinfo),
    ESP_ELFSYM_EXPORT(encode_event),
    ESP_ELFSYM_EXPORT(encode_gadget),
    ESP_ELFSYM_EXPORT(encode_locale),
    ESP_ELFSYM_EXPORT(encode_notify),
    ESP_ELFSYM_EXPORT(encode_point),
    ESP_ELFSYM_EXPORT(encode_rectangle),
    ESP_ELFSYM_EXPORT(encode_rgb),
    ESP_ELFSYM_EXPORT(encode_string),
    ESP_ELFSYM_EXPORT(m68k_get_emu_state),
    ESP_ELFSYM_EXPORT(m68k_get_state),
    ESP_ELFSYM_EXPORT(m68k_trapname),
    ESP_ELFSYM_EXPORT(emupalmos_arm_syscall),
    ESP_ELFSYM_EXPORT(palmos_omtrap),
    ESP_ELFSYM_EXPORT(palmos_pinstrap),
    ESP_ELFSYM_EXPORT(palmos_highdensitytrap),
    ESP_ELFSYM_EXPORT(palmos_serialtrap),
    ESP_ELFSYM_EXPORT(palmos_filesystemtrap),
    ESP_ELFSYM_EXPORT(palmos_intltrap),
    ESP_ELFSYM_EXPORT(palmos_flpemtrap),
    ESP_ELFSYM_EXPORT(palmos_flptrap),
    ESP_ELFSYM_EXPORT(palmos_accessortrap),
    ESP_ELFSYM_EXPORT(palmos_expansiontrap),
    ESP_ELFSYM_EXPORT(palmos_tsmtrap),
    ESP_ELFSYM_EXPORT(palmos_lmtrap),
    ESP_ELFSYM_EXPORT(palmos_navtrap),
    ESP_ELFSYM_EXPORT(palmos_netlibtrap),
    ESP_ELFSYM_EXPORT(palmos_gpdlibtrap),
    ESP_ELFSYM_EXPORT(palmos_systrap),
    ESP_ELFSYM_EXPORT(default_instr_hook_callback),
    ESP_ELFSYM_EXPORT(m68040_fpu_op0),
    ESP_ELFSYM_EXPORT(m68040_fpu_op1),
    ESP_ELFSYM_EXPORT(m68881_mmu_ops),
    ESP_ELFSYM_EXPORT(m68k_context_size),
    ESP_ELFSYM_EXPORT(m68k_cycles_remaining),
    ESP_ELFSYM_EXPORT(m68k_cycles_run),
    ESP_ELFSYM_EXPORT(m68k_end_timeslice),
    ESP_ELFSYM_EXPORT(m68k_execute),
    ESP_ELFSYM_EXPORT(m68k_get_context),
    ESP_ELFSYM_EXPORT(m68k_get_reg),
    ESP_ELFSYM_EXPORT(m68k_get_virq),
    ESP_ELFSYM_EXPORT(m68k_init),
    ESP_ELFSYM_EXPORT(m68k_init_once),
    ESP_ELFSYM_EXPORT(m68k_make_hex),
    ESP_ELFSYM_EXPORT(m68k_modify_timeslice),
    ESP_ELFSYM_EXPORT(m68k_pulse_halt),
    ESP_ELFSYM_EXPORT(m68k_pulse_reset),
    ESP_ELFSYM_EXPORT(m68k_set_bkpt_ack_callback),
    ESP_ELFSYM_EXPORT(m68k_set_cmpild_instr_callback),
    ESP_ELFSYM_EXPORT(m68k_set_context),
    ESP_ELFSYM_EXPORT(m68k_set_cpu_type),
    ESP_ELFSYM_EXPORT(m68k_set_fc_callback),
    ESP_ELFSYM_EXPORT(m68k_set_illg_instr_callback),
    ESP_ELFSYM_EXPORT(m68k_set_instr_hook2_callback),
    ESP_ELFSYM_EXPORT(m68k_set_instr_hook_callback),
    ESP_ELFSYM_EXPORT(m68k_set_int_ack_callback),
    ESP_ELFSYM_EXPORT(m68k_set_irq),
    ESP_ELFSYM_EXPORT(m68k_set_pc_changed_callback),
    ESP_ELFSYM_EXPORT(m68k_set_reg),
    ESP_ELFSYM_EXPORT(m68k_set_reset_instr_callback),
    ESP_ELFSYM_EXPORT(m68k_set_rte_instr_callback),
    ESP_ELFSYM_EXPORT(m68k_set_tas_instr_callback),
    ESP_ELFSYM_EXPORT(m68k_set_virq),
    ESP_ELFSYM_EXPORT(pmmu_translate_addr),
    ESP_ELFSYM_EXPORT(float128_add),
    ESP_ELFSYM_EXPORT(float128_div),
    ESP_ELFSYM_EXPORT(float128_eq),
    ESP_ELFSYM_EXPORT(float128_eq_signaling),
    ESP_ELFSYM_EXPORT(float128_is_nan),
    ESP_ELFSYM_EXPORT(float128_is_signaling_nan),
    ESP_ELFSYM_EXPORT(float128_le),
    ESP_ELFSYM_EXPORT(float128_le_quiet),
    ESP_ELFSYM_EXPORT(float128_lt),
    ESP_ELFSYM_EXPORT(float128_lt_quiet),
    ESP_ELFSYM_EXPORT(float128_mul),
    ESP_ELFSYM_EXPORT(float128_rem),
    ESP_ELFSYM_EXPORT(float128_round_to_int),
    ESP_ELFSYM_EXPORT(float128_sqrt),
    ESP_ELFSYM_EXPORT(float128_sub),
    ESP_ELFSYM_EXPORT(float128_to_float32),
    ESP_ELFSYM_EXPORT(float128_to_float64),
    ESP_ELFSYM_EXPORT(float128_to_floatx80),
    ESP_ELFSYM_EXPORT(float128_to_int32),
    ESP_ELFSYM_EXPORT(float128_to_int32_round_to_zero),
    ESP_ELFSYM_EXPORT(float128_to_int64),
    ESP_ELFSYM_EXPORT(float128_to_int64_round_to_zero),
    ESP_ELFSYM_EXPORT(float32_add),
    ESP_ELFSYM_EXPORT(float32_div),
    ESP_ELFSYM_EXPORT(float32_eq),
    ESP_ELFSYM_EXPORT(float32_eq_signaling),
    ESP_ELFSYM_EXPORT(float32_is_nan),
    ESP_ELFSYM_EXPORT(float32_is_signaling_nan),
    ESP_ELFSYM_EXPORT(float32_le),
    ESP_ELFSYM_EXPORT(float32_le_quiet),
    ESP_ELFSYM_EXPORT(float32_lt),
    ESP_ELFSYM_EXPORT(float32_lt_quiet),
    ESP_ELFSYM_EXPORT(float32_mul),
    ESP_ELFSYM_EXPORT(float32_rem),
    ESP_ELFSYM_EXPORT(float32_round_to_int),
    ESP_ELFSYM_EXPORT(float32_sqrt),
    ESP_ELFSYM_EXPORT(float32_sub),
    ESP_ELFSYM_EXPORT(float32_to_float128),
    ESP_ELFSYM_EXPORT(float32_to_float64),
    ESP_ELFSYM_EXPORT(float32_to_floatx80),
    ESP_ELFSYM_EXPORT(float32_to_int32),
    ESP_ELFSYM_EXPORT(float32_to_int32_round_to_zero),
    ESP_ELFSYM_EXPORT(float32_to_int64),
    ESP_ELFSYM_EXPORT(float32_to_int64_round_to_zero),
    ESP_ELFSYM_EXPORT(float64_add),
    ESP_ELFSYM_EXPORT(float64_div),
    ESP_ELFSYM_EXPORT(float64_eq),
    ESP_ELFSYM_EXPORT(float64_eq_signaling),
    ESP_ELFSYM_EXPORT(float64_is_nan),
    ESP_ELFSYM_EXPORT(float64_is_signaling_nan),
    ESP_ELFSYM_EXPORT(float64_le),
    ESP_ELFSYM_EXPORT(float64_le_quiet),
    ESP_ELFSYM_EXPORT(float64_lt),
    ESP_ELFSYM_EXPORT(float64_lt_quiet),
    ESP_ELFSYM_EXPORT(float64_mul),
    ESP_ELFSYM_EXPORT(float64_rem),
    ESP_ELFSYM_EXPORT(float64_round_to_int),
    ESP_ELFSYM_EXPORT(float64_sqrt),
    ESP_ELFSYM_EXPORT(float64_sub),
    ESP_ELFSYM_EXPORT(float64_to_float128),
    ESP_ELFSYM_EXPORT(float64_to_float32),
    ESP_ELFSYM_EXPORT(float64_to_floatx80),
    ESP_ELFSYM_EXPORT(float64_to_int32),
    ESP_ELFSYM_EXPORT(float64_to_int32_round_to_zero),
    ESP_ELFSYM_EXPORT(float64_to_int64),
    ESP_ELFSYM_EXPORT(float64_to_int64_round_to_zero),
    ESP_ELFSYM_EXPORT(float_detect_tininess),
    ESP_ELFSYM_EXPORT(float_exception_flags),
    ESP_ELFSYM_EXPORT(float_raise),
    ESP_ELFSYM_EXPORT(float_rounding_mode),
    ESP_ELFSYM_EXPORT(floatx80_add),
    ESP_ELFSYM_EXPORT(floatx80_div),
    ESP_ELFSYM_EXPORT(floatx80_eq),
    ESP_ELFSYM_EXPORT(floatx80_eq_signaling),
    ESP_ELFSYM_EXPORT(floatx80_is_nan),
    ESP_ELFSYM_EXPORT(floatx80_is_signaling_nan),
    ESP_ELFSYM_EXPORT(floatx80_le),
    ESP_ELFSYM_EXPORT(floatx80_le_quiet),
    ESP_ELFSYM_EXPORT(floatx80_lt),
    ESP_ELFSYM_EXPORT(floatx80_lt_quiet),
    ESP_ELFSYM_EXPORT(floatx80_mul),
    ESP_ELFSYM_EXPORT(floatx80_rem),
    ESP_ELFSYM_EXPORT(floatx80_round_to_int),
    ESP_ELFSYM_EXPORT(floatx80_rounding_precision),
    ESP_ELFSYM_EXPORT(floatx80_sqrt),
    ESP_ELFSYM_EXPORT(floatx80_sub),
    ESP_ELFSYM_EXPORT(floatx80_to_float128),
    ESP_ELFSYM_EXPORT(floatx80_to_float32),
    ESP_ELFSYM_EXPORT(floatx80_to_float64),
    ESP_ELFSYM_EXPORT(floatx80_to_int32),
    ESP_ELFSYM_EXPORT(floatx80_to_int32_round_to_zero),
    ESP_ELFSYM_EXPORT(floatx80_to_int64),
    ESP_ELFSYM_EXPORT(floatx80_to_int64_round_to_zero),
    ESP_ELFSYM_EXPORT(int32_to_float128),
    ESP_ELFSYM_EXPORT(int32_to_float32),
    ESP_ELFSYM_EXPORT(int32_to_float64),
    ESP_ELFSYM_EXPORT(int32_to_floatx80),
    ESP_ELFSYM_EXPORT(int64_to_float128),
    ESP_ELFSYM_EXPORT(int64_to_float32),
    ESP_ELFSYM_EXPORT(int64_to_float64),
    ESP_ELFSYM_EXPORT(int64_to_floatx80),
    ESP_ELFSYM_EXPORT(propagateFloatx80NaN),
    ESP_ELFSYM_EXPORT(roundAndPackFloatx80),
    ESP_ELFSYM_EXPORT(m68ki_build_opcode_table),
    ESP_ELFSYM_EXPORT(m68ki_cycles),
    ESP_ELFSYM_EXPORT(m68ki_instruction_jump_table),
    ESP_ELFSYM_EXPORT(disasm),
    ESP_ELFSYM_EXPORT(decode_notif),
    ESP_ELFSYM_EXPORT(decode_sysNotifyAppCrashedEvent),
    ESP_ELFSYM_EXPORT(decode_sysNotifyAppLaunchingEvent),
    ESP_ELFSYM_EXPORT(decode_sysNotifyAppQuittingEvent),
    ESP_ELFSYM_EXPORT(decode_sysNotifyCardInsertedEvent),
    ESP_ELFSYM_EXPORT(decode_sysNotifyCardRemovedEvent),
    ESP_ELFSYM_EXPORT(decode_sysNotifyDBCreatedEvent),
    ESP_ELFSYM_EXPORT(decode_sysNotifyDBDeletedEvent),
    ESP_ELFSYM_EXPORT(decode_sysNotifyDisplayChangeEvent),
    ESP_ELFSYM_EXPORT(decode_sysNotifyGPSDataEvent),
    ESP_ELFSYM_EXPORT(decode_sysNotifyLocaleChangedEvent),
    ESP_ELFSYM_EXPORT(decode_sysNotifySelectDay),
    ESP_ELFSYM_EXPORT(decode_sysNotifySyncFinishEvent),
    ESP_ELFSYM_EXPORT(decode_sysNotifyTimeChangeEvent),
    ESP_ELFSYM_EXPORT(decode_sysNotifyVolumeMountedEvent),
    ESP_ELFSYM_EXPORT(decode_sysNotifyVolumeUnmountedEvent),
    ESP_ELFSYM_EXPORT(encode_notif),
    ESP_ELFSYM_EXPORT(encode_sysNotifyAppCrashedEvent),
    ESP_ELFSYM_EXPORT(encode_sysNotifyAppLaunchingEvent),
    ESP_ELFSYM_EXPORT(encode_sysNotifyAppQuittingEvent),
    ESP_ELFSYM_EXPORT(encode_sysNotifyCardInsertedEvent),
    ESP_ELFSYM_EXPORT(encode_sysNotifyCardRemovedEvent),
    ESP_ELFSYM_EXPORT(encode_sysNotifyDBCreatedEvent),
    ESP_ELFSYM_EXPORT(encode_sysNotifyDBDeletedEvent),
    ESP_ELFSYM_EXPORT(encode_sysNotifyDisplayChangeEvent),
    ESP_ELFSYM_EXPORT(encode_sysNotifyGPSDataEvent),
    ESP_ELFSYM_EXPORT(encode_sysNotifyLocaleChangedEvent),
    ESP_ELFSYM_EXPORT(encode_sysNotifySelectDay),
    ESP_ELFSYM_EXPORT(encode_sysNotifySyncFinishEvent),
    ESP_ELFSYM_EXPORT(encode_sysNotifyTimeChangeEvent),
    ESP_ELFSYM_EXPORT(encode_sysNotifyVolumeMountedEvent),
    ESP_ELFSYM_EXPORT(encode_sysNotifyVolumeUnmountedEvent),
    ESP_ELFSYM_EXPORT(decode_launch),
    ESP_ELFSYM_EXPORT(decode_sysAppLaunchCmdAlarmTriggered),
    ESP_ELFSYM_EXPORT(decode_sysAppLaunchCmdDisplayAlarm),
    ESP_ELFSYM_EXPORT(decode_sysAppLaunchCmdGoTo),
    ESP_ELFSYM_EXPORT(decode_sysAppLaunchCmdPanelCalledFromApp),
    ESP_ELFSYM_EXPORT(decode_sysAppLaunchCmdSystemReset),
    ESP_ELFSYM_EXPORT(encode_launch),
    ESP_ELFSYM_EXPORT(encode_sysAppLaunchCmdAlarmTriggered),
    ESP_ELFSYM_EXPORT(encode_sysAppLaunchCmdDisplayAlarm),
    ESP_ELFSYM_EXPORT(encode_sysAppLaunchCmdGoTo),
    ESP_ELFSYM_EXPORT(encode_sysAppLaunchCmdPanelCalledFromApp),
    ESP_ELFSYM_EXPORT(encode_sysAppLaunchCmdSystemReset),
    ESP_ELFSYM_EXPORT(memAccess),
    ESP_ELFSYM_EXPORT(memDeinit),
    ESP_ELFSYM_EXPORT(memInit),
    ESP_ELFSYM_EXPORT(memRegionAdd),
    ESP_ELFSYM_EXPORT(uarmDisasm),
    ESP_ELFSYM_EXPORT(uarmFinish),
    ESP_ELFSYM_EXPORT(uarmGetReg),
    ESP_ELFSYM_EXPORT(uarmInit),
    ESP_ELFSYM_EXPORT(uarmRun),
    ESP_ELFSYM_EXPORT(uarmSetReg),
    ESP_ELFSYM_EXPORT(cpuCoprocessorRegister),
    ESP_ELFSYM_EXPORT(cpuCycle),
    ESP_ELFSYM_EXPORT(cpuDeinit),
    ESP_ELFSYM_EXPORT(cpuDisasm),
    ESP_ELFSYM_EXPORT(cpuGetCPAR),
    ESP_ELFSYM_EXPORT(cpuGetPid),
    ESP_ELFSYM_EXPORT(cpuGetRegExternal),
    ESP_ELFSYM_EXPORT(cpuInit),
    ESP_ELFSYM_EXPORT(cpuIrq),
    ESP_ELFSYM_EXPORT(cpuMemOpExternal),
    ESP_ELFSYM_EXPORT(cpuReg),
    ESP_ELFSYM_EXPORT(cpuSetCPAR),
    ESP_ELFSYM_EXPORT(cpuSetPid),
    ESP_ELFSYM_EXPORT(cpuSetReg),
    ESP_ELFSYM_EXPORT(cpuSetVectorAddr),
    ESP_ELFSYM_EXPORT(icacheDeinit),
    ESP_ELFSYM_EXPORT(icacheFetch),
    ESP_ELFSYM_EXPORT(icacheInit),
    ESP_ELFSYM_EXPORT(icacheInval),
    ESP_ELFSYM_EXPORT(icacheInvalAddr),
    ESP_ELFSYM_EXPORT(mmuDeinit),
    ESP_ELFSYM_EXPORT(mmuDump),
    ESP_ELFSYM_EXPORT(mmuGetDomainCfg),
    ESP_ELFSYM_EXPORT(mmuGetR),
    ESP_ELFSYM_EXPORT(mmuGetS),
    ESP_ELFSYM_EXPORT(mmuGetTTP),
    ESP_ELFSYM_EXPORT(mmuInit),
    ESP_ELFSYM_EXPORT(mmuIsOn),
    ESP_ELFSYM_EXPORT(mmuSetDomainCfg),
    ESP_ELFSYM_EXPORT(mmuSetR),
    ESP_ELFSYM_EXPORT(mmuSetS),
    ESP_ELFSYM_EXPORT(mmuSetTTP),
    ESP_ELFSYM_EXPORT(mmuTlbFlush),
    ESP_ELFSYM_EXPORT(mmuTranslate),
    ESP_ELFSYM_EXPORT(ramDeinit),
    ESP_ELFSYM_EXPORT(ramInit),
    ESP_ELFSYM_EXPORT(pxa270icPrvCoprocAccess),
    ESP_ELFSYM_EXPORT(socIcDeinit),
    ESP_ELFSYM_EXPORT(socIcInit),
    ESP_ELFSYM_EXPORT(socIcInt),
    ESP_ELFSYM_EXPORT(cp15Cycle),
    ESP_ELFSYM_EXPORT(cp15Deinit),
    ESP_ELFSYM_EXPORT(cp15Init),
    ESP_ELFSYM_EXPORT(cp15SetFaultStatus),
    ESP_ELFSYM_EXPORT(darm_armv7_disasm),
    ESP_ELFSYM_EXPORT(darm_condition_index),
    ESP_ELFSYM_EXPORT(darm_condition_meaning_fp),
    ESP_ELFSYM_EXPORT(darm_condition_meaning_int),
    ESP_ELFSYM_EXPORT(darm_condition_name),
    ESP_ELFSYM_EXPORT(darm_enctype_name),
    ESP_ELFSYM_EXPORT(darm_immshift_decode),
    ESP_ELFSYM_EXPORT(darm_mnemonic_name),
    ESP_ELFSYM_EXPORT(darm_register_name),
    ESP_ELFSYM_EXPORT(darm_shift_type_name),
    ESP_ELFSYM_EXPORT(armv7_format_strings),
    ESP_ELFSYM_EXPORT(armv7_instr_labels),
    ESP_ELFSYM_EXPORT(armv7_instr_types),
    ESP_ELFSYM_EXPORT(type_bits_instr_lookup),
    ESP_ELFSYM_EXPORT(type_brnchmisc_instr_lookup),
    ESP_ELFSYM_EXPORT(type_mul_instr_lookup),
    ESP_ELFSYM_EXPORT(type_opless_instr_lookup),
    ESP_ELFSYM_EXPORT(type_pas_instr_lookup),
    ESP_ELFSYM_EXPORT(type_pusr_instr_lookup),
    ESP_ELFSYM_EXPORT(type_sat_instr_lookup),
    ESP_ELFSYM_EXPORT(type_shift_instr_lookup),
    ESP_ELFSYM_EXPORT(type_stack0_instr_lookup),
    ESP_ELFSYM_EXPORT(type_stack1_instr_lookup),
    ESP_ELFSYM_EXPORT(type_stack2_instr_lookup),
    ESP_ELFSYM_EXPORT(type_sync_instr_lookup),
    ESP_ELFSYM_EXPORT(type_uncond2_instr_lookup),
    ESP_ELFSYM_EXPORT(darm_disasm),
    ESP_ELFSYM_EXPORT(darm_init),
    ESP_ELFSYM_EXPORT(darm_reglist),
    ESP_ELFSYM_EXPORT(darm_str),
    ESP_ELFSYM_EXPORT(darm_str2),
    ESP_ELFSYM_EXPORT(darm_enctypes),
    ESP_ELFSYM_EXPORT(darm_mnemonics),
    ESP_ELFSYM_EXPORT(darm_registers),
    ESP_ELFSYM_EXPORT(darm_thumb_disasm),
    ESP_ELFSYM_EXPORT(darm_thumb2_disasm),
    ESP_ELFSYM_EXPORT(darm_thumb2_str),
    ESP_ELFSYM_EXPORT(thumb2_decode_immshift),
    ESP_ELFSYM_EXPORT(thumb2_parse_flag),
    ESP_ELFSYM_EXPORT(thumb2_parse_imm),
    ESP_ELFSYM_EXPORT(thumb2_parse_misc),
    ESP_ELFSYM_EXPORT(thumb2_parse_reg),
    ESP_ELFSYM_EXPORT(thumb_expand_imm),
    ESP_ELFSYM_EXPORT(thumb2_branch_misc_ctrl),
    ESP_ELFSYM_EXPORT(thumb2_coproc_simd),
    ESP_ELFSYM_EXPORT(thumb2_data_reg),
    ESP_ELFSYM_EXPORT(thumb2_data_shifted_reg),
    ESP_ELFSYM_EXPORT(thumb2_decode_instruction),
    ESP_ELFSYM_EXPORT(thumb2_load_byte_hints),
    ESP_ELFSYM_EXPORT(thumb2_load_halfword_hints),
    ESP_ELFSYM_EXPORT(thumb2_load_store_dual),
    ESP_ELFSYM_EXPORT(thumb2_load_store_multiple),
    ESP_ELFSYM_EXPORT(thumb2_load_word),
    ESP_ELFSYM_EXPORT(thumb2_long_mult_acc),
    ESP_ELFSYM_EXPORT(thumb2_misc_ctrl),
    ESP_ELFSYM_EXPORT(thumb2_misc_op),
    ESP_ELFSYM_EXPORT(thumb2_modified_immediate),
    ESP_ELFSYM_EXPORT(thumb2_move_shift),
    ESP_ELFSYM_EXPORT(thumb2_mult_acc_diff),
    ESP_ELFSYM_EXPORT(thumb2_nm_decoder),
    ESP_ELFSYM_EXPORT(thumb2_parallel_signed),
    ESP_ELFSYM_EXPORT(thumb2_parallel_unsigned),
    ESP_ELFSYM_EXPORT(thumb2_plain_immediate),
    ESP_ELFSYM_EXPORT(thumb2_proc_state),
    ESP_ELFSYM_EXPORT(thumb2_store_single_item),
    ESP_ELFSYM_EXPORT(thumb2_instr_labels),
    ESP_ELFSYM_EXPORT(thumb2_instruction_strings),
    ESP_ELFSYM_EXPORT(thumb_instr_labels),
    ESP_ELFSYM_EXPORT(thumb_instr_types),
    ESP_ELFSYM_EXPORT(type_extend_instr_lookup),
    ESP_ELFSYM_EXPORT(type_gpi_instr_lookup),
    ESP_ELFSYM_EXPORT(type_hints_instr_lookup),
    ESP_ELFSYM_EXPORT(type_rev_instr_lookup),
    ESP_ELFSYM_EXPORT(EmulatorMain),
    ESP_ELFSYM_EXPORT(TOSMain),
    ESP_ELFSYM_EXPORT(tos_convert_color),
    ESP_ELFSYM_EXPORT(tos_get_key),
    ESP_ELFSYM_EXPORT(tos_has_key),
    ESP_ELFSYM_EXPORT(tos_read_byte),
    ESP_ELFSYM_EXPORT(tos_systrap),
    ESP_ELFSYM_EXPORT(tos_write_byte),
    ESP_ELFSYM_EXPORT(tos_write_screen),
    ESP_ELFSYM_EXPORT(xtos_convert_color),
    ESP_ELFSYM_EXPORT(Cauxin),
    ESP_ELFSYM_EXPORT(Cauxis),
    ESP_ELFSYM_EXPORT(Cauxos),
    ESP_ELFSYM_EXPORT(Cauxout),
    ESP_ELFSYM_EXPORT(Cconin),
    ESP_ELFSYM_EXPORT(Cconis),
    ESP_ELFSYM_EXPORT(Cconos),
    ESP_ELFSYM_EXPORT(Cconout),
    ESP_ELFSYM_EXPORT(Cconrs),
    ESP_ELFSYM_EXPORT(Cconws),
    ESP_ELFSYM_EXPORT(Cnecin),
    ESP_ELFSYM_EXPORT(Cprnos),
    ESP_ELFSYM_EXPORT(Cprnout),
    ESP_ELFSYM_EXPORT(Crawcin),
    ESP_ELFSYM_EXPORT(Crawio),
    ESP_ELFSYM_EXPORT(Dchroot),
    ESP_ELFSYM_EXPORT(Dclosedir),
    ESP_ELFSYM_EXPORT(Dcntl),
    ESP_ELFSYM_EXPORT(Dcreate),
    ESP_ELFSYM_EXPORT(Ddelete),
    ESP_ELFSYM_EXPORT(Dfree),
    ESP_ELFSYM_EXPORT(Dgetcwd),
    ESP_ELFSYM_EXPORT(Dgetdrv),
    ESP_ELFSYM_EXPORT(Dgetpath),
    ESP_ELFSYM_EXPORT(Dlock),
    ESP_ELFSYM_EXPORT(Dopendir),
    ESP_ELFSYM_EXPORT(Dpathconf),
    ESP_ELFSYM_EXPORT(Dreaddir),
    ESP_ELFSYM_EXPORT(Dreadlabel),
    ESP_ELFSYM_EXPORT(Drewinddir),
    ESP_ELFSYM_EXPORT(Dsetdrv),
    ESP_ELFSYM_EXPORT(Dsetkey),
    ESP_ELFSYM_EXPORT(Dsetpath),
    ESP_ELFSYM_EXPORT(Dwritelabel),
    ESP_ELFSYM_EXPORT(Dxopendir),
    ESP_ELFSYM_EXPORT(Dxreaddir),
    ESP_ELFSYM_EXPORT(Faccept),
    ESP_ELFSYM_EXPORT(Fattrib),
    ESP_ELFSYM_EXPORT(Fbind),
    ESP_ELFSYM_EXPORT(Fchdir),
    ESP_ELFSYM_EXPORT(Fchmod),
    ESP_ELFSYM_EXPORT(Fchown),
    ESP_ELFSYM_EXPORT(Fchown16),
    ESP_ELFSYM_EXPORT(Fclose),
    ESP_ELFSYM_EXPORT(Fconnect),
    ESP_ELFSYM_EXPORT(Fcreate),
    ESP_ELFSYM_EXPORT(Fdatime),
    ESP_ELFSYM_EXPORT(Fdelete),
    ESP_ELFSYM_EXPORT(Fdirfd),
    ESP_ELFSYM_EXPORT(Fdup),
    ESP_ELFSYM_EXPORT(Ffchmod),
    ESP_ELFSYM_EXPORT(Ffchown),
    ESP_ELFSYM_EXPORT(Ffdopendir),
    ESP_ELFSYM_EXPORT(Fflush),
    ESP_ELFSYM_EXPORT(Fforce),
    ESP_ELFSYM_EXPORT(Ffstat64),
    ESP_ELFSYM_EXPORT(Fgetchar),
    ESP_ELFSYM_EXPORT(Fgetpeername),
    ESP_ELFSYM_EXPORT(Fgetsockname),
    ESP_ELFSYM_EXPORT(Fgetsockopt),
    ESP_ELFSYM_EXPORT(Finstat),
    ESP_ELFSYM_EXPORT(Flink),
    ESP_ELFSYM_EXPORT(Flisten),
    ESP_ELFSYM_EXPORT(Flock),
    ESP_ELFSYM_EXPORT(Flock2),
    ESP_ELFSYM_EXPORT(Fmidipipe),
    ESP_ELFSYM_EXPORT(Fopen),
    ESP_ELFSYM_EXPORT(Foutstat),
    ESP_ELFSYM_EXPORT(Fpipe),
    ESP_ELFSYM_EXPORT(Fpoll),
    ESP_ELFSYM_EXPORT(Fputchar),
    ESP_ELFSYM_EXPORT(Fread),
    ESP_ELFSYM_EXPORT(Freadlink),
    ESP_ELFSYM_EXPORT(Freadv),
    ESP_ELFSYM_EXPORT(Frecvfrom),
    ESP_ELFSYM_EXPORT(Frecvmsg),
    ESP_ELFSYM_EXPORT(Frename),
    ESP_ELFSYM_EXPORT(Frlock),
    ESP_ELFSYM_EXPORT(Frunlock),
    ESP_ELFSYM_EXPORT(Fseek),
    ESP_ELFSYM_EXPORT(Fseek64),
    ESP_ELFSYM_EXPORT(Fselect),
    ESP_ELFSYM_EXPORT(Fsendmsg),
    ESP_ELFSYM_EXPORT(Fsendto),
    ESP_ELFSYM_EXPORT(Fsetsockopt),
    ESP_ELFSYM_EXPORT(Fsfirst),
    ESP_ELFSYM_EXPORT(Fshutdown),
    ESP_ELFSYM_EXPORT(Fsnext),
    ESP_ELFSYM_EXPORT(Fsocket),
    ESP_ELFSYM_EXPORT(Fsocketpair),
    ESP_ELFSYM_EXPORT(Fstat64),
    ESP_ELFSYM_EXPORT(Fsymlink),
    ESP_ELFSYM_EXPORT(Fsync),
    ESP_ELFSYM_EXPORT(Funlock),
    ESP_ELFSYM_EXPORT(Fwrite),
    ESP_ELFSYM_EXPORT(Fwritev),
    ESP_ELFSYM_EXPORT(Fxattr),
    ESP_ELFSYM_EXPORT(Maccess),
    ESP_ELFSYM_EXPORT(Maddalt),
    ESP_ELFSYM_EXPORT(Mfree),
    ESP_ELFSYM_EXPORT(Mshrink),
    ESP_ELFSYM_EXPORT(Mvalidate),
    ESP_ELFSYM_EXPORT(Mxalloc),
    ESP_ELFSYM_EXPORT(Nversion),
    ESP_ELFSYM_EXPORT(Pause),
    ESP_ELFSYM_EXPORT(Pdomain),
    ESP_ELFSYM_EXPORT(Pfork),
    ESP_ELFSYM_EXPORT(Pgetauid),
    ESP_ELFSYM_EXPORT(Pgetegid),
    ESP_ELFSYM_EXPORT(Pgeteuid),
    ESP_ELFSYM_EXPORT(Pgetgid),
    ESP_ELFSYM_EXPORT(Pgetgroups),
    ESP_ELFSYM_EXPORT(Pgetpgrp),
    ESP_ELFSYM_EXPORT(Pgetpid),
    ESP_ELFSYM_EXPORT(Pgetppid),
    ESP_ELFSYM_EXPORT(Pgetpriority),
    ESP_ELFSYM_EXPORT(Pgetuid),
    ESP_ELFSYM_EXPORT(Pkill),
    ESP_ELFSYM_EXPORT(Pmsg),
    ESP_ELFSYM_EXPORT(Pnice),
    ESP_ELFSYM_EXPORT(Prenice),
    ESP_ELFSYM_EXPORT(Prusage),
    ESP_ELFSYM_EXPORT(Psemaphore),
    ESP_ELFSYM_EXPORT(Psetauid),
    ESP_ELFSYM_EXPORT(Psetegid),
    ESP_ELFSYM_EXPORT(Pseteuid),
    ESP_ELFSYM_EXPORT(Psetgid),
    ESP_ELFSYM_EXPORT(Psetgroups),
    ESP_ELFSYM_EXPORT(Psetlimit),
    ESP_ELFSYM_EXPORT(Psetpgrp),
    ESP_ELFSYM_EXPORT(Psetpriority),
    ESP_ELFSYM_EXPORT(Psetregid),
    ESP_ELFSYM_EXPORT(Psetreuid),
    ESP_ELFSYM_EXPORT(Psetuid),
    ESP_ELFSYM_EXPORT(Psigaction),
    ESP_ELFSYM_EXPORT(Psigblock),
    ESP_ELFSYM_EXPORT(Psigintr),
    ESP_ELFSYM_EXPORT(Psignal),
    ESP_ELFSYM_EXPORT(Psigpause),
    ESP_ELFSYM_EXPORT(Psigpending),
    ESP_ELFSYM_EXPORT(Psigreturn),
    ESP_ELFSYM_EXPORT(Psigsetmask),
    ESP_ELFSYM_EXPORT(Psysctl),
    ESP_ELFSYM_EXPORT(Pterm),
    ESP_ELFSYM_EXPORT(Pterm0),
    ESP_ELFSYM_EXPORT(Ptermres),
    ESP_ELFSYM_EXPORT(Ptrace),
    ESP_ELFSYM_EXPORT(Pumask),
    ESP_ELFSYM_EXPORT(Pusrval),
    ESP_ELFSYM_EXPORT(Pvfork),
    ESP_ELFSYM_EXPORT(Pwait),
    ESP_ELFSYM_EXPORT(Pwait3),
    ESP_ELFSYM_EXPORT(Pwaitpid),
    ESP_ELFSYM_EXPORT(Salert),
    ESP_ELFSYM_EXPORT(Shutdown),
    ESP_ELFSYM_EXPORT(Srealloc),
    ESP_ELFSYM_EXPORT(Ssystem),
    ESP_ELFSYM_EXPORT(Suptime),
    ESP_ELFSYM_EXPORT(Sversion),
    ESP_ELFSYM_EXPORT(Syield),
    ESP_ELFSYM_EXPORT(Sync),
    ESP_ELFSYM_EXPORT(Sysconf),
    ESP_ELFSYM_EXPORT(Tadjtime),
    ESP_ELFSYM_EXPORT(Talarm),
    ESP_ELFSYM_EXPORT(Tgetdate),
    ESP_ELFSYM_EXPORT(Tgettime),
    ESP_ELFSYM_EXPORT(Tgettimeofday),
    ESP_ELFSYM_EXPORT(Tmalarm),
    ESP_ELFSYM_EXPORT(Tsetdate),
    ESP_ELFSYM_EXPORT(Tsetitimer),
    ESP_ELFSYM_EXPORT(Tsettime),
    ESP_ELFSYM_EXPORT(Tsettimeofday),
    ESP_ELFSYM_EXPORT(Bconin),
    ESP_ELFSYM_EXPORT(Bconout),
    ESP_ELFSYM_EXPORT(Bconstat),
    ESP_ELFSYM_EXPORT(Bcostat),
    ESP_ELFSYM_EXPORT(Drvmap),
    ESP_ELFSYM_EXPORT(Getbpb),
    ESP_ELFSYM_EXPORT(Getmpb),
    ESP_ELFSYM_EXPORT(Kbshift),
    ESP_ELFSYM_EXPORT(Mediach),
    ESP_ELFSYM_EXPORT(Rwabs),
    ESP_ELFSYM_EXPORT(Setexc),
    ESP_ELFSYM_EXPORT(Tickcal),
    ESP_ELFSYM_EXPORT(Bconmap),
    ESP_ELFSYM_EXPORT(Bioskeys),
    ESP_ELFSYM_EXPORT(Blitmode),
    ESP_ELFSYM_EXPORT(CacheCtrl),
    ESP_ELFSYM_EXPORT(Cursconf),
    ESP_ELFSYM_EXPORT(DMAread),
    ESP_ELFSYM_EXPORT(DMAwrite),
    ESP_ELFSYM_EXPORT(Dbmsg),
    ESP_ELFSYM_EXPORT(Dosound),
    ESP_ELFSYM_EXPORT(Dsp_Available),
    ESP_ELFSYM_EXPORT(Dsp_BlkBytes),
    ESP_ELFSYM_EXPORT(Dsp_BlkHandShake),
    ESP_ELFSYM_EXPORT(Dsp_BlkUnpacked),
    ESP_ELFSYM_EXPORT(Dsp_BlkWords),
    ESP_ELFSYM_EXPORT(Dsp_DoBlock),
    ESP_ELFSYM_EXPORT(Dsp_ExecBoot),
    ESP_ELFSYM_EXPORT(Dsp_ExecProg),
    ESP_ELFSYM_EXPORT(Dsp_FlushSubroutines),
    ESP_ELFSYM_EXPORT(Dsp_GetProgAbility),
    ESP_ELFSYM_EXPORT(Dsp_GetWordSize),
    ESP_ELFSYM_EXPORT(Dsp_HStat),
    ESP_ELFSYM_EXPORT(Dsp_Hf0),
    ESP_ELFSYM_EXPORT(Dsp_Hf1),
    ESP_ELFSYM_EXPORT(Dsp_Hf2),
    ESP_ELFSYM_EXPORT(Dsp_Hf3),
    ESP_ELFSYM_EXPORT(Dsp_IOStream),
    ESP_ELFSYM_EXPORT(Dsp_InStream),
    ESP_ELFSYM_EXPORT(Dsp_InqSubrAbility),
    ESP_ELFSYM_EXPORT(Dsp_LoadProg),
    ESP_ELFSYM_EXPORT(Dsp_LoadSubroutine),
    ESP_ELFSYM_EXPORT(Dsp_Lock),
    ESP_ELFSYM_EXPORT(Dsp_LodToBinary),
    ESP_ELFSYM_EXPORT(Dsp_MultBlocks),
    ESP_ELFSYM_EXPORT(Dsp_OutStream),
    ESP_ELFSYM_EXPORT(Dsp_RemoveInterrupts),
    ESP_ELFSYM_EXPORT(Dsp_RequestUniqueAbility),
    ESP_ELFSYM_EXPORT(Dsp_Reserve),
    ESP_ELFSYM_EXPORT(Dsp_RunSubroutine),
    ESP_ELFSYM_EXPORT(Dsp_SetVectors),
    ESP_ELFSYM_EXPORT(Dsp_TriggerHC),
    ESP_ELFSYM_EXPORT(Dsp_Unlock),
    ESP_ELFSYM_EXPORT(EgetPalette),
    ESP_ELFSYM_EXPORT(EgetShift),
    ESP_ELFSYM_EXPORT(EsetBank),
    ESP_ELFSYM_EXPORT(EsetColor),
    ESP_ELFSYM_EXPORT(EsetGray),
    ESP_ELFSYM_EXPORT(EsetPalette),
    ESP_ELFSYM_EXPORT(EsetShift),
    ESP_ELFSYM_EXPORT(EsetSmear),
    ESP_ELFSYM_EXPORT(Flopfmt),
    ESP_ELFSYM_EXPORT(Floprate),
    ESP_ELFSYM_EXPORT(Floprd),
    ESP_ELFSYM_EXPORT(Flopver),
    ESP_ELFSYM_EXPORT(Flopwr),
    ESP_ELFSYM_EXPORT(Gettime),
    ESP_ELFSYM_EXPORT(Giaccess),
    ESP_ELFSYM_EXPORT(Ikbdws),
    ESP_ELFSYM_EXPORT(Initmouse),
    ESP_ELFSYM_EXPORT(Iorec),
    ESP_ELFSYM_EXPORT(Jdisint),
    ESP_ELFSYM_EXPORT(Jenabint),
    ESP_ELFSYM_EXPORT(Kbrate),
    ESP_ELFSYM_EXPORT(Keytbl),
    ESP_ELFSYM_EXPORT(Metaclose),
    ESP_ELFSYM_EXPORT(Metadiscinfo),
    ESP_ELFSYM_EXPORT(Metagettoc),
    ESP_ELFSYM_EXPORT(Metainit),
    ESP_ELFSYM_EXPORT(Metaioctl),
    ESP_ELFSYM_EXPORT(Metaopen),
    ESP_ELFSYM_EXPORT(Metaread),
    ESP_ELFSYM_EXPORT(Metaseek),
    ESP_ELFSYM_EXPORT(Metasetsongtime),
    ESP_ELFSYM_EXPORT(Metastartaudio),
    ESP_ELFSYM_EXPORT(Metastatus),
    ESP_ELFSYM_EXPORT(Metastopaudio),
    ESP_ELFSYM_EXPORT(Metawrite),
    ESP_ELFSYM_EXPORT(Mfpint),
    ESP_ELFSYM_EXPORT(Midiws),
    ESP_ELFSYM_EXPORT(NVMaccess),
    ESP_ELFSYM_EXPORT(Offgibit),
    ESP_ELFSYM_EXPORT(Ongibit),
    ESP_ELFSYM_EXPORT(Protobt),
    ESP_ELFSYM_EXPORT(Prtblk),
    ESP_ELFSYM_EXPORT(Puntaes),
    ESP_ELFSYM_EXPORT(Random),
    ESP_ELFSYM_EXPORT(Rsconf),
    ESP_ELFSYM_EXPORT(Scrdmp),
    ESP_ELFSYM_EXPORT(Setprt),
    ESP_ELFSYM_EXPORT(Settime),
    ESP_ELFSYM_EXPORT(Ssbrk),
    ESP_ELFSYM_EXPORT(ValidMode),
    ESP_ELFSYM_EXPORT(VgetRGB),
    ESP_ELFSYM_EXPORT(VgetSize),
    ESP_ELFSYM_EXPORT(VsetMask),
    ESP_ELFSYM_EXPORT(VsetMode),
    ESP_ELFSYM_EXPORT(VsetRGB),
    ESP_ELFSYM_EXPORT(VsetSync),
    ESP_ELFSYM_EXPORT(Vsync),
    ESP_ELFSYM_EXPORT(WavePlay),
    ESP_ELFSYM_EXPORT(WdgCtrl),
    ESP_ELFSYM_EXPORT(Xbios),
    ESP_ELFSYM_EXPORT(Xbtimer),
    ESP_ELFSYM_EXPORT(buffoper),
    ESP_ELFSYM_EXPORT(buffptr),
    ESP_ELFSYM_EXPORT(devconnect),
    ESP_ELFSYM_EXPORT(dsptristate),
    ESP_ELFSYM_EXPORT(gpio),
    ESP_ELFSYM_EXPORT(locksnd),
    ESP_ELFSYM_EXPORT(mon_type),
    ESP_ELFSYM_EXPORT(setbuffer),
    ESP_ELFSYM_EXPORT(setinterrupt),
    ESP_ELFSYM_EXPORT(setmode),
    ESP_ELFSYM_EXPORT(setmontracks),
    ESP_ELFSYM_EXPORT(settracks),
    ESP_ELFSYM_EXPORT(sndstatus),
    ESP_ELFSYM_EXPORT(soundcmd),
    ESP_ELFSYM_EXPORT(unlocksnd),
    ESP_ELFSYM_EXPORT(vdi_call),
    ESP_ELFSYM_EXPORT(aes_call),
    ESP_ELFSYM_EXPORT(kd_clear),
    ESP_ELFSYM_EXPORT(kd_create),
    ESP_ELFSYM_EXPORT(kd_data_destructor),
    ESP_ELFSYM_EXPORT(kd_free),
    ESP_ELFSYM_EXPORT(kd_insert),
    ESP_ELFSYM_EXPORT(kd_insert3),
    ESP_ELFSYM_EXPORT(kd_nearest),
    ESP_ELFSYM_EXPORT(kd_nearest3),
    ESP_ELFSYM_EXPORT(kd_nearest_range),
    ESP_ELFSYM_EXPORT(kd_nearest_range3),
    ESP_ELFSYM_EXPORT(kd_res_end),
    ESP_ELFSYM_EXPORT(kd_res_free),
    ESP_ELFSYM_EXPORT(kd_res_item),
    ESP_ELFSYM_EXPORT(kd_res_item3),
    ESP_ELFSYM_EXPORT(kd_res_item_data),
    ESP_ELFSYM_EXPORT(kd_res_next),
    ESP_ELFSYM_EXPORT(kd_res_rewind),
    ESP_ELFSYM_EXPORT(kd_res_size),
    ESP_ELFSYM_END
};
