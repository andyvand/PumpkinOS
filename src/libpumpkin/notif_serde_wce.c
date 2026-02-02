/* This file was generated from "notifications.struct". Do not edit. */
#include <PalmOS.h>
#include <VFSMgr.h>
#include <GPSLib.h>

#include "bytes.h"
#include "notif_serde.h"

uint8_t *serialize_sysNotifyAppLaunchingEvent(void *p, UInt32 *size) {
  SysNotifyAppLaunchOrQuitType *param = p;
  uint8_t *buf = sys_calloc(1, *size + 10);
  UInt32 i = *size;
  i += put4b(param->version, buf, i);
  i += put4b(param->dbID, buf, i);
  i += put2b(param->cardNo, buf, i);
  *size = i;
  return buf;
}

int deserialize_sysNotifyAppLaunchingEvent(uint8_t *buf, UInt32 size, SysNotifyAppLaunchOrQuitType *param) {
  int r = -1;
  if (size >= 10) {
    UInt32 i = 0;
    i += get4b((uint32_t *)&param->version, buf, i);
    i += get4b((uint32_t *)&param->dbID, buf, i);
    i += get2b((uint16_t *)&param->cardNo, buf, i);
    r = 0;
  }
  return r;
}

uint8_t *serialize_sysNotifyAppQuittingEvent(void *p, UInt32 *size) {
  SysNotifyAppLaunchOrQuitType *param = p;
  uint8_t *buf = sys_calloc(1, *size + 10);
  UInt32 i = *size;
  i += put4b(param->version, buf, i);
  i += put4b(param->dbID, buf, i);
  i += put2b(param->cardNo, buf, i);
  *size = i;
  return buf;
}

int deserialize_sysNotifyAppQuittingEvent(uint8_t *buf, UInt32 size, SysNotifyAppLaunchOrQuitType *param) {
  int r = -1;
  if (size >= 10) {
    UInt32 i = 0;
    i += get4b((uint32_t *)&param->version, buf, i);
    i += get4b((uint32_t *)&param->dbID, buf, i);
    i += get2b((uint16_t *)&param->cardNo, buf, i);
    r = 0;
  }
  return r;
}

uint8_t *serialize_sysNotifyAppCrashedEvent(void *p, UInt32 *size) {
  SysNotifyAppLaunchOrQuitType *param = p;
  uint8_t *buf = sys_calloc(1, *size + 10);
  UInt32 i = *size;
  i += put4b(param->version, buf, i);
  i += put4b(param->dbID, buf, i);
  i += put2b(param->cardNo, buf, i);
  *size = i;
  return buf;
}

int deserialize_sysNotifyAppCrashedEvent(uint8_t *buf, UInt32 size, SysNotifyAppLaunchOrQuitType *param) {
  int r = -1;
  if (size >= 10) {
    UInt32 i = 0;
    i += get4b((uint32_t *)&param->version, buf, i);
    i += get4b((uint32_t *)&param->dbID, buf, i);
    i += get2b((uint16_t *)&param->cardNo, buf, i);
    r = 0;
  }
  return r;
}

uint8_t *serialize_sysNotifyDisplayChangeEvent(void *p, UInt32 *size) {
  SysNotifyDisplayChangeDetailsType *param = p;
  uint8_t *buf = sys_calloc(1, *size + 8);
  UInt32 i = *size;
  i += put4b(param->oldDepth, buf, i);
  i += put4b(param->newDepth, buf, i);
  *size = i;
  return buf;
}

int deserialize_sysNotifyDisplayChangeEvent(uint8_t *buf, UInt32 size, SysNotifyDisplayChangeDetailsType *param) {
  int r = -1;
  if (size >= 8) {
    UInt32 i = 0;
    i += get4b((uint32_t *)&param->oldDepth, buf, i);
    i += get4b((uint32_t *)&param->newDepth, buf, i);
    r = 0;
  }
  return r;
}

