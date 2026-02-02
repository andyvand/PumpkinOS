/* This file was generated from "notifications.struct". Do not edit. */
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
#include "notif_serde.h"
#include "emu_notif_serde.h"

void decode_sysNotifyAppLaunchingEvent(UInt32 buf, SysNotifyAppLaunchOrQuitType *param) {
  UInt32 offset = 0;
  MemSet(param, sizeof(SysNotifyAppLaunchOrQuitType), 0);
  param->version = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->dbID = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->cardNo = m68k_read_memory_16(buf + offset);
  offset += 2;
}

void encode_sysNotifyAppLaunchingEvent(UInt32 buf, SysNotifyAppLaunchOrQuitType *param) {
  UInt32 offset = 0;
  m68k_write_memory_32(buf + offset, param->version);
  offset += 4;
  m68k_write_memory_32(buf + offset, param->dbID);
  offset += 4;
  m68k_write_memory_16(buf + offset, param->cardNo);
  offset += 2;
}

void decode_sysNotifyAppQuittingEvent(UInt32 buf, SysNotifyAppLaunchOrQuitType *param) {
  UInt32 offset = 0;
  MemSet(param, sizeof(SysNotifyAppLaunchOrQuitType), 0);
  param->version = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->dbID = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->cardNo = m68k_read_memory_16(buf + offset);
  offset += 2;
}

void encode_sysNotifyAppQuittingEvent(UInt32 buf, SysNotifyAppLaunchOrQuitType *param) {
  UInt32 offset = 0;
  m68k_write_memory_32(buf + offset, param->version);
  offset += 4;
  m68k_write_memory_32(buf + offset, param->dbID);
  offset += 4;
  m68k_write_memory_16(buf + offset, param->cardNo);
  offset += 2;
}

void decode_sysNotifyAppCrashedEvent(UInt32 buf, SysNotifyAppLaunchOrQuitType *param) {
  UInt32 offset = 0;
  MemSet(param, sizeof(SysNotifyAppLaunchOrQuitType), 0);
  param->version = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->dbID = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->cardNo = m68k_read_memory_16(buf + offset);
  offset += 2;
}

void encode_sysNotifyAppCrashedEvent(UInt32 buf, SysNotifyAppLaunchOrQuitType *param) {
  UInt32 offset = 0;
  m68k_write_memory_32(buf + offset, param->version);
  offset += 4;
  m68k_write_memory_32(buf + offset, param->dbID);
  offset += 4;
  m68k_write_memory_16(buf + offset, param->cardNo);
  offset += 2;
}

void decode_sysNotifyDisplayChangeEvent(UInt32 buf, SysNotifyDisplayChangeDetailsType *param) {
  UInt32 offset = 0;
  MemSet(param, sizeof(SysNotifyDisplayChangeDetailsType), 0);
  param->oldDepth = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->newDepth = m68k_read_memory_32(buf + offset);
  offset += 4;
}

void encode_sysNotifyDisplayChangeEvent(UInt32 buf, SysNotifyDisplayChangeDetailsType *param) {
  UInt32 offset = 0;
  m68k_write_memory_32(buf + offset, param->oldDepth);
  offset += 4;
  m68k_write_memory_32(buf + offset, param->newDepth);
  offset += 4;
}

void decode_sysNotifyDBCreatedEvent(UInt32 buf, SysNotifyDBCreatedType *param) {
  UInt32 offset = 0;
  UInt32 i = 0;
  MemSet(param, sizeof(SysNotifyDBCreatedType), 0);
  for (i = 0; i < dmDBNameLength; i++) {
    param->dbName[i] = m68k_read_memory_8(buf + offset + i);
  }
  offset += dmDBNameLength;
  param->creator = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->type = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->newDBID = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->cardNo = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->resDB = m68k_read_memory_8(buf + offset);
  offset += 1;
  param->padding = m68k_read_memory_8(buf + offset);
  offset += 1;
}

