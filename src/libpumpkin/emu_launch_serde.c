/* This file was generated from "launchcodes.struct". Do not edit. */
#include <PalmOS.h>
#include <VFSMgr.h>
#include <GPSLib.h>

#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "logtrap.h"
#include "m68k.h"
#include "m68kcpu.h"
#include "emupalmosinc.h"
#include "emupalmos.h"
#include "launch_serde.h"
#include "emu_launch_serde.h"

void decode_sysAppLaunchCmdSystemReset(UInt32 buf, SysAppLaunchCmdSystemResetType *param) {
  UInt32 offset = 0;
  MemSet(param, sizeof(SysAppLaunchCmdSystemResetType), 0);
  param->hardReset = m68k_read_memory_8(buf + offset);
  offset += 1;
  param->createDefaultDB = m68k_read_memory_8(buf + offset);
  offset += 1;
}

void encode_sysAppLaunchCmdSystemReset(UInt32 buf, SysAppLaunchCmdSystemResetType *param) {
  UInt32 offset = 0;
  m68k_write_memory_8(buf + offset, param->hardReset);
  offset += 1;
  m68k_write_memory_8(buf + offset, param->createDefaultDB);
  offset += 1;
}

void decode_sysAppLaunchCmdAlarmTriggered(UInt32 buf, SysAlarmTriggeredParamType *param) {
  UInt32 offset = 0;
  MemSet(param, sizeof(SysAlarmTriggeredParamType), 0);
  param->ref = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->alarmSeconds = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->purgeAlarm = m68k_read_memory_8(buf + offset);
  offset += 1;
  param->padding = m68k_read_memory_8(buf + offset);
  offset += 1;
}

void encode_sysAppLaunchCmdAlarmTriggered(UInt32 buf, SysAlarmTriggeredParamType *param) {
  UInt32 offset = 0;
  m68k_write_memory_32(buf + offset, param->ref);
  offset += 4;
  m68k_write_memory_32(buf + offset, param->alarmSeconds);
  offset += 4;
  m68k_write_memory_8(buf + offset, param->purgeAlarm);
  offset += 1;
  m68k_write_memory_8(buf + offset, param->padding);
  offset += 1;
}

void decode_sysAppLaunchCmdDisplayAlarm(UInt32 buf, SysDisplayAlarmParamType *param) {
  UInt32 offset = 0;
  MemSet(param, sizeof(SysDisplayAlarmParamType), 0);
  param->ref = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->alarmSeconds = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->soundAlarm = m68k_read_memory_8(buf + offset);
  offset += 1;
  param->padding = m68k_read_memory_8(buf + offset);
  offset += 1;
}

void encode_sysAppLaunchCmdDisplayAlarm(UInt32 buf, SysDisplayAlarmParamType *param) {
  UInt32 offset = 0;
  m68k_write_memory_32(buf + offset, param->ref);
  offset += 4;
  m68k_write_memory_32(buf + offset, param->alarmSeconds);
  offset += 4;
  m68k_write_memory_8(buf + offset, param->soundAlarm);
  offset += 1;
  m68k_write_memory_8(buf + offset, param->padding);
  offset += 1;
}

void decode_sysAppLaunchCmdGoTo(UInt32 buf, GoToParamsType *param) {
  UInt32 offset = 0;
  MemSet(param, sizeof(GoToParamsType), 0);
  param->searchStrLen = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->dbCardNo = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->dbID = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->recordNum = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->matchPos = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->matchFieldNum = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->matchCustom = m68k_read_memory_32(buf + offset);
  offset += 4;
}

void encode_sysAppLaunchCmdGoTo(UInt32 buf, GoToParamsType *param) {
  UInt32 offset = 0;
  m68k_write_memory_16(buf + offset, param->searchStrLen);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->dbCardNo);
  offset += 2;
  m68k_write_memory_32(buf + offset, param->dbID);
  offset += 4;
  m68k_write_memory_16(buf + offset, param->recordNum);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->matchPos);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->matchFieldNum);
  offset += 2;
  m68k_write_memory_32(buf + offset, param->matchCustom);
  offset += 4;
}

void decode_sysAppLaunchCmdPanelCalledFromApp(UInt32 buf, PanelAppType *param) {
  UInt32 offset = 0;
  UInt32 i = 0;
  MemSet(param, sizeof(PanelAppType), 0);
  for (i = 0; i < 32; i++) {
    param->name[i] = m68k_read_memory_8(buf + offset + i);
  }
  offset += 32;
}

void encode_sysAppLaunchCmdPanelCalledFromApp(UInt32 buf, PanelAppType *param) {
  UInt32 offset = 0;
  UInt32 i = 0;
  for (i = 0; i < 32; i++) {
  m68k_write_memory_8(buf + offset + i, param->name[i]);
  }
  offset += 32;
}

int decode_launch(
UInt32 paramType, UInt32 buf, launch_union_t *param) {
  UInt32 i = 0;
  int r = 0;
  switch (paramType) {
    case sysAppLaunchCmdSystemReset:
      decode_sysAppLaunchCmdSystemReset(buf, &param->p0);
      break;
    case sysAppLaunchCmdAlarmTriggered:
      decode_sysAppLaunchCmdAlarmTriggered(buf, &param->p1);
      break;
    case sysAppLaunchCmdDisplayAlarm:
      decode_sysAppLaunchCmdDisplayAlarm(buf, &param->p2);
      break;
    case sysAppLaunchCmdGoTo:
      decode_sysAppLaunchCmdGoTo(buf, &param->p3);
      break;
    case sysAppLaunchCmdPanelCalledFromApp:
      decode_sysAppLaunchCmdPanelCalledFromApp(buf, &param->p4);
      break;
    default:
      if (paramType >= sysAppLaunchCmdCustomBase) {
        for (i = 0; i < 64; i++) {
          param->buf[i] = m68k_read_memory_8(buf + i);
        }
      } else {
        r = -1;
      }
      break;
  }
  return r;
}

int encode_launch(UInt32 paramType, UInt32 buf, launch_union_t *param) {
  int r = 0;
  switch (paramType) {
    case sysAppLaunchCmdSystemReset:
      encode_sysAppLaunchCmdSystemReset(buf, &param->p0);
      break;
    case sysAppLaunchCmdAlarmTriggered:
      encode_sysAppLaunchCmdAlarmTriggered(buf, &param->p1);
      break;
    case sysAppLaunchCmdDisplayAlarm:
      encode_sysAppLaunchCmdDisplayAlarm(buf, &param->p2);
      break;
    case sysAppLaunchCmdGoTo:
      encode_sysAppLaunchCmdGoTo(buf, &param->p3);
      break;
    case sysAppLaunchCmdPanelCalledFromApp:
      encode_sysAppLaunchCmdPanelCalledFromApp(buf, &param->p4);
      break;
    default:
      r = -1;
      break;
  }
  return r;
}