uint8_t *serialize_sysNotifyDBCreatedEvent(void *p, UInt32 *size) {
  SysNotifyDBCreatedType *param = p;
  uint8_t *buf = sys_calloc(1, *size + 16 + dmDBNameLength);
  UInt32 i = *size;
  sys_memcpy(buf + i, param->dbName, dmDBNameLength);
  i += dmDBNameLength;
  i += put4b(param->creator, buf, i);
  i += put4b(param->type, buf, i);
  i += put4b(param->newDBID, buf, i);
  i += put2b(param->cardNo, buf, i);
  i += put1(param->resDB, buf, i);
  i += put1(param->padding, buf, i);
  *size = i;
  return buf;
}

int deserialize_sysNotifyDBCreatedEvent(uint8_t *buf, UInt32 size, SysNotifyDBCreatedType *param) {
  int r = -1;
  if (size >= 16 + dmDBNameLength) {
    UInt32 i = 0;
    sys_memcpy(param->dbName, buf + i, dmDBNameLength);
    i += dmDBNameLength;
    i += get4b((uint32_t *)&param->creator, buf, i);
    i += get4b((uint32_t *)&param->type, buf, i);
    i += get4b((uint32_t *)&param->newDBID, buf, i);
    i += get2b((uint16_t *)&param->cardNo, buf, i);
    i += get1((uint8_t *)&param->resDB, buf, i);
    i += get1((uint8_t *)&param->padding, buf, i);
    r = 0;
  }
  return r;
}

uint8_t *serialize_sysNotifyDBDeletedEvent(void *p, UInt32 *size) {
  SysNotifyDBDeletedType *param = p;
  uint8_t *buf = sys_calloc(1, *size + 16 + dmDBNameLength);
  UInt32 i = *size;
  i += put4b(param->oldDBID, buf, i);
  i += put2b(param->cardNo, buf, i);
  i += put2b(param->attributes, buf, i);
  sys_memcpy(buf + i, param->dbName, dmDBNameLength);
  i += dmDBNameLength;
  i += put4b(param->creator, buf, i);
  i += put4b(param->type, buf, i);
  *size = i;
  return buf;
}

int deserialize_sysNotifyDBDeletedEvent(uint8_t *buf, UInt32 size, SysNotifyDBDeletedType *param) {
  int r = -1;
  if (size >= 16 + dmDBNameLength) {
    UInt32 i = 0;
    i += get4b((uint32_t *)&param->oldDBID, buf, i);
    i += get2b((uint16_t *)&param->cardNo, buf, i);
    i += get2b((uint16_t *)&param->attributes, buf, i);
    sys_memcpy(param->dbName, buf + i, dmDBNameLength);
    i += dmDBNameLength;
    i += get4b((uint32_t *)&param->creator, buf, i);
    i += get4b((uint32_t *)&param->type, buf, i);
    r = 0;
  }
  return r;
}

uint8_t *serialize_sysNotifyTimeChangeEvent(void *p, UInt32 *size) {
  Int32 *param = p;
  uint8_t *buf = sys_calloc(1, *size + 4);
  UInt32 i = *size;
  i += put4b((uint32_t)*param, buf, i);
  *size = i;
  return buf;
}

int deserialize_sysNotifyTimeChangeEvent(uint8_t *buf, UInt32 size, Int32 *param) {
  int r = -1;
  if (size == 4) {
    get4b((uint32_t *)param, buf, 0);
    r = 0;
  }
  return r;
}

uint8_t *serialize_sysNotifyGPSDataEvent(void *p, UInt32 *size) {
  UInt16 *param = p;
  uint8_t *buf = sys_calloc(1, *size + 2);
  UInt32 i = *size;
  i += put2b((uint16_t)*param, buf, i);
  *size = i;
  return buf;
}

int deserialize_sysNotifyGPSDataEvent(uint8_t *buf, UInt32 size, UInt16 *param) {
  int r = -1;
  if (size == 2) {
    get2b((uint16_t *)param, buf, 0);
    r = 0;
  }
  return r;
}