void encode_sysNotifyDBCreatedEvent(UInt32 buf, SysNotifyDBCreatedType *param) {
  UInt32 offset = 0;
  UInt32 i = 0;
  for (i = 0; i < dmDBNameLength; i++) {
  m68k_write_memory_8(buf + offset + i, param->dbName[i]);
  }
  offset += dmDBNameLength;
  m68k_write_memory_32(buf + offset, param->creator);
  offset += 4;
  m68k_write_memory_32(buf + offset, param->type);
  offset += 4;
  m68k_write_memory_32(buf + offset, param->newDBID);
  offset += 4;
  m68k_write_memory_16(buf + offset, param->cardNo);
  offset += 2;
  m68k_write_memory_8(buf + offset, param->resDB);
  offset += 1;
  m68k_write_memory_8(buf + offset, param->padding);
  offset += 1;
}

void decode_sysNotifyDBDeletedEvent(UInt32 buf, SysNotifyDBDeletedType *param) {
  UInt32 offset = 0;
  UInt32 i = 0;
  MemSet(param, sizeof(SysNotifyDBDeletedType), 0);
  param->oldDBID = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->cardNo = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->attributes = m68k_read_memory_16(buf + offset);
  offset += 2;
  for (i = 0; i < dmDBNameLength; i++) {
    param->dbName[i] = m68k_read_memory_8(buf + offset + i);
  }
  offset += dmDBNameLength;
  param->creator = m68k_read_memory_32(buf + offset);
  offset += 4;
  param->type = m68k_read_memory_32(buf + offset);
  offset += 4;
}

void encode_sysNotifyDBDeletedEvent(UInt32 buf, SysNotifyDBDeletedType *param) {
  UInt32 offset = 0;
  UInt32 i = 0;
  m68k_write_memory_32(buf + offset, param->oldDBID);
  offset += 4;
  m68k_write_memory_16(buf + offset, param->cardNo);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->attributes);
  offset += 2;
  for (i = 0; i < dmDBNameLength; i++) {
  m68k_write_memory_8(buf + offset + i, param->dbName[i]);
  }
  offset += dmDBNameLength;
  m68k_write_memory_32(buf + offset, param->creator);
  offset += 4;
  m68k_write_memory_32(buf + offset, param->type);
  offset += 4;
}

void decode_sysNotifyTimeChangeEvent(UInt32 buf, Int32 *param) {
  *param = m68k_read_memory_32(buf);
}

void encode_sysNotifyTimeChangeEvent(UInt32 buf, Int32 *param) {
  m68k_write_memory_32(buf, *param);
}

void decode_sysNotifyGPSDataEvent(UInt32 buf, UInt16 *param) {
  *param = m68k_read_memory_16(buf);
}

void encode_sysNotifyGPSDataEvent(UInt32 buf, UInt16 *param) {
  m68k_write_memory_16(buf, *param);
}

void decode_sysNotifyVolumeMountedEvent(UInt32 buf, VFSAnyMountParamType *param) {
  UInt32 offset = 0;
  MemSet(param, sizeof(VFSAnyMountParamType), 0);
  param->volRefNum = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->reserved = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->mountClass = m68k_read_memory_32(buf + offset);
  offset += 4;
}

void encode_sysNotifyVolumeMountedEvent(UInt32 buf, VFSAnyMountParamType *param) {
  UInt32 offset = 0;
  m68k_write_memory_16(buf + offset, param->volRefNum);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->reserved);
  offset += 2;
  m68k_write_memory_32(buf + offset, param->mountClass);
  offset += 4;
}

void decode_sysNotifyCardInsertedEvent(UInt32 buf, UInt16 *param) {
  *param = (UInt16)buf;
}

void encode_sysNotifyCardInsertedEvent(UInt32 buf, UInt16 *param) {
}

void decode_sysNotifyCardRemovedEvent(UInt32 buf, UInt16 *param) {
  *param = (UInt16)buf;
}

void encode_sysNotifyCardRemovedEvent(UInt32 buf, UInt16 *param) {
}

void decode_sysNotifyVolumeUnmountedEvent(UInt32 buf, UInt16 *param) {
  *param = (UInt16)buf;
}

void encode_sysNotifyVolumeUnmountedEvent(UInt32 buf, UInt16 *param) {
}

void decode_sysNotifyLocaleChangedEvent(UInt32 buf, SysNotifyLocaleChangedType *param) {
  UInt32 offset = 0;
  MemSet(param, sizeof(SysNotifyLocaleChangedType), 0);
  param->oldLocale.language = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->oldLocale.country = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->newLocale.language = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->newLocale.country = m68k_read_memory_16(buf + offset);
  offset += 2;
}

