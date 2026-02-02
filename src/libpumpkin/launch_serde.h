/* This file was generated from "launchcodes.struct". Do not edit. */
#ifndef launch_serde_h
#define launch_serde_h
uint8_t *serialize_sysAppLaunchCmdSystemReset(void *p, UInt32 *size);
int deserialize_sysAppLaunchCmdSystemReset(uint8_t *buf, UInt32 size, SysAppLaunchCmdSystemResetType *param);
uint8_t *serialize_sysAppLaunchCmdAlarmTriggered(void *p, UInt32 *size);
int deserialize_sysAppLaunchCmdAlarmTriggered(uint8_t *buf, UInt32 size, SysAlarmTriggeredParamType *param);
uint8_t *serialize_sysAppLaunchCmdDisplayAlarm(void *p, UInt32 *size);
int deserialize_sysAppLaunchCmdDisplayAlarm(uint8_t *buf, UInt32 size, SysDisplayAlarmParamType *param);
uint8_t *serialize_sysAppLaunchCmdGoTo(void *p, UInt32 *size);
int deserialize_sysAppLaunchCmdGoTo(uint8_t *buf, UInt32 size, GoToParamsType *param);
uint8_t *serialize_sysAppLaunchCmdPanelCalledFromApp(void *p, UInt32 *size);
int deserialize_sysAppLaunchCmdPanelCalledFromApp(uint8_t *buf, UInt32 size, PanelAppType *param);

typedef union {
  SysAppLaunchCmdSystemResetType p0;
  SysAlarmTriggeredParamType p1;
  SysDisplayAlarmParamType p2;
  GoToParamsType p3;
  PanelAppType p4;
  uint8_t buf[64];
} launch_union_t;

uint8_t *serialize_launch(UInt32 paramType, void *param, UInt32 *size);
int deserialize_launch(UInt32 paramType, void *buf, UInt32 size, launch_union_t *param);
#endif
