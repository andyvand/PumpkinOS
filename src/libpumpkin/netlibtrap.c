#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "pumpkin.h"
#include "logtrap.h"
#include "m68k.h"
#include "m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_netlibtrap(uint16_t trap) {
  UInt16 i = 0;
  uint32_t sp = 0;
  uint16_t idx = 0;
  char buf[256];
  Err err = 0;
  Err res = 0;
  uint16_t refNum = 0;
  uint32_t netIFErrsP = 0;
  UInt16 netIFErrs = 0;
  uint16_t immediate = 0;
  uint32_t countP = 0;
  UInt16 l_countP = 0;
  uint32_t eventP = 0;
  SysEventType l_eventP;
  uint8_t refresh = 0;
  uint32_t allInterfacesUpP = 0;
  UInt8 l_allInterfacesUpP = 0;
  uint32_t netIFErrP = 0;
  UInt16 l_netIFErrP = 0;
  uint32_t inet = 0;
  uint32_t spaceP = 0;
  char *s_spaceP = NULL;
  Char *resc = NULL;
  uint32_t r_res = 0;
  uint16 libRefnum = 0;
  uint32 a = 0;
  NetIPAddr resip = 0;
  char *s_a = NULL;
  uint8_t domain = 0;
  uint16_t type = 0;
  int16_t protocol = 0;
  int32_t timeout = 0;
  uint32_t errP = 0;
  Err l_errP = 0;
  int16_t socket = 0;
  uint32 sockAddrP = 0;
  NetSocketAddrType l_sockAddrP;
  int16_t addrLen = 0;
  NetSocketRef ressock = 0;
  uint16_t queueLen = 0;
  uint32_t addrLenP = 0;
  Int16 l_addrLenP = 0;
  int16_t direction = 0;
  uint32_t recordP = 0;
  char *s_recordP = NULL;
  uint32_t recordOffset = 0;
  uint16_t rcvLen = 0;
  uint16_t flags = 0;
  uint32_t fromAddrP = 0;
  char *s_fromAddrP = NULL;
  uint32_t fromLenP = 0;
  UInt16 l_fromLenP = 0;
  uint16_t index = 0;
  uint32_t ifCreatorP = 0;
  UInt32 l_ifCreatorP = 0;
  uint32_t ifInstanceP = 0;
  UInt16 l_ifInstanceP = 0;
  uint32_t ifCreator = 0;
  uint16_t ifInstance = 0;
  uint16_t width = 0;
  uint32_t readFDs = 0;
  NetFDSetType l_readFDs;
  uint32_t writeFDs = 0;
  NetFDSetType l_writeFDs;
  uint32_t exceptFDs = 0;
  NetFDSetType l_exceptFDs;
  uint32_t strP = 0;
  char *s_strP = NULL;
  uint16_t configIndex = 0;
  uint32_t openFlags = 0;
  uint32_t nameP = 0;
  NetConfigNameType l_nameP;
  uint32_t indexP = 0;
  UInt16 l_indexP = 0;
  uint32_t newNameP = 0;
  NetConfigNameType l_newNameP;
  uint16_t aliasToIndex = 0;
  uint16_t aliasIndex = 0;
  uint32_t isAnotherAliasP = 0;
  Boolean l_isAnotherAliasP = 0;
  uint32_t nameArrayP = 0;
  NetConfigNameType nameArray[16];
  UInt16 arrayEntries = 0;
  char *s_nameP = NULL;
  NetHostInfoBufType l_bufP;
  NetHostInfoType *resnh = NULL;
  UInt32 l_addrP = 0;
  uint16_t len = 0;
  uint32_t arrayEntriesP = 0;
  char *s_bufP = NULL;
  uint16_t bufLen = 0;
  uint32_t toAddrP = 0;
  NetSocketAddrType toAddr;
  uint16_t toLen = 0;
  uint32_t bufP = 0;
  uint32_t addrP = 0;
  NetSocketAddrType fromAddr;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;

  switch (trap) {
    case sysLibTrapOpen: {
      refNum = ARG16;
      netIFErrsP = ARG32;
      err = NetLibOpen(refNum, netIFErrsP ? &netIFErrs : NULL);
      if (netIFErrsP) m68k_write_memory_16(netIFErrsP, netIFErrs);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibOpen(refNum=%d, netIFErrsP=0x%08X): %d", refNum, netIFErrsP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapClose: {
      refNum = ARG16;
      immediate = ARG16;
      err = NetLibClose(refNum, immediate);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibClose(refNum=%d, immediate=%d): %d", refNum, immediate, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapSleep: {
      refNum = ARG16;
      err = NetLibSleep(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSleep(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapWake: {
      refNum = ARG16;
      err = NetLibWake(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibWake(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case netLibTrapFinishCloseWait: {
      // Err NetLibFinishCloseWait(UInt16 libRefnum);
      libRefnum = ARG16;
      res = NetLibFinishCloseWait(libRefnum);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibFinishCloseWait(libRefnum=%d): %d", libRefnum, res);
    }
    break;
    case netLibTrapOpenIfCloseWait: {
      // Err NetLibOpenIfCloseWait(UInt16 libRefnum);
      libRefnum = ARG16;
      res = NetLibOpenIfCloseWait(libRefnum);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibOpenIfCloseWait(libRefnum=%d): %d", libRefnum, res);
    }
    break;
    case netLibTrapOpenCount: {
      // Err NetLibOpenCount(UInt16 refNum, out UInt16 *countP);
      refNum = ARG16;
      countP = ARG32;
      res = NetLibOpenCount(refNum, countP ? &l_countP : NULL);
      if (countP) m68k_write_memory_16(countP, l_countP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibOpenCount(refNum=%d, countP=0x%08X [%d]): %d", refNum, countP, l_countP, res);
    }
    break;
    case netLibTrapHandlePowerOff: {
      // Err NetLibHandlePowerOff(UInt16 refNum, in SysEventType *eventP);
      refNum = ARG16;
      eventP = ARG32;
	  res = NetLibHandlePowerOff((uint16_t)refNum, eventP ? &l_eventP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibHandlePowerOff(refNum=%d, eventP=0x%08X): %d", refNum, eventP, res);
    }
    break;
    case netLibTrapConnectionRefresh: {
      // Err NetLibConnectionRefresh(UInt16 refNum, Boolean refresh, out UInt8 *allInterfacesUpP, out UInt16 *netIFErrP);
      refNum = ARG16;
      refresh = ARG8;
      allInterfacesUpP = ARG32;
      netIFErrP = ARG32;
      res = NetLibConnectionRefresh(refNum, refresh, allInterfacesUpP ? &l_allInterfacesUpP : NULL, netIFErrP ? &l_netIFErrP : NULL);
      if (allInterfacesUpP) m68k_write_memory_8(allInterfacesUpP, l_allInterfacesUpP);
      if (netIFErrP) m68k_write_memory_16(netIFErrP, l_netIFErrP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConnectionRefresh(refNum=%d, refresh=%d, allInterfacesUpP=0x%08X, netIFErrP=0x%08X [%d]): %d", refNum, refresh, allInterfacesUpP, netIFErrP, l_netIFErrP, res);
    }
    break;
    case netLibTrapAddrINToA: {
      // Char *NetLibAddrINToA(UInt16 libRefnum, NetIPAddr inet, out Char *spaceP);
      libRefnum = ARG16;
      inet = ARG32;
      spaceP = ARG32;
      s_spaceP = emupalmos_trap_in(spaceP, trap, 2);
      resc = NetLibAddrINToA(libRefnum, inet, spaceP ? s_spaceP : NULL);
      r_res = emupalmos_trap_out(resc);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibAddrINToA(libRefnum=%d, inet=%d, spaceP=0x%08X [%s]): 0x%08X", libRefnum, inet, spaceP, s_spaceP, r_res);
    }
    break;
    case netLibTrapAddrAToIN: {
      // NetIPAddr NetLibAddrAToIN(UInt16 libRefnum, in Char *a);
      libRefnum = ARG16;
      a = ARG32;
	  s_a = emupalmos_trap_in(a, trap, 1);
      resip = NetLibAddrAToIN(libRefnum, a ? s_a : NULL);
      m68k_set_reg(M68K_REG_D0, resip);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibAddrAToIN(libRefnum=%d, a=0x%08X [%s]): %d", libRefnum, a, s_a, resip);
    }
    break;
    case netLibTrapSocketOpen: {
      // NetSocketRef NetLibSocketOpen(UInt16 libRefnum, NetSocketAddrEnum domain, NetSocketTypeEnum type, Int16 protocol, Int32 timeout, out Err *errP);
      libRefnum = ARG16;
      domain = ARG8;
      type = ARG8;
      protocol = ARG16;
      timeout = ARG32;
      errP = ARG32;
      ressock = NetLibSocketOpen(libRefnum, domain, (uint8_t)type, protocol, timeout, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, ressock);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSocketOpen(libRefnum=%d, domain=%d, type=%d, protocol=%d, timeout=%d, errP=0x%08X): %d", libRefnum, domain, type, protocol, timeout, errP, ressock);
    }
    break;
    case netLibTrapSocketClose: {
      // Int16 NetLibSocketClose(UInt16 libRefnum, NetSocketRef socket, Int32 timeout, out Err *errP);
      libRefnum = ARG16;
      socket = ARG16;
      timeout = ARG32;
      errP = ARG32;
      res = NetLibSocketClose(libRefnum, socket, timeout, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
	  m68k_set_reg(M68K_REG_D0, (Int16)res);
	  debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSocketClose(libRefnum=%d, socket=%d, timeout=%d, errP=0x%08X): %d", libRefnum, socket, timeout, errP, (Int16)res);
    }
    break;
    case netLibTrapSocketBind: {
      // Int16 NetLibSocketBind(UInt16 libRefnum, NetSocketRef socket, in NetSocketAddrType *sockAddrP, Int16 addrLen, Int32 timeout, out Err *errP);
      libRefnum = ARG16;
      socket = ARG16;
      sockAddrP = ARG32;

      decode_NetSocketAddrType(sockAddrP, &l_sockAddrP);
      addrLen = ARG16;
      timeout = ARG32;
      errP = ARG32;
      res = NetLibSocketBind(libRefnum, socket, sockAddrP ? &l_sockAddrP : NULL, addrLen, timeout, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
	  m68k_set_reg(M68K_REG_D0, (Int16)res);
	  debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSocketBind(libRefnum=%d, socket=%d, sockAddrP=0x%08X, addrLen=%d, timeout=%d, errP=0x%08X): %d", libRefnum, socket, sockAddrP, addrLen, timeout, errP, (Int16)res);
    }
    break;
    case netLibTrapSocketConnect: {
      // Int16 NetLibSocketConnect(UInt16 libRefnum, NetSocketRef socket, in NetSocketAddrType *sockAddrP, Int16 addrLen, Int32 timeout, out Err *errP);
      libRefnum = ARG16;
      socket = ARG16;
      sockAddrP = ARG32;

      decode_NetSocketAddrType(sockAddrP, &l_sockAddrP);
      addrLen = ARG16;
      timeout = ARG32;
      errP = ARG32;
      res = NetLibSocketConnect(libRefnum, socket, sockAddrP ? &l_sockAddrP : NULL, addrLen, timeout, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
	  m68k_set_reg(M68K_REG_D0, (Int16)res);
	  debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSocketConnect(libRefnum=%d, socket=%d, sockAddrP=0x%08X, addrLen=%d, timeout=%d, errP=0x%08X): %d", libRefnum, socket, sockAddrP, addrLen, timeout, errP, (Int16)res);
    }
    break;
    case netLibTrapSocketListen: {
      // Int16 NetLibSocketListen(UInt16 libRefnum, NetSocketRef socket, UInt16 queueLen, Int32 timeout, out Err *errP);
      libRefnum = ARG16;
      socket = ARG16;
      queueLen = ARG16;
      timeout = ARG32;
      errP = ARG32;
      res = NetLibSocketListen(libRefnum, socket, queueLen, timeout, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSocketListen(libRefnum=%d, socket=%d, queueLen=%d, timeout=%d, errP=0x%08X): %d", libRefnum, socket, queueLen, timeout, errP, res);
    }
    break;
    case netLibTrapSocketAccept: {
      // Int16 NetLibSocketAccept(UInt16 libRefnum, NetSocketRef socket, out NetSocketAddrType *sockAddrP, inout Int16 *addrLenP, Int32 timeout, out Err *errP);
      libRefnum = ARG16;
      socket = ARG16;
      sockAddrP = ARG32;

      addrLenP = ARG32;

      if (addrLenP) l_addrLenP = m68k_read_memory_16(addrLenP);
      timeout = ARG32;
      errP = ARG32;

      res = NetLibSocketAccept(libRefnum, socket, sockAddrP ? &l_sockAddrP : NULL, addrLenP ? &l_addrLenP : NULL, timeout, errP ? &l_errP : NULL);
      encode_NetSocketAddrType(sockAddrP, &l_sockAddrP);
      if (addrLenP) m68k_write_memory_16(addrLenP, l_addrLenP);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSocketAccept(libRefnum=%d, socket=%d, sockAddrP=0x%08X, addrLenP=0x%08X [%d], timeout=%d, errP=0x%08X): %d", libRefnum, socket, sockAddrP, addrLenP, l_addrLenP, timeout, errP, res);
    }
    break;
    case netLibTrapSocketShutdown: {
      // Int16 NetLibSocketShutdown(UInt16 libRefnum, NetSocketRef socket, Int16 direction, Int32 timeout, out Err *errP);
      libRefnum = ARG16;
      socket = ARG16;
      direction = ARG16;
      timeout = ARG32;
      errP = ARG32;

      res = NetLibSocketShutdown(libRefnum, socket, direction, timeout, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSocketShutdown(libRefnum=%d, socket=%d, direction=%d, timeout=%d, errP=0x%08X): %d", libRefnum, socket, direction, timeout, errP, res);
    }
    break;
    case netLibTrapDmReceive: {
      // Int16 NetLibDmReceive(UInt16 libRefnum, NetSocketRef socket, out void *recordP, UInt32 recordOffset, UInt16 rcvLen, UInt16 flags, in void *fromAddrP, inout UInt16 *fromLenP, Int32 timeout, out Err *errP);
      libRefnum = ARG16;
      socket = ARG16;
      recordP = ARG32;
      s_recordP = emupalmos_trap_in(recordP, trap, 2);
      recordOffset = ARG32;
      rcvLen = ARG16;
      flags = ARG16;
      fromAddrP = ARG32;
      s_fromAddrP = emupalmos_trap_in(fromAddrP, trap, 6);
      fromLenP = ARG32;

      if (fromLenP) l_fromLenP = m68k_read_memory_16(fromLenP);
      timeout = ARG32;
      errP = ARG32;

      res = NetLibDmReceive(libRefnum, socket, recordP ? s_recordP : NULL, recordOffset, rcvLen, flags, fromAddrP ? s_fromAddrP : NULL, fromLenP ? &l_fromLenP : NULL, timeout, errP ? &l_errP : NULL);
      if (fromLenP) m68k_write_memory_16(fromLenP, l_fromLenP);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibDmReceive(libRefnum=%d, socket=%d, recordP=0x%08X, recordOffset=%d, rcvLen=%d, flags=%d, fromAddrP=0x%08X, fromLenP=0x%08X [%d], timeout=%d, errP=0x%08X): %d", libRefnum, socket, recordP, recordOffset, rcvLen, flags, fromAddrP, fromLenP, l_fromLenP, timeout, errP, res);
    }
    break;
    case netLibTrapIFGet: {
      // Err NetLibIFGet(UInt16 libRefnum, UInt16 index, out UInt32 *ifCreatorP, out UInt16 *ifInstanceP);
      libRefnum = ARG16;
      index = ARG16;
      ifCreatorP = ARG32;
      ifInstanceP = ARG32;
      res = NetLibIFGet(libRefnum, index, ifCreatorP ? &l_ifCreatorP : NULL, ifInstanceP ? &l_ifInstanceP : NULL);
      if (ifCreatorP) m68k_write_memory_32(ifCreatorP, l_ifCreatorP);
      if (ifInstanceP) m68k_write_memory_16(ifInstanceP, l_ifInstanceP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibIFGet(libRefnum=%d, index=%d, ifCreatorP=0x%08X [%d], ifInstanceP=0x%08X [%d]): %d", libRefnum, index, ifCreatorP, l_ifCreatorP, ifInstanceP, l_ifInstanceP, res);
    }
    break;
    case netLibTrapIFAttach: {
      // Err NetLibIFAttach(UInt16 libRefnum, UInt32 ifCreator, UInt16 ifInstance, Int32 timeout);
      libRefnum = ARG16;
      ifCreator = ARG32;
      ifInstance = ARG16;
      timeout = ARG32;
      res = NetLibIFAttach(libRefnum, ifCreator, ifInstance, timeout);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibIFAttach(libRefnum=%d, ifCreator=%d, ifInstance=%d, timeout=%d): %d", libRefnum, ifCreator, ifInstance, timeout, res);
    }
    break;
    case netLibTrapIFDetach: {
      // Err NetLibIFDetach(UInt16 libRefnum, UInt32 ifCreator, UInt16 ifInstance, Int32 timeout);
      libRefnum = ARG16;
      ifCreator = ARG32;
      ifInstance = ARG16;
      timeout = ARG32;
      res = NetLibIFDetach(libRefnum, ifCreator, ifInstance, timeout);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibIFDetach(libRefnum=%d, ifCreator=%d, ifInstance=%d, timeout=%d): %d", libRefnum, ifCreator, ifInstance, timeout, res);
    }
    break;
    case netLibTrapIFUp: {
      // Err NetLibIFUp(UInt16 libRefnum, UInt32 ifCreator, UInt16 ifInstance);
      libRefnum = ARG16;
      ifCreator = ARG32;
      ifInstance = ARG16;
      res = NetLibIFUp(libRefnum, ifCreator, ifInstance);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibIFUp(libRefnum=%d, ifCreator=%d, ifInstance=%d): %d", libRefnum, ifCreator, ifInstance, res);
    }
    break;
    case netLibTrapIFDown: {
      // Err NetLibIFDown(UInt16 libRefnum, UInt32 ifCreator, UInt16 ifInstance, Int32 timeout);
      libRefnum = ARG16;
      ifCreator = ARG32;
      ifInstance = ARG16;
      timeout = ARG32;
      res = NetLibIFDown(libRefnum, ifCreator, ifInstance, timeout);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibIFDown(libRefnum=%d, ifCreator=%d, ifInstance=%d, timeout=%d): %d", libRefnum, ifCreator, ifInstance, timeout, res);
    }
    break;
    case netLibTrapSelect: {
      // Int16 NetLibSelect(UInt16 libRefnum, UInt16 width, in NetFDSetType *readFDs, in NetFDSetType *writeFDs, in NetFDSetType *exceptFDs, Int32 timeout, out Err *errP);
      libRefnum = ARG16;
      width = ARG16;
      readFDs = ARG32;
      MemSet(&l_readFDs, sizeof(NetFDSetType), 0);
      if (readFDs) l_readFDs.mask[0] = m68k_read_memory_32(readFDs);
      writeFDs = ARG32;
      MemSet(&l_writeFDs, sizeof(NetFDSetType), 0);
      if (writeFDs) l_writeFDs.mask[0] = m68k_read_memory_32(writeFDs);
      exceptFDs = ARG32;
      MemSet(&l_exceptFDs, sizeof(NetFDSetType), 0);
      if (exceptFDs) l_exceptFDs.mask[0] = m68k_read_memory_32(exceptFDs);
      timeout = ARG32;
      errP = ARG32;

      res = NetLibSelect(libRefnum, width, readFDs ? &l_readFDs : NULL, writeFDs ? &l_writeFDs : NULL, exceptFDs ? &l_exceptFDs : NULL, timeout, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSelect(libRefnum=%d, width=%d, readFDs=0x%08X, writeFDs=0x%08X, exceptFDs=0x%08X, timeout=%d, errP=0x%08X): %d", libRefnum, width, readFDs, writeFDs, exceptFDs, timeout, errP, res);
    }
    break;
    case netLibTrapTracePutS: {
      // Err NetLibTracePutS(UInt16 libRefnum, in Char *strP);
      libRefnum = ARG16;
      strP = ARG32;
      s_strP = emupalmos_trap_in(strP, trap, 1);
      res = NetLibTracePutS(libRefnum, strP ? s_strP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibTracePutS(libRefnum=%d, strP=0x%08X [%s]): %d", libRefnum, strP, s_strP, res);
    }
    break;
    case netLibTrapOpenConfig: {
      // Err NetLibOpenConfig(UInt16 refNum, UInt16 configIndex, UInt32 openFlags, out UInt16 *netIFErrP);
      refNum = ARG16;
      configIndex = ARG16;
      openFlags = ARG32;
      netIFErrP = ARG32;
	  res = NetLibOpenConfig((uint16_t)refNum, configIndex, openFlags, netIFErrP ? &l_netIFErrP : NULL);
      if (netIFErrP) m68k_write_memory_16(netIFErrP, l_netIFErrP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibOpenConfig(refNum=%d, configIndex=%d, openFlags=%d, netIFErrP=0x%08X [%d]): %d", refNum, configIndex, openFlags, netIFErrP, l_netIFErrP, res);
    }
    break;
    case netLibTrapConfigMakeActive: {
      // Err NetLibConfigMakeActive(UInt16 refNum, UInt16 configIndex);
      refNum = ARG16;
      configIndex = ARG16;
	  res = NetLibConfigMakeActive((uint16_t)refNum, configIndex);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigMakeActive(refNum=%d, configIndex=%d): %d", refNum, configIndex, res);
    }
    break;
    case netLibTrapConfigIndexFromName: {
      // Err NetLibConfigIndexFromName(UInt16 refNum, in NetConfigNameType *nameP, out UInt16 *indexP);
      refNum = ARG16;
      nameP = ARG32;
      decode_NetConfigNameType(nameP, &l_nameP);
      indexP = ARG32;
	  res = NetLibConfigIndexFromName((uint16_t)refNum, nameP ? &l_nameP : NULL, indexP ? &l_indexP : NULL);
      if (indexP) m68k_write_memory_16(indexP, l_indexP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigIndexFromName(refNum=%d, nameP=0x%08X, indexP=0x%08X [%d]): %d", refNum, nameP, indexP, l_indexP, res);
    }
    break;
    case netLibTrapConfigDelete: {
      // Err NetLibConfigDelete(UInt16 refNum, UInt16 index);
      refNum = ARG16;
      index = ARG16;
	  res = NetLibConfigDelete((uint16_t)refNum, index);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigDelete(refNum=%d, index=%d): %d", refNum, index, res);
    }
    break;
    case netLibTrapConfigSaveAs: {
      // Err NetLibConfigSaveAs(UInt16 refNum, in NetConfigNameType *nameP);
      refNum = ARG16;
      nameP = ARG32;
      decode_NetConfigNameType(nameP, &l_nameP);
	  res = NetLibConfigSaveAs((uint16_t)refNum, nameP ? &l_nameP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigSaveAs(refNum=%d, nameP=0x%08X): %d", refNum, nameP, res);
    }
    break;
    case netLibTrapConfigRename: {
      // Err NetLibConfigRename(UInt16 refNum, UInt16 index, in NetConfigNameType *newNameP);
      refNum = ARG16;
      index = ARG16;
      newNameP = ARG32;
      decode_NetConfigNameType(newNameP, &l_newNameP);
	  res = NetLibConfigRename((uint16_t )refNum, index, newNameP ? &l_newNameP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigRename(refNum=%d, index=%d, newNameP=0x%08X): %d", refNum, index, newNameP, res);
    }
    break;
    case netLibTrapConfigAliasSet: {
      // Err NetLibConfigAliasSet(UInt16 refNum, UInt16 configIndex, UInt16 aliasToIndex);
      refNum = ARG16;
      configIndex = ARG16;
      aliasToIndex = ARG16;
	  res = NetLibConfigAliasSet((uint16_t)refNum, configIndex, aliasToIndex);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigAliasSet(refNum=%d, configIndex=%d, aliasToIndex=%d): %d", refNum, configIndex, aliasToIndex, res);
    }
    break;
    case netLibTrapConfigAliasGet: {
      // Err NetLibConfigAliasGet(UInt16 refNum, UInt16 aliasIndex, out UInt16 *indexP, out Boolean *isAnotherAliasP);
      refNum = ARG16;
      aliasIndex = ARG16;
      indexP = ARG32;
      isAnotherAliasP = ARG32;
	  res = NetLibConfigAliasGet((uint16_t)refNum, aliasIndex, indexP ? &l_indexP : NULL, isAnotherAliasP ? &l_isAnotherAliasP : NULL);
      if (indexP) m68k_write_memory_16(indexP, l_indexP);
      if (isAnotherAliasP) m68k_write_memory_8(isAnotherAliasP, l_isAnotherAliasP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigAliasGet(refNum=%d, aliasIndex=%d, indexP=0x%08X [%d], isAnotherAliasP=0x%08X): %d", refNum, aliasIndex, indexP, l_indexP, isAnotherAliasP, res);
    }
    break;
    case netLibTrapConfigList: {
      // Err NetLibConfigList(UInt16 refNum, out NetConfigNameType nameArray[], inout UInt16 *arrayEntriesP)
      libRefnum = ARG16;
      nameArrayP = ARG32;
      arrayEntriesP = ARG32;
      arrayEntries = 0;
      if (arrayEntriesP) arrayEntries = m68k_read_memory_16(arrayEntriesP);
      if (arrayEntries > 16) arrayEntries = 16;
      err = NetLibConfigList(libRefnum, nameArray, &arrayEntries);
      if (arrayEntries > 16) arrayEntries = 16;
      for (i = 0; i < arrayEntries; i++) {
        encode_NetConfigNameType(nameArrayP + i*4, &nameArray[i]);
      }
      if (arrayEntriesP) m68k_write_memory_16(arrayEntriesP, arrayEntries);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigList(refNum=%d, nameArray=0x%08X, arrayEntriesP=0x%08X [%d]): %d", libRefnum, nameArrayP, arrayEntriesP, arrayEntries, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case netLibTrapGetHostByName: {
      // NetHostInfoType *NetLibGetHostByName(UInt16 libRefnum, in Char *nameP, out NetHostInfoBufType *bufP, Int32 timeout, out Err *errP);
      libRefnum = ARG16;
      nameP = ARG32;
      s_nameP = emupalmos_trap_in(nameP, trap, 1);
      bufP = ARG32;

      timeout = ARG32;
      errP = ARG32;

      resnh = NetLibGetHostByName(libRefnum, s_nameP, &l_bufP, timeout, &l_errP);
      encode_NetHostInfoBufType(bufP, &l_bufP);
      if (errP) m68k_write_memory_16(errP, l_errP);
      r_res = resnh ? bufP : 0;
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibGetHostByName(libRefnum=%d, nameP=0x%08X, bufP=0x%08X, timeout=%d, errP=0x%08X): 0x%08X", libRefnum, nameP, bufP, timeout, errP, r_res);
      }
      break;
    case netLibTrapGetHostByAddr: {
      // NetHostInfoType *NetLibGetHostByAddr(UInt16 libRefnum, in UInt8 *addrP, UInt16 len, UInt16 type, out NetHostInfoBufType *bufP, Int32 timeout, out Err *errP);
      libRefnum = ARG16;
      addrP = ARG32;
      if (addrP) l_addrP = m68k_read_memory_32(addrP);
      len = ARG16;
      type = ARG16;
      bufP = ARG32;

      timeout = ARG32;
      errP = ARG32;

      resnh = NetLibGetHostByAddr(libRefnum, (UInt8 *)&l_addrP, len, type, &l_bufP, timeout, &l_errP);
      encode_NetHostInfoBufType(bufP, &l_bufP);
      if (errP) m68k_write_memory_16(errP, l_errP);
      r_res = resnh ? bufP : 0;
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibGetHostByAddr(libRefnum=%d, addrP=0x%08X, len=%d, type=%d, bufP=0x%08X, timeout=%d, errP=0x%08X): 0x%08X", libRefnum, addrP, len, type, bufP, timeout, errP, r_res);
      }
      break;
    case netLibTrapSend: {
      // Int16 NetLibSend(UInt16 libRefnum, NetSocketRef socket, in void *bufP, UInt16 bufLen, UInt16 flags, in void *toAddrP, UInt16 toLen, Int32 timeout, out Err *errP);
      libRefnum = ARG16;
      socket = ARG16;
      bufP = ARG32;
      s_bufP = emupalmos_trap_in(bufP, trap, 2);
      bufLen = ARG16;
      flags = ARG16;
      toAddrP = ARG32;
      decode_NetSocketAddrType(toAddrP, &toAddr);
      toLen = ARG16;
      timeout = ARG32;
      errP = ARG32;

      res = NetLibSend(libRefnum, socket, s_bufP, bufLen, flags, toAddrP ? &toAddr : NULL, toLen, timeout, &l_errP);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSend(libRefnum=%d, socket=%d, bufP=0x%08X, bufLen=%d, flags=%d, toAddrP=0x%08X, toLen=%d, timeout=%d, errP=0x%08X): %d", libRefnum, socket, bufP, bufLen, flags, toAddrP, toLen, timeout, errP, res);
      }
      break;
    case netLibTrapReceive: {
      // Int16 NetLibReceive(UInt16 libRefnum, NetSocketRef socket, out void *bufP, UInt16 bufLen, UInt16 flags, out void *fromAddrP, inout UInt16 *fromLenP, Int32 timeout, out Err *errP);
      libRefnum = ARG16;
      socket = ARG16;
      bufP = ARG32;
      s_bufP = emupalmos_trap_in(bufP, trap, 2);
      bufLen = ARG16;
      flags = ARG16;
      fromAddrP = ARG32;
      fromLenP = ARG32;

      if (fromLenP) l_fromLenP = m68k_read_memory_16(fromLenP);
      timeout = ARG32;
      errP = ARG32;

      res = NetLibReceive(libRefnum, socket, s_bufP, bufLen, flags, fromAddrP ? &fromAddr : NULL, fromLenP ? &l_fromLenP : NULL, timeout, &l_errP);
      encode_NetSocketAddrType(fromAddrP, &fromAddr);
      if (fromLenP) m68k_write_memory_16(fromLenP, l_fromLenP);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibReceive(libRefnum=%d, socket=%d, bufP=0x%08X, bufLen=%d, flags=%d, fromAddrP=0x%08X, fromLenP=0x%08X, timeout=%d, errP=0x%08X): %d", libRefnum, socket, bufP, bufLen, flags, fromAddrP, fromLenP, timeout, errP, res);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "NetLib trap 0x%04X not mapped", trap);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
