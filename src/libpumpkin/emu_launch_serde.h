/* This file was generated from "launchcodes.struct". Do not edit. */
void decode_sysAppLaunchCmdSystemReset(UInt32 buf, SysAppLaunchCmdSystemResetType *param);
void encode_sysAppLaunchCmdSystemReset(UInt32 buf, SysAppLaunchCmdSystemResetType *param);
void decode_sysAppLaunchCmdAlarmTriggered(UInt32 buf, SysAlarmTriggeredParamType *param);
void encode_sysAppLaunchCmdAlarmTriggered(UInt32 buf, SysAlarmTriggeredParamType *param);
void decode_sysAppLaunchCmdDisplayAlarm(UInt32 buf, SysDisplayAlarmParamType *param);
void encode_sysAppLaunchCmdDisplayAlarm(UInt32 buf, SysDisplayAlarmParamType *param);
void decode_sysAppLaunchCmdGoTo(UInt32 buf, GoToParamsType *param);
void encode_sysAppLaunchCmdGoTo(UInt32 buf, GoToParamsType *param);
void decode_sysAppLaunchCmdPanelCalledFromApp(UInt32 buf, PanelAppType *param);
void encode_sysAppLaunchCmdPanelCalledFromApp(UInt32 buf, PanelAppType *param);

int decode_launch(UInt32 paramType, UInt32 buf, launch_union_t *param);
int encode_launch(UInt32 paramType, UInt32 buf, launch_union_t *param);