void encode_sysNotifyLocaleChangedEvent(UInt32 buf, SysNotifyLocaleChangedType *param) {
  UInt32 offset = 0;
  m68k_write_memory_16(buf + offset, param->oldLocale.language);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->oldLocale.country);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->newLocale.language);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->newLocale.country);
  offset += 2;
}

void decode_sysNotifySelectDay(UInt32 buf, SysNotifySelectDayDetailsType *param) {
  UInt32 addr = 0;
  UInt32 offset = 0;
  MemSet(param, sizeof(SysNotifySelectDayDetailsType), 0);
  param->daySelectorP->bounds.topLeft.x = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->daySelectorP->bounds.topLeft.y = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->daySelectorP->bounds.extent.x = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->daySelectorP->bounds.extent.x = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->daySelectorP->visible = m68k_read_memory_8(buf + offset);
  offset += 1;
  param->daySelectorP->reserved1 = m68k_read_memory_8(buf + offset);
  offset += 1;
  param->daySelectorP->visibleMonth = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->daySelectorP->visibleYear = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->daySelectorP->selected.second = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->daySelectorP->selected.minute = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->daySelectorP->selected.hour = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->daySelectorP->selected.day = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->daySelectorP->selected.month = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->daySelectorP->selected.year = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->daySelectorP->selected.weekDay = m68k_read_memory_16(buf + offset);
  offset += 2;
  param->daySelectorP->selectDayBy = m68k_read_memory_8(buf + offset);
  offset += 1;
  param->daySelectorP->reserved2 = m68k_read_memory_8(buf + offset);
  offset += 1;
  addr = m68k_read_memory_32(buf + offset);
  param->titleP = addr ? (char *)pumpkin_heap_base() + addr : NULL;
  offset += 4;
  param->dayChanged = m68k_read_memory_8(buf + offset);
  offset += 1;
  param->padding[0] = m68k_read_memory_8(buf + offset);
  offset += 1;
  param->padding[1] = m68k_read_memory_8(buf + offset);
  offset += 1;
  param->padding[2] = m68k_read_memory_8(buf + offset);
  offset += 1;
}

void encode_sysNotifySelectDay(UInt32 buf, SysNotifySelectDayDetailsType *param) {
  UInt32 offset = 0;
  m68k_write_memory_16(buf + offset, param->daySelectorP->bounds.topLeft.x);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->daySelectorP->bounds.topLeft.y);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->daySelectorP->bounds.extent.x);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->daySelectorP->bounds.extent.x);
  offset += 2;
  m68k_write_memory_8(buf + offset, param->daySelectorP->visible);
  offset += 1;
  m68k_write_memory_8(buf + offset, param->daySelectorP->reserved1);
  offset += 1;
  m68k_write_memory_16(buf + offset, param->daySelectorP->visibleMonth);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->daySelectorP->visibleYear);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->daySelectorP->selected.second);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->daySelectorP->selected.minute);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->daySelectorP->selected.hour);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->daySelectorP->selected.day);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->daySelectorP->selected.month);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->daySelectorP->selected.year);
  offset += 2;
  m68k_write_memory_16(buf + offset, param->daySelectorP->selected.weekDay);
  offset += 2;
  m68k_write_memory_8(buf + offset, param->daySelectorP->selectDayBy);
  offset += 1;
  m68k_write_memory_8(buf + offset, param->daySelectorP->reserved2);
  offset += 1;
  m68k_write_memory_32(buf + offset, param->titleP ? (uint32_t)(param->titleP - (char *)pumpkin_heap_base()) : 0);
  offset += 4;
  m68k_write_memory_8(buf + offset, param->dayChanged);
  offset += 1;
  m68k_write_memory_8(buf + offset, param->padding[0]);
  offset += 1;
  m68k_write_memory_8(buf + offset, param->padding[1]);
  offset += 1;
  m68k_write_memory_8(buf + offset, param->padding[2]);
  offset += 1;
}

void decode_sysNotifySyncFinishEvent(UInt32 buf, void *param) {
}

void encode_sysNotifySyncFinishEvent(UInt32 buf, void *param) {
}