uint8_t *serialize_sysNotifyVolumeMountedEvent(void *p, UInt32 *size) {
  VFSAnyMountParamType *param = p;
  uint8_t *buf = sys_calloc(1, *size + 8);
  UInt32 i = *size;
  i += put2b(param->volRefNum, buf, i);
  i += put2b(param->reserved, buf, i);
  i += put4b(param->mountClass, buf, i);
  *size = i;
  return buf;
}

int deserialize_sysNotifyVolumeMountedEvent(uint8_t *buf, UInt32 size, VFSAnyMountParamType *param) {
  int r = -1;
  if (size >= 8) {
    UInt32 i = 0;
    i += get2b((uint16_t *)&param->volRefNum, buf, i);
    i += get2b((uint16_t *)&param->reserved, buf, i);
    i += get4b((uint32_t *)&param->mountClass, buf, i);
    r = 0;
  }
  return r;
}

uint8_t *serialize_sysNotifyCardInsertedEvent(void *p, UInt32 *size) {
  uint8_t *buf = (uint8_t *)p;
  UInt32 i = 0;
  *size = i;
  return buf;
}

int deserialize_sysNotifyCardInsertedEvent(uint8_t *buf, UInt32 size, UInt16 *param) {
  *param = (UInt16)((uintptr_t)buf);
  return 0;
}
uint8_t *serialize_sysNotifyCardRemovedEvent(void *p, UInt32 *size) {
  uint8_t *buf = (uint8_t *)p;
  UInt32 i = 0;
  *size = i;
  return buf;
}

int deserialize_sysNotifyCardRemovedEvent(uint8_t *buf, UInt32 size, UInt16 *param) {
  *param = (UInt16)((uintptr_t)buf);
  return 0;
}
uint8_t *serialize_sysNotifyVolumeUnmountedEvent(void *p, UInt32 *size) {
  uint8_t *buf = (uint8_t *)p;
  UInt32 i = 0;
  *size = i;
  return buf;
}

int deserialize_sysNotifyVolumeUnmountedEvent(uint8_t *buf, UInt32 size, UInt16 *param) {
  *param = (UInt16)((uintptr_t)buf);
  return 0;
}
uint8_t *serialize_sysNotifyLocaleChangedEvent(void *p, UInt32 *size) {
  SysNotifyLocaleChangedType *param = p;
  uint8_t *buf = sys_calloc(1, *size + 8);
  UInt32 i = *size;
  i += put2b(param->oldLocale.language, buf, i);
  i += put2b(param->oldLocale.country, buf, i);
  i += put2b(param->newLocale.language, buf, i);
  i += put2b(param->newLocale.country, buf, i);
  *size = i;
  return buf;
}

int deserialize_sysNotifyLocaleChangedEvent(uint8_t *buf, UInt32 size, SysNotifyLocaleChangedType *param) {
  int r = -1;
  if (size >= 8) {
    UInt32 i = 0;
    i += get2b((uint16_t *)&param->oldLocale.language, buf, i);
    i += get2b((uint16_t *)&param->oldLocale.country, buf, i);
    i += get2b((uint16_t *)&param->newLocale.language, buf, i);
    i += get2b((uint16_t *)&param->newLocale.country, buf, i);
    r = 0;
  }
  return r;
}

