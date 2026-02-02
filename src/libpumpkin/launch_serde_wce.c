/* This file was generated from "launchcodes.struct". Do not edit. */
#include <PalmOS.h>
#include <VFSMgr.h>
#include <GPSLib.h>

#include "bytes.h"
#include "launch_serde.h"

uint8_t *serialize_sysAppLaunchCmdSystemReset(void *p, UInt32 *size) {
  SysAppLaunchCmdSystemResetType *param = p;
  uint8_t *buf = sys_calloc(1, *size + 2);
  UInt32 i = *size;
  i += put1(param->hardReset, buf, i);
  i += put1(param->createDefaultDB, buf, i);
  *size = i;
  return buf;
}

int deserialize_sysAppLaunchCmdSystemReset(uint8_t *buf, UInt32 size, SysAppLaunchCmdSystemResetType *param) {
  int r = -1;
  if (size >= 2) {
    UInt32 i = 0;
    i += get1((uint8_t *)&param->hardReset, buf, i);
    i += get1((uint8_t *)&param->createDefaultDB, buf, i);
    r = 0;
  }
  return r;
}

uint8_t *serialize_sysAppLaunchCmdAlarmTriggered(void *p, UInt32 *size) {
  SysAlarmTriggeredParamType *param = p;
  uint8_t *buf = sys_calloc(1, *size + 10);
  UInt32 i = *size;
  i += put4b(param->ref, buf, i);
  i += put4b(param->alarmSeconds, buf, i);
  i += put1(param->purgeAlarm, buf, i);
  i += put1(param->padding, buf, i);
  *size = i;
  return buf;
}

int deserialize_sysAppLaunchCmdAlarmTriggered(uint8_t *buf, UInt32 size, SysAlarmTriggeredParamType *param) {
  int r = -1;
  if (size >= 10) {
    UInt32 i = 0;
    i += get4b((uint32_t *)&param->ref, buf, i);
    i += get4b((uint32_t *)&param->alarmSeconds, buf, i);
    i += get1((uint8_t *)&param->purgeAlarm, buf, i);
    i += get1((uint8_t *)&param->padding, buf, i);
    r = 0;
  }
  return r;
}

uint8_t *serialize_sysAppLaunchCmdDisplayAlarm(void *p, UInt32 *size) {
  SysDisplayAlarmParamType *param = p;
  uint8_t *buf = sys_calloc(1, *size + 10);
  UInt32 i = *size;
  i += put4b(param->ref, buf, i);
  i += put4b(param->alarmSeconds, buf, i);
  i += put1(param->soundAlarm, buf, i);
  i += put1(param->padding, buf, i);
  *size = i;
  return buf;
}

int deserialize_sysAppLaunchCmdDisplayAlarm(uint8_t *buf, UInt32 size, SysDisplayAlarmParamType *param) {
  int r = -1;
  if (size >= 10) {
    UInt32 i = 0;
    i += get4b((uint32_t *)&param->ref, buf, i);
    i += get4b((uint32_t *)&param->alarmSeconds, buf, i);
    i += get1((uint8_t *)&param->soundAlarm, buf, i);
    i += get1((uint8_t *)&param->padding, buf, i);
    r = 0;
  }
  return r;
}

uint8_t *serialize_sysAppLaunchCmdGoTo(void *p, UInt32 *size) {
  GoToParamsType *param = p;
  uint8_t *buf = sys_calloc(1, *size + 18);
  UInt32 i = *size;
  i += put2b(param->searchStrLen, buf, i);
  i += put2b(param->dbCardNo, buf, i);
  i += put4b(param->dbID, buf, i);
  i += put2b(param->recordNum, buf, i);
  i += put2b(param->matchPos, buf, i);
  i += put2b(param->matchFieldNum, buf, i);
  i += put4b(param->matchCustom, buf, i);
  *size = i;
  return buf;
}

int deserialize_sysAppLaunchCmdGoTo(uint8_t *buf, UInt32 size, GoToParamsType *param) {
  int r = -1;
  if (size >= 18) {
    UInt32 i = 0;
    i += get2b((uint16_t *)&param->searchStrLen, buf, i);
    i += get2b((uint16_t *)&param->dbCardNo, buf, i);
    i += get4b((uint32_t *)&param->dbID, buf, i);
    i += get2b((uint16_t *)&param->recordNum, buf, i);
    i += get2b((uint16_t *)&param->matchPos, buf, i);
    i += get2b((uint16_t *)&param->matchFieldNum, buf, i);
    i += get4b((uint32_t *)&param->matchCustom, buf, i);
    r = 0;
  }
  return r;
}

uint8_t *serialize_sysAppLaunchCmdPanelCalledFromApp(void *p, UInt32 *size) {
  PanelAppType *param = p;
  uint8_t *buf = sys_calloc(1, *size + 0 + 32);
  UInt32 i = *size;
  sys_memcpy(buf + i, param->name, 32);
  i += 32;
  *size = i;
  return buf;
}

int deserialize_sysAppLaunchCmdPanelCalledFromApp(uint8_t *buf, UInt32 size, PanelAppType *param) {
  int r = -1;
  if (size >= 0 + 32) {
    UInt32 i = 0;
    sys_memcpy(param->name, buf + i, 32);
    i += 32;
    r = 0;
  }
  return r;
}

uint8_t *serialize_launch(UInt32 paramType, void *param, UInt32 *size) {
  switch (paramType) {
    case sysAppLaunchCmdSystemReset:
      return serialize_sysAppLaunchCmdSystemReset(param, size);
    case sysAppLaunchCmdAlarmTriggered:
      return serialize_sysAppLaunchCmdAlarmTriggered(param, size);
    case sysAppLaunchCmdDisplayAlarm:
      return serialize_sysAppLaunchCmdDisplayAlarm(param, size);
    case sysAppLaunchCmdGoTo:
      return serialize_sysAppLaunchCmdGoTo(param, size);
    case sysAppLaunchCmdPanelCalledFromApp:
      return serialize_sysAppLaunchCmdPanelCalledFromApp(param, size);
    default:
      break;
  }
  return NULL;
}

int deserialize_launch(UInt32 paramType, void *buf, UInt32 size, launch_union_t *param) {
  switch (paramType) {
    case sysAppLaunchCmdSystemReset:
      return deserialize_sysAppLaunchCmdSystemReset(buf, size, &param->p0);
    case sysAppLaunchCmdAlarmTriggered:
      return deserialize_sysAppLaunchCmdAlarmTriggered(buf, size, &param->p1);
    case sysAppLaunchCmdDisplayAlarm:
      return deserialize_sysAppLaunchCmdDisplayAlarm(buf, size, &param->p2);
    case sysAppLaunchCmdGoTo:
      return deserialize_sysAppLaunchCmdGoTo(buf, size, &param->p3);
    case sysAppLaunchCmdPanelCalledFromApp:
      return deserialize_sysAppLaunchCmdPanelCalledFromApp(buf, size, &param->p4);
    default:
      break;
  }
  return -1;
}