int decode_notif(UInt32 paramType, UInt32 buf, notif_union_t *param) {
  int r = 0;
  UInt32 i = 0;
  switch (paramType) {
    case sysNotifyAppLaunchingEvent:
      decode_sysNotifyAppLaunchingEvent(buf, &param->p0);
      break;
    case sysNotifyAppQuittingEvent:
      decode_sysNotifyAppQuittingEvent(buf, &param->p1);
      break;
    case sysNotifyAppCrashedEvent:
      decode_sysNotifyAppCrashedEvent(buf, &param->p2);
      break;
    case sysNotifyDisplayChangeEvent:
      decode_sysNotifyDisplayChangeEvent(buf, &param->p3);
      break;
    case sysNotifyDBCreatedEvent:
      decode_sysNotifyDBCreatedEvent(buf, &param->p4);
      break;
    case sysNotifyDBDeletedEvent:
      decode_sysNotifyDBDeletedEvent(buf, &param->p5);
      break;
    case sysNotifyTimeChangeEvent:
      decode_sysNotifyTimeChangeEvent(buf, &param->p6);
      break;
    case sysNotifyGPSDataEvent:
      decode_sysNotifyGPSDataEvent(buf, &param->p7);
      break;
    case sysNotifyVolumeMountedEvent:
      decode_sysNotifyVolumeMountedEvent(buf, &param->p8);
      break;
    case sysNotifyCardInsertedEvent:
      decode_sysNotifyCardInsertedEvent(buf, &param->p9);
      break;
    case sysNotifyCardRemovedEvent:
      decode_sysNotifyCardRemovedEvent(buf, &param->p10);
      break;
    case sysNotifyVolumeUnmountedEvent:
      decode_sysNotifyVolumeUnmountedEvent(buf, &param->p11);
      break;
    case sysNotifyLocaleChangedEvent:
      decode_sysNotifyLocaleChangedEvent(buf, &param->p12);
      break;
    case sysNotifySelectDay:
      decode_sysNotifySelectDay(buf, &param->p13);
      break;
    case sysNotifySyncFinishEvent:
      decode_sysNotifySyncFinishEvent(buf, &param->p14);
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

int encode_notif(UInt32 paramType, UInt32 buf, notif_union_t *param) {
  int r = 0;
  switch (paramType) {
    case sysNotifyAppLaunchingEvent:
      encode_sysNotifyAppLaunchingEvent(buf, &param->p0);
      break;
    case sysNotifyAppQuittingEvent:
      encode_sysNotifyAppQuittingEvent(buf, &param->p1);
      break;
    case sysNotifyAppCrashedEvent:
      encode_sysNotifyAppCrashedEvent(buf, &param->p2);
      break;
    case sysNotifyDisplayChangeEvent:
      encode_sysNotifyDisplayChangeEvent(buf, &param->p3);
      break;
    case sysNotifyDBCreatedEvent:
      encode_sysNotifyDBCreatedEvent(buf, &param->p4);
      break;
    case sysNotifyDBDeletedEvent:
      encode_sysNotifyDBDeletedEvent(buf, &param->p5);
      break;
    case sysNotifyTimeChangeEvent:
      encode_sysNotifyTimeChangeEvent(buf, &param->p6);
      break;
    case sysNotifyGPSDataEvent:
      encode_sysNotifyGPSDataEvent(buf, &param->p7);
      break;
    case sysNotifyVolumeMountedEvent:
      encode_sysNotifyVolumeMountedEvent(buf, &param->p8);
      break;
    case sysNotifyCardInsertedEvent:
      encode_sysNotifyCardInsertedEvent(buf, &param->p9);
      break;
    case sysNotifyCardRemovedEvent:
      encode_sysNotifyCardRemovedEvent(buf, &param->p10);
      break;
    case sysNotifyVolumeUnmountedEvent:
      encode_sysNotifyVolumeUnmountedEvent(buf, &param->p11);
      break;
    case sysNotifyLocaleChangedEvent:
      encode_sysNotifyLocaleChangedEvent(buf, &param->p12);
      break;
    case sysNotifySelectDay:
      encode_sysNotifySelectDay(buf, &param->p13);
      break;
    case sysNotifySyncFinishEvent:
      encode_sysNotifySyncFinishEvent(buf, &param->p14);
      break;
    default:
      r = -1;
      break;
  }
  return r;
}