uint8_t *serialize_sysNotifySelectDay(void *p, UInt32 *size) {
  SysNotifySelectDayDetailsType *param = p;
  uint8_t *buf = sys_calloc(1, *size + 34 + 2 + sys_strlen(param->titleP) + 2);
  UInt16 len;
  UInt32 i = *size;
  i += put2b(param->daySelectorP->bounds.topLeft.x, buf, i);
  i += put2b(param->daySelectorP->bounds.topLeft.y, buf, i);
  i += put2b(param->daySelectorP->bounds.extent.x, buf, i);
  i += put2b(param->daySelectorP->bounds.extent.x, buf, i);
  i += put1(param->daySelectorP->visible, buf, i);
  i += put1(param->daySelectorP->reserved1, buf, i);
  i += put2b(param->daySelectorP->visibleMonth, buf, i);
  i += put2b(param->daySelectorP->visibleYear, buf, i);
  i += put2b(param->daySelectorP->selected.second, buf, i);
  i += put2b(param->daySelectorP->selected.minute, buf, i);
  i += put2b(param->daySelectorP->selected.hour, buf, i);
  i += put2b(param->daySelectorP->selected.day, buf, i);
  i += put2b(param->daySelectorP->selected.month, buf, i);
  i += put2b(param->daySelectorP->selected.year, buf, i);
  i += put2b(param->daySelectorP->selected.weekDay, buf, i);
  i += put1(param->daySelectorP->selectDayBy, buf, i);
  i += put1(param->daySelectorP->reserved2, buf, i);
  len = (int)(sys_strlen(param->titleP) + 1);
  i += put2b(len, buf, i);
  sys_memcpy(buf + i, param->titleP, len);
  i += len;
  if (len % 2) i += put1(0, buf, i);
  i += put1(param->dayChanged, buf, i);
  i += put1(param->padding[0], buf, i);
  i += put1(param->padding[1], buf, i);
  i += put1(param->padding[2], buf, i);
  *size = i;
  return buf;
}

int deserialize_sysNotifySelectDay(uint8_t *buf, UInt32 size, SysNotifySelectDayDetailsType *param) {
  int r = -1;
  if (size >= 34) {
    UInt32 i = 0;
    UInt16 len;
    i += get2b((uint16_t *)&param->daySelectorP->bounds.topLeft.x, buf, i);
    i += get2b((uint16_t *)&param->daySelectorP->bounds.topLeft.y, buf, i);
    i += get2b((uint16_t *)&param->daySelectorP->bounds.extent.x, buf, i);
    i += get2b((uint16_t *)&param->daySelectorP->bounds.extent.x, buf, i);
    i += get1((uint8_t *)&param->daySelectorP->visible, buf, i);
    i += get1((uint8_t *)&param->daySelectorP->reserved1, buf, i);
    i += get2b((uint16_t *)&param->daySelectorP->visibleMonth, buf, i);
    i += get2b((uint16_t *)&param->daySelectorP->visibleYear, buf, i);
    i += get2b((uint16_t *)&param->daySelectorP->selected.second, buf, i);
    i += get2b((uint16_t *)&param->daySelectorP->selected.minute, buf, i);
    i += get2b((uint16_t *)&param->daySelectorP->selected.hour, buf, i);
    i += get2b((uint16_t *)&param->daySelectorP->selected.day, buf, i);
    i += get2b((uint16_t *)&param->daySelectorP->selected.month, buf, i);
    i += get2b((uint16_t *)&param->daySelectorP->selected.year, buf, i);
    i += get2b((uint16_t *)&param->daySelectorP->selected.weekDay, buf, i);
    i += get1((uint8_t *)&param->daySelectorP->selectDayBy, buf, i);
    i += get1((uint8_t *)&param->daySelectorP->reserved2, buf, i);
    i += get2b(&len, buf, i);
    param->titleP = (char *)(buf + i);
    if (len % 2) i++;
    i += len;
    i += get1((uint8_t *)&param->dayChanged, buf, i);
    i += get1((uint8_t *)&param->padding[0], buf, i);
    i += get1((uint8_t *)&param->padding[1], buf, i);
    i += get1((uint8_t *)&param->padding[2], buf, i);
    r = 0;
  }
  return r;
}

uint8_t *serialize_sysNotifySyncFinishEvent(void *p, UInt32 *size) {
  uint8_t *buf = sys_calloc(1, *size);
  UInt32 i = *size;
  *size = i;
  return buf;
}

int deserialize_sysNotifySyncFinishEvent(uint8_t *buf, UInt32 size, void *param) {
  return 0;
}

