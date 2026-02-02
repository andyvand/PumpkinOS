/* This file was generated from "notifications.struct". Do not edit. */
#ifndef notif_serde_h
#define notif_serde_h
uint8_t *serialize_sysNotifyAppLaunchingEvent(void *p, UInt32 *size);
int deserialize_sysNotifyAppLaunchingEvent(uint8_t *buf, UInt32 size, SysNotifyAppLaunchOrQuitType *param);
uint8_t *serialize_sysNotifyAppQuittingEvent(void *p, UInt32 *size);
int deserialize_sysNotifyAppQuittingEvent(uint8_t *buf, UInt32 size, SysNotifyAppLaunchOrQuitType *param);
uint8_t *serialize_sysNotifyAppCrashedEvent(void *p, UInt32 *size);
int deserialize_sysNotifyAppCrashedEvent(uint8_t *buf, UInt32 size, SysNotifyAppLaunchOrQuitType *param);
uint8_t *serialize_sysNotifyDisplayChangeEvent(void *p, UInt32 *size);
int deserialize_sysNotifyDisplayChangeEvent(uint8_t *buf, UInt32 size, SysNotifyDisplayChangeDetailsType *param);
uint8_t *serialize_sysNotifyDBCreatedEvent(void *p, UInt32 *size);
int deserialize_sysNotifyDBCreatedEvent(uint8_t *buf, UInt32 size, SysNotifyDBCreatedType *param);
uint8_t *serialize_sysNotifyDBDeletedEvent(void *p, UInt32 *size);
int deserialize_sysNotifyDBDeletedEvent(uint8_t *buf, UInt32 size, SysNotifyDBDeletedType *param);
uint8_t *serialize_sysNotifyTimeChangeEvent(void *p, UInt32 *size);
int deserialize_sysNotifyTimeChangeEvent(uint8_t *buf, UInt32 size, Int32 *param);
uint8_t *serialize_sysNotifyGPSDataEvent(void *p, UInt32 *size);
int deserialize_sysNotifyGPSDataEvent(uint8_t *buf, UInt32 size, UInt16 *param);
uint8_t *serialize_sysNotifyVolumeMountedEvent(void *p, UInt32 *size);
int deserialize_sysNotifyVolumeMountedEvent(uint8_t *buf, UInt32 size, VFSAnyMountParamType *param);
uint8_t *serialize_sysNotifyCardInsertedEvent(void *p, UInt32 *size);
int deserialize_sysNotifyCardInsertedEvent(uint8_t *buf, UInt32 size, UInt16 *param);
uint8_t *serialize_sysNotifyCardRemovedEvent(void *p, UInt32 *size);
int deserialize_sysNotifyCardRemovedEvent(uint8_t *buf, UInt32 size, UInt16 *param);
uint8_t *serialize_sysNotifyVolumeUnmountedEvent(void *p, UInt32 *size);
int deserialize_sysNotifyVolumeUnmountedEvent(uint8_t *buf, UInt32 size, UInt16 *param);
uint8_t *serialize_sysNotifyLocaleChangedEvent(void *p, UInt32 *size);
int deserialize_sysNotifyLocaleChangedEvent(uint8_t *buf, UInt32 size, SysNotifyLocaleChangedType *param);
uint8_t *serialize_sysNotifySelectDay(void *p, UInt32 *size);
int deserialize_sysNotifySelectDay(uint8_t *buf, UInt32 size, SysNotifySelectDayDetailsType *param);
uint8_t *serialize_sysNotifySyncFinishEvent(void *p, UInt32 *size);
int deserialize_sysNotifySyncFinishEvent(uint8_t *buf, UInt32 size, void *param);

typedef union {
  SysNotifyAppLaunchOrQuitType p0;
  SysNotifyAppLaunchOrQuitType p1;
  SysNotifyAppLaunchOrQuitType p2;
  SysNotifyDisplayChangeDetailsType p3;
  SysNotifyDBCreatedType p4;
  SysNotifyDBDeletedType p5;
  Int32 p6;
  UInt16 p7;
  VFSAnyMountParamType p8;
  UInt16 p9;
  UInt16 p10;
  UInt16 p11;
  SysNotifyLocaleChangedType p12;
  SysNotifySelectDayDetailsType p13;
  uint32_t p14;
  uint8_t buf[64];
} notif_union_t;

uint8_t *serialize_notif(UInt32 paramType, void *param, UInt32 *size);
int deserialize_notif(UInt32 paramType, void *buf, UInt32 size, notif_union_t *param);
#endif