uint8_t *serialize_notif(UInt32 paramType, void *param, UInt32 *size) {
  switch (paramType) {
    case sysNotifyAppLaunchingEvent:
      return serialize_sysNotifyAppLaunchingEvent(param, size);
    case sysNotifyAppQuittingEvent:
      return serialize_sysNotifyAppQuittingEvent(param, size);
    case sysNotifyAppCrashedEvent:
      return serialize_sysNotifyAppCrashedEvent(param, size);
    case sysNotifyDisplayChangeEvent:
      return serialize_sysNotifyDisplayChangeEvent(param, size);
    case sysNotifyDBCreatedEvent:
      return serialize_sysNotifyDBCreatedEvent(param, size);
    case sysNotifyDBDeletedEvent:
      return serialize_sysNotifyDBDeletedEvent(param, size);
    case sysNotifyTimeChangeEvent:
      return serialize_sysNotifyTimeChangeEvent(param, size);
    case sysNotifyGPSDataEvent:
      return serialize_sysNotifyGPSDataEvent(param, size);
    case sysNotifyVolumeMountedEvent:
      return serialize_sysNotifyVolumeMountedEvent(param, size);
    case sysNotifyCardInsertedEvent:
      return serialize_sysNotifyCardInsertedEvent(param, size);
    case sysNotifyCardRemovedEvent:
      return serialize_sysNotifyCardRemovedEvent(param, size);
    case sysNotifyVolumeUnmountedEvent:
      return serialize_sysNotifyVolumeUnmountedEvent(param, size);
    case sysNotifyLocaleChangedEvent:
      return serialize_sysNotifyLocaleChangedEvent(param, size);
    case sysNotifySelectDay:
      return serialize_sysNotifySelectDay(param, size);
    case sysNotifySyncFinishEvent:
      return serialize_sysNotifySyncFinishEvent(param, size);
    default:
      break;
  }
  return NULL;
}

int deserialize_notif(UInt32 paramType, void *buf, UInt32 size, notif_union_t *param) {
  switch (paramType) {
    case sysNotifyAppLaunchingEvent:
      return deserialize_sysNotifyAppLaunchingEvent(buf, size, &param->p0);
    case sysNotifyAppQuittingEvent:
      return deserialize_sysNotifyAppQuittingEvent(buf, size, &param->p1);
    case sysNotifyAppCrashedEvent:
      return deserialize_sysNotifyAppCrashedEvent(buf, size, &param->p2);
    case sysNotifyDisplayChangeEvent:
      return deserialize_sysNotifyDisplayChangeEvent(buf, size, &param->p3);
    case sysNotifyDBCreatedEvent:
      return deserialize_sysNotifyDBCreatedEvent(buf, size, &param->p4);
    case sysNotifyDBDeletedEvent:
      return deserialize_sysNotifyDBDeletedEvent(buf, size, &param->p5);
    case sysNotifyTimeChangeEvent:
      return deserialize_sysNotifyTimeChangeEvent(buf, size, &param->p6);
    case sysNotifyGPSDataEvent:
      return deserialize_sysNotifyGPSDataEvent(buf, size, &param->p7);
    case sysNotifyVolumeMountedEvent:
      return deserialize_sysNotifyVolumeMountedEvent(buf, size, &param->p8);
    case sysNotifyCardInsertedEvent:
      return deserialize_sysNotifyCardInsertedEvent(buf, size, &param->p9);
    case sysNotifyCardRemovedEvent:
      return deserialize_sysNotifyCardRemovedEvent(buf, size, &param->p10);
    case sysNotifyVolumeUnmountedEvent:
      return deserialize_sysNotifyVolumeUnmountedEvent(buf, size, &param->p11);
    case sysNotifyLocaleChangedEvent:
      return deserialize_sysNotifyLocaleChangedEvent(buf, size, &param->p12);
    case sysNotifySelectDay:
      return deserialize_sysNotifySelectDay(buf, size, &param->p13);
    case sysNotifySyncFinishEvent:
      return deserialize_sysNotifySyncFinishEvent(buf, size, &param->p14);
    default:
      break;
  }
  return -1;
}
