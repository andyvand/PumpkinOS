case sysTrapSysAppStartup: {
  // Err SysAppStartup(SysAppInfoPtr *appInfoPP, MemPtr *prevGlobalsP, MemPtr *globalsPtrP)
  appInfoPP = ARG32;
  prevGlobalsP = ARG32;
  globalsPtrP = ARG32;
  if (appInfoPP) m68k_write_memory_32(appInfoPP, state->sysAppInfoStart);
  if (prevGlobalsP) m68k_write_memory_32(prevGlobalsP, 0);
  if (globalsPtrP) m68k_write_memory_32(globalsPtrP, 0);
  debug(DEBUG_INFO, "EmuPalmOS", "SysAppStartup called");
  m68k_set_reg(M68K_REG_D0, 0);
}
break;
case sysTrapSysAppExit:
  // Err SysAppExit(SysAppInfoPtr appInfoP, MemPtr prevGlobalsP, MemPtr globalsP)
  debug(DEBUG_INFO, "EmuPalmOS", "SysAppExit called");
  m68k_set_reg(M68K_REG_D0, 0);
  m68k_pulse_halt();
  emupalmos_finish(1);
break;
case sysTrapSysGetAppInfo: {
  // SysAppInfoPtr SysGetAppInfo(SysAppInfoPtr *uiAppPP, SysAppInfoPtr *actionCodeAppPP)
  // XXX uiAppPP and actionCodeAppPP ignored
  debug(DEBUG_TRACE, "EmuPalmOS", "SysGetAppInfo(): 0x%08X", state->sysAppInfoStart);
  m68k_set_reg(M68K_REG_A0, state->sysAppInfoStart);
}
break;
case sysTrapSysTaskDelay: {
  // Err SysTaskDelay(Int32 delay)
  delay = ARG32;
  err = SysTaskDelay(delay);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysTaskDelay(%d): %d", delay, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSysLibFind: { // Return a reference number for a library that is already loaded, given its name.
  // Err SysLibFind(const Char *nameP, UInt16 *refNumP)
  nameP = ARG32;
  refNumP = ARG32;
  name = (char *)emupalmos_trap_in(nameP, trap, 0);
  emupalmos_trap_in(refNumP, trap, 1);
  if (SysLibFind(name, &refNum) != errNone || refNum == 0) {
    refNum = SysLibFind68K(name);
  }
  err = refNum ? errNone : sysErrLibNotFound;
  if (refNum == 0) refNum = 0xffff;
  if (refNumP) m68k_write_memory_16(refNumP, refNum);
  debug(DEBUG_INFO, "EmuPalmOS", "SysLibFind(0x%08X \"%s\", 0x%08X): %d", nameP, name ? name : "", refNumP, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSysLibLoad: {
  // Err SysLibLoad(UInt32 libType, UInt32 libCreator, UInt16 *refNumP)
  libType = ARG32;
  libCreator = ARG32;
  refNumP = ARG32;
  emupalmos_trap_in(refNumP, trap, 2);
  pumpkin_id2s(libType, buf);
  pumpkin_id2s(libCreator, buf2);
  debug(DEBUG_INFO, "EmuPalmOS", "SysLibLoad('%s', '%s', 0x%08X) native", buf, buf2, refNumP);
  r = state->SysLibLoad_addr;
}
break;
case sysTrapSysLibNewRefNum68K: {
  // Boolean SysLibNewRefNum68K(UInt32 type, UInt32 creator, UInt16 *refNum)
  type = ARG32;
  creator = ARG32;
  refNumP = ARG32;
  emupalmos_trap_in(refNumP, trap, 2);
  exists = SysLibNewRefNum68K(type, creator, &refNum);
  if (refNumP) m68k_write_memory_16(refNumP, refNum);
  pumpkin_id2s(type, buf);
  pumpkin_id2s(creator, buf2);
  debug(DEBUG_INFO, "EmuPalmOS", "SysLibNewRefNum68K('%s', '%s', 0x%08X): %d ", buf, buf2, refNumP, exists);
  m68k_set_reg(M68K_REG_D0, exists);
}
break;
case sysTrapSysLibRegister68K: {
  // Err SysLibRegister68K(UInt16 refNum, LocalID dbID, void *code, UInt32 size, UInt16 *dispatchTblP, UInt8 *globalsP)
  refNum = ARG16;
  id = ARG32;
  code = ARG32;
  size = ARG32;
  dispatchTblP = ARG32;
  globalsP = ARG32;
  dbIDL = id;
  err = SysLibRegister68K(refNum, dbIDL, emupalmos_trap_in(code, trap, 2), size, emupalmos_trap_in(dispatchTblP, trap, 4), emupalmos_trap_in(globalsP, trap, 5));
  if (err == errNone) {
      p = SysLibTblEntry68K(refNum, &tbl);
    if (p) {
  pP = emupalmos_trap_out(p);
  m68k_write_memory_32(pP +  0, emupalmos_trap_out(tbl.dispatchTblP));
  m68k_write_memory_32(pP +  4, emupalmos_trap_out(tbl.globalsP));
  m68k_write_memory_32(pP +  8, tbl.dbID);
  m68k_write_memory_32(pP + 12, 0); // XXX codeResH
    }
  }
  debug(DEBUG_INFO, "EmuPalmOS", "SysLibRegister68K(%d, 0x%08X, 0x%08X, %d, 0x%08X, 0x%08X)", refNum, id, code, size, dispatchTblP, globalsP);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSysLibCancelRefNum68K: {
  // void SysLibCancelRefNum68K(UInt16 refNum)
  refNum = ARG16;
  SysLibCancelRefNum68K(refNum);
  debug(DEBUG_INFO, "EmuPalmOS", "SysLibCancelRefNum68K(%d)", refNum);
}
break;
case sysTrapSysLibTblEntry: {
  // SysLibTblEntryType *SysLibTblEntry(UInt16 refNum)
  refNum = ARG16;
  p = SysLibTblEntry68K(refNum, &tbl);
  a = emupalmos_trap_out(p);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysLibTblEntry(%d): 0x%08X", refNum, a);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapSysLibRemove: {
  // Err SysLibRemove(UInt16 refNum)
  refNum = ARG16;
  SysLibCancelRefNum68K(refNum);
  debug(DEBUG_INFO, "EmuPalmOS", "SysLibRemove(%d): 0", refNum);
  m68k_set_reg(M68K_REG_D0, errNone);
}
break;
case sysTrapPceNativeCall: {
  // UInt32 PceNativeCall(NativeFuncType *nativeFuncP, userDataP)
#ifdef ARMEMU
  nativeFuncP = ARG32;
  userDataP = ARG32;
  emupalmos_trap_in(nativeFuncP, trap, 0);
  emupalmos_trap_in(userDataP, trap, 1);
  res = arm_native_call_pce(nativeFuncP, userDataP);
  debug(DEBUG_TRACE, "EmuPalmOS", "PceNativeCall(0x%08X, 0x%08X): %d", nativeFuncP, userDataP, res);
  m68k_set_reg(M68K_REG_A0, res);
  m68k_set_reg(M68K_REG_D0, res);
#endif
}
break;
case sysTrapSysGetStackInfo: {
  // Boolean SysGetStackInfo(MemPtr *startPP, MemPtr *endPP)
  startPP = ARG32;
  endPP = ARG32;
  emupalmos_trap_in(startPP, trap, 0);
  emupalmos_trap_in(endPP, trap, 1);
  // XXX
  //if (startPP) m68k_write_memory_32(startPP, state->stackStart);
  //if (endPP) m68k_write_memory_32(endPP, state->stackStart + stackSize);
  if (startPP) m68k_write_memory_32(startPP, state->stackStart + stackSize);
  if (endPP) m68k_write_memory_32(endPP, state->stackStart);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysGetStackInfo(0x%08X [0x%08X], 0x%08X [0x%08X]): %d", startPP, state->stackStart, endPP, state->stackStart + stackSize, true);
  m68k_set_reg(M68K_REG_D0, true);
}
break;
case sysTrapSysSetTrapAddress: {
  // Err SysSetTrapAddress(UInt16 trapNum, void *procP)
  trapNum = ARG16;
  procP = ARG32;
  selector = 0;
  emupalmos_trap_in(procP, trap, 1);
  s = logtrap_trapname(state->lt, trap, &selector, 0);
  res = sysErrParamErr;
  debug(DEBUG_INFO, "EmuPalmOS", "SysSetTrapAddress(0x%04X [ %s ], 0x%08X): %d", trapNum, s ? s : "unknown", procP, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapSysGetTrapAddress: {
  // void *SysGetTrapAddress(UInt16 trapNum)
  trapNum = ARG16;
  a = 0;
  selector = 0;
  s = logtrap_trapname(state->lt, trap, &selector, 0);
  if (s) {
   a = pumpkin_heap_size() + (trapNum << 2);
  }
  debug(DEBUG_INFO, "EmuPalmOS", "SysGetTrapAddress(0x%04X [ %s ]): 0x%08X", trapNum, s ? s : "unknown", a);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapHwrGetROMToken: {
  // Err HwrGetROMToken(UInt16 cardNo, UInt32 token, out UInt8 **dataP, out UInt16 *sizeP)
  cardNo = ARG16;
  token = ARG32;
  dataP = ARG32;
  sizeP = ARG32;
  emupalmos_trap_in(dataP, trap, 2);
  emupalmos_trap_in(sizeP, trap, 3);
  res = HwrGetROMToken(cardNo, token, &l_dataP, &l_sizeP);
  if (dataP) m68k_write_memory_32(dataP, emupalmos_trap_out(l_dataP));
  if (sizeP) m68k_write_memory_16(sizeP, l_sizeP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "HwrGetROMToken(cardNo=%d, token=%d, dataP=0x%08X, sizeP=0x%08X): %d", cardNo, token, dataP, sizeP, res);
}
break;
case sysTrapSysCreatePanelList: {
  // Boolean SysCreatePanelList(UInt16 *panelCount, MemHandle *panelIDs)
  panelCountP = ARG32;
  panelIDsP = ARG32;
  emupalmos_trap_in(panelCountP, trap, 0);
  emupalmos_trap_in(panelIDsP, trap, 1);
  resb = SysCreatePanelList(&panelCount, &panelIDs);
  if (panelCountP) m68k_write_memory_16(panelCountP, panelCount);
  if (panelIDsP) m68k_write_memory_32(panelIDsP, emupalmos_trap_out(panelIDs));
  m68k_set_reg(M68K_REG_D0, resb);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysCreatePanelList(panelCount=0x%08X, panelIDs=0x%08X): %d", panelCountP, panelIDsP, resb);
}
break;
case sysTrapSysInsertionSort:
case sysTrapSysQSort: {
  // void SysQSort(void *baseP, UInt16 numOfElements, Int16 width, CmpFuncPtr comparF, Int32 other)
  baseP = ARG32;
  numOfElements = ARG16;
  width = ARG16;
  comparF = ARG32;
  other = ARG32;
  base = emupalmos_trap_in(baseP, trap, 0);
  emupalmos_trap_in(comparF, trap, 3);
  SysQSort68k(base, numOfElements, (int16_t)width, comparF, other);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysQSort68k(0x%08X, %d, %d, 0x%08X, %d)", baseP, numOfElements, (int16_t)width, comparF, other);
}
break;
case sysTrapSysBinarySearch: {
  // Boolean SysBinarySearch(void const *baseP, UInt16 numOfElements, Int16 width, SearchFuncPtr searchF, void const *searchData, Int32 other, Int32 *position, Boolean findFirst)
  baseP = ARG32;
  numOfElements = ARG16;
  width = ARG16;
  searchF = ARG32;
  searchData = ARG32;
  other = ARG32;
  positionP = ARG32;
  findFirst = ARG8;
  emupalmos_trap_in(baseP, trap, 0);
  emupalmos_trap_in(searchF, trap, 3);
  emupalmos_trap_in(searchData, trap, 4);
  emupalmos_trap_in(positionP, trap, 6);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysBinarySearch(0x%08X, %d, %d, 0x%08X, 0x%08X, %d, 0x%08X, %d) native 0x%08X", baseP, numOfElements, (int16_t)width, searchF, searchData, other, positionP, findFirst, state->SysQSort_addr);
  r = state->SysBinarySearch_addr;
}
break;
case sysTrapHostControl: {
  // UInt32 HostControl(HostControlTrapNumber selector, ...)
  selector = ARG16;
  res = 0;
  debug(DEBUG_TRACE, "EmuPalmOS", "HostControl(0x%04X): 0x%08X", selector, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapSysNotifyRegister: {
  // Err SysNotifyRegister(UInt16 cardNo, LocalID dbID, UInt32 notifyType, SysNotifyProcPtr callbackP, Int8 priority, userDataP)
  cardNo = ARG16;
  dbID = ARG32;
  notifyType = ARG32;
  callbackP = ARG32;
  priority = ARG8;
  userDataP = ARG32;
  SysNotifyProcPtr callback = emupalmos_trap_in(callbackP, trap, 3);
  userData = emupalmos_trap_in(userDataP, trap, 5);
  res = SysNotifyRegister(cardNo, dbID, notifyType, callback, priority, userData);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysNotifyRegister(%d, 0x%08X, 0x%08X, 0x%08X, %u, 0x%08X): %d", cardNo, dbID, notifyType, callbackP, priority, userDataP, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapSysNotifyUnregister: {
  // Err SysNotifyUnregister(UInt16 cardNo, LocalID dbID, UInt32 notifyType, Int8 priority)
  cardNo = ARG16;
  dbID = ARG32;
  notifyType = ARG32;
  priority = ARG8;
  res = SysNotifyUnregister(cardNo, dbID, notifyType, priority);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysNotifyUnregister(%d, 0x%08X, 0x%08X, %u): %d", cardNo, dbID, notifyType, priority, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapSysNotifyBroadcast: {
  // Err SysNotifyBroadcast(SysNotifyParamType *notify)
  notifyP = ARG32;
  emupalmos_trap_in(notifyP, trap, 0);
  decode_notify(notifyP, &notify);
  res = SysNotifyBroadcast(notifyP ? &notify : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysNotifyBroadcast(0x%08X): %d", notifyP, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapSysFormPointerArrayToStrings: {
  // MemHandle SysFormPointerArrayToStrings(Char *c, Int16 stringCount)
  c = ARG32;
  emupalmos_trap_in(c, trap, 0);
  stringCount = ARG16;
  debug(DEBUG_TRACE, "EmuPalmOS", "SysFormPointerArrayToStrings(0x%08X, %d) native 0x%08X", c, stringCount, state->SysFormPointerArrayToStrings_addr);
  r = state->SysFormPointerArrayToStrings_addr;
}
break;
case sysTrapSysCopyStringResource: {
  // void SysCopyStringResource(string, Int16 theID)
  stringP = ARG32;
  theID = ARG16;
  string = emupalmos_trap_in(stringP, trap, 0);
  SysCopyStringResource(string, theID);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysCopyStringResource(0x%08X, %d)", stringP, theID);
}
break;
case sysTrapSysStringByIndex: {
  // sysStringByIndex(UInt16 resID, UInt16 index, strP, UInt16 maxLen)
  uresID = ARG16;
  index = ARG16;
  strP = ARG32;
  maxLen = ARG16;
  strp = emupalmos_trap_in(strP, trap, 2);
  resc = SysStringByIndex(uresID, (uint16_t)index, strP ? strp : NULL, maxLen);
  p32 = emupalmos_trap_out(resc);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysStringByIndex(%d, %d, 0x%08X, %d): 0x%08X", uresID, (uint16_t)index, strP, maxLen, p32);
  m68k_set_reg(M68K_REG_A0, p32);
}
break;
case sysTrapSysReset:
  // void SysReset(void)
  SysReset();
  debug(DEBUG_TRACE, "EmuPalmOS", "SysReset()");
break;
case sysTrapSysErrString: {
  // sysErrString(Err err, strP, UInt16 maxLen)
  err = ARG16;
  strP = ARG32;
  maxLen = ARG16;
  strc = emupalmos_trap_in(strP, trap, 1);
  resc = SysErrString((int16_t)err, strc, maxLen);
  p32 = emupalmos_trap_out(resc);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysErrString(%d, 0x%08X, %u): 0x%08X", (int16_t)err, strP, maxLen, p32);
  m68k_set_reg(M68K_REG_A0, p32);
}
break;
case sysTrapResLoadConstant: {
  // UInt32 ResLoadConstant(UInt16 rscID)
  rscID = ARG16;
  res = ResLoadConstant(rscID);
  debug(DEBUG_TRACE, "EmuPalmOS", "ResLoadConstant(%d): 0x%08X", rscID, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapAttnListOpen: {
  // void AttnListOpen(void)
  AttnListOpen();
  debug(DEBUG_TRACE, "EmuPalmOS", "AttnListOpen()");
}
break;
case sysTrapAttnIndicatorEnable: {
  // void AttnIndicatorEnable(Boolean enableIt)
  enableIt = ARG8;
  AttnIndicatorEnable((Boolean)enableIt);
  debug(DEBUG_TRACE, "EmuPalmOS", "AttnIndicatorEnable(%d)", enableIt);
}
break;
case sysTrapAttnIterate: {
  // void AttnIterate(UInt16 cardNo, LocalID dbID, UInt32 iterationData)
  cardNo = ARG16;
  dbID = ARG32;
  iterationData = ARG32;
  AttnIterate(cardNo, dbID, iterationData);
  debug(DEBUG_TRACE, "EmuPalmOS", "AttnIterate(%d, 0x%08X, %u)", cardNo, dbID, iterationData);
}
break;
case sysTrapDlkGetSyncInfo: {
  // Err DlkGetSyncInfo(UInt32 *succSyncDateP, UInt32 *lastSyncDateP, DlkSyncStateType *syncStateP, Char *nameBufP, Char *logBufP, Int32 *logLenP)
  succSyncDateP = ARG32;
  lastSyncDateP = ARG32;
  syncStateP = ARG32;
  nameBufP = ARG32;
  logBufP = ARG32;
  logLenP = ARG32;
  emupalmos_trap_in(succSyncDateP, trap, 0);
  emupalmos_trap_in(lastSyncDateP, trap, 1);
  emupalmos_trap_in(syncStateP, trap, 2);
  char *nameBuf = emupalmos_trap_in(nameBufP, trap, 3);
  char *logBuf = emupalmos_trap_in(logBufP, trap, 4);
  err = DlkGetSyncInfo(&succSyncDate, &lastSyncDate, &syncState, nameBuf, logBuf, &logLen);
  if (succSyncDateP) m68k_write_memory_32(succSyncDateP, succSyncDate);
  if (lastSyncDateP) m68k_write_memory_32(lastSyncDateP, lastSyncDate);
  if (syncStateP) m68k_write_memory_8(syncStateP, syncState);
  if (logLenP) m68k_write_memory_32(logLenP, logLen);
  debug(DEBUG_TRACE, "EmuPalmOS", "DlkGetSyncInfo(0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X): %d", succSyncDateP, lastSyncDateP, syncStateP, nameBufP, logBufP, logLenP, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapErrDisplayFileLineMsg: {
  // void ErrDisplayFileLineMsg(const Char * const filename, UInt16 lineNo, const Char * const msg)
  filenameP = ARG32;
  lineNo = ARG16;
  msgP = ARG32;
  filename = emupalmos_trap_in(filenameP, trap, 0);
  msg = emupalmos_trap_in(msgP, trap, 2);
  ErrDisplayFileLineMsg(filename, lineNo, msg);
  debug(DEBUG_INFO, "EmuPalmOS", "ErrDisplayFileLineMsg(0x%08X \"%s\", %d, 0x%08X \"%s\")", filenameP, filename ? filename : "", lineNo, msgP, msg ? msg : "");
}
break;
case sysTrapFileControl: {
  // Err FileControl(FileOpEnum op, FileHand stream, inout void *valueP, inout Int32 *valueLenP)
  op = ARG8;
  stream = ARG32;
  l_stream = (FileHand)emupalmos_trap_in(stream, trap, 1);
  valueP = ARG32;
  s_valueP = emupalmos_trap_in(valueP, trap, 2);
  valueLenP = ARG32;
  emupalmos_trap_in(valueLenP, trap, 3);
  if (valueLenP) l_valueLenP = m68k_read_memory_32(valueLenP);
  // XXX read valueP
  res = FileControl(op, l_stream, s_valueP, &l_valueLenP);
  // XXX fill valueP
  if (valueLenP) m68k_write_memory_32(valueLenP, l_valueLenP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileControl(op=%d, stream=0x%08X, valueP=0x%08X, valueLenP=0x%08X): %d", op, stream, valueP, valueLenP, res);
}
break;
case sysTrapFtrPtrNew: {
  // Err FtrPtrNew(UInt32 creator, UInt16 featureNum, UInt32 size, void **newPtrP)
  creator = ARG32;
  featureNum = ARG16;
  size = ARG32;
  newPtrP = ARG32;
  emupalmos_trap_in(newPtrP, trap, 3);
  p = MemPtrNew(size);
  if (p) {
    a = emupalmos_trap_out(p);
    if (newPtrP) m68k_write_memory_32(newPtrP, a);
    err = FtrSet(creator, featureNum, a);
  } else {
    err = memErrNotEnoughSpace;
  }
  pumpkin_id2s(creator, buf);
  debug(DEBUG_TRACE, "EmuPalmOS", "FtrPtrNew('%s', %d, %d, 0x%08X): %d", buf, featureNum, size, newPtrP, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapFtrPtrFree: {
  // Err FtrPtrFree(UInt32 creator, UInt16 featureNum)
  creator = ARG32;
  featureNum = ARG16;
  a = 0;
  err = FtrGet(creator, featureNum, &a);
  if (err == errNone && a) {
    p = emupalmos_trap_in(a, trap, -1);
    MemPtrFree(p);
  }
  pumpkin_id2s(creator, buf);
  debug(DEBUG_TRACE, "EmuPalmOS", "FtrPtrFree('%s', %d): %d", buf, featureNum, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapFtrUnregister: {
  // Err FtrUnregister(UInt32 creator, UInt16 featureNum)
  creator = ARG32;
  featureNum = ARG16;
  err = FtrUnregister(creator, featureNum);
  pumpkin_id2s(creator, buf);
  debug(DEBUG_TRACE, "EmuPalmOS", "FtrUnregister('%s', %d): %d", buf, featureNum, err);
}
break;
case sysTrapFtrGet: {
  // Err FtrGet(UInt32 creator, UInt16 featureNum, UInt32 *valueP)
  creator = ARG32;
  featureNum = ARG16;
  valueP = ARG32;
  emupalmos_trap_in(valueP, trap, 2);
  pumpkin_id2s(creator, buf);
  err = FtrGet(creator, featureNum, &value);

  if (creator == sysFileCSystem && featureNum == sysFtrNumProcessorID && err == errNone) {
#ifdef ARMEMU
    // If the processor is 68K, Cubis writes directly to the display bitmap. It works ONLY if the display is 8bpp.
    //value = sysFtrNumProcessorEZ;

    // If the processor is ARM, Cubis does not write directly to the display bitmap. It works both on 8pp and 16bpp. No hooks are necessary.
    value = sysFtrNumProcessorARM720T;
#else
    value = sysFtrNumProcessorEZ;
#endif
}

  debug(DEBUG_TRACE, "EmuPalmOS", "FtrGet('%s', %d, 0x%08X [0x%08X]): %d", buf, featureNum, valueP, value, err);
  m68k_write_memory_32(valueP, value);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapFtrSet: {
  // Err FtrSet(UInt32 creator, UInt16 featureNum, UInt32 newValue)
  creator = ARG32;
  featureNum = ARG16;
  newValue = ARG32;
  pumpkin_id2s(creator, buf);
  err = FtrSet(creator, featureNum, newValue);
  debug(DEBUG_TRACE, "EmuPalmOS", "FtrSet('%s', %d, %d): %d", buf, featureNum, newValue, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSelectOneTime: {
  // Boolean SelectOneTime(Int16 *hour, Int16 *minute, const titleP)
  uint32_t hourP = ARG32;
  minP = ARG32;
  titleP = ARG32;
  emupalmos_trap_in(hourP, trap, 0);
  emupalmos_trap_in(minP, trap, 1);
  title = (char *)emupalmos_trap_in(titleP, trap, 2);
  if (hourP) hour = m68k_read_memory_16(hourP);
  if (minP) min = m68k_read_memory_16(minP);
  resb = SelectOneTime(hourP ? &hour : NULL, minP ? &min : NULL, title);
  if (hourP) m68k_write_memory_16(hourP, hour);
  if (minP) m68k_write_memory_16(minP, min);
  debug(DEBUG_TRACE, "EmuPalmOS", "SelectOneTime(0x%08X, 0x%08X, 0x%08X): %d", hourP, minP, titleP, resb);
  m68k_set_reg(M68K_REG_D0, resb);
}
break;
case sysTrapSelectDay: {
  // Boolean SelectDay(const SelectDayType selectDayBy, Int16 *month, Int16 *day, Int16 *year, const title)
  selectDayBy = ARG8;
  monthP = ARG32;
  dayP = ARG32;
  yearP = ARG32;
  titleP = ARG32;
  emupalmos_trap_in(monthP, trap, 1);
  emupalmos_trap_in(dayP, trap, 2);
  emupalmos_trap_in(yearP, trap, 3);
  title = (char *)emupalmos_trap_in(titleP, trap, 4);
  if (monthP) month = m68k_read_memory_16(monthP);
  if (dayP) day = m68k_read_memory_16(dayP);
  if (yearP) year = m68k_read_memory_16(yearP);
  resb = SelectDay(selectDayBy, monthP ? &month : NULL, dayP ? &day : NULL, yearP ? &year : NULL, title);
  if (monthP) m68k_write_memory_16(monthP, month);
  if (dayP) m68k_write_memory_16(dayP, day);
  if (yearP) m68k_write_memory_16(yearP, year);
  debug(DEBUG_TRACE, "EmuPalmOS", "SelectDay(%d, 0x%08X, 0x%08X, 0x%08X, 0x%08X): %d", selectDayBy, monthP, dayP, yearP, titleP, resb);
  m68k_set_reg(M68K_REG_D0, resb);
}
break;
case sysTrapDaysInMonth: {
  // Int16 DaysInMonth(Int16 month, Int16 year)
  month = ARG16;
  year = ARG16;
  res = DaysInMonth(month, year);
  debug(DEBUG_TRACE, "EmuPalmOS", "DaysInMonth(%d, %d): %d", month, year, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapDayOfWeek: {
  // Int16 DayOfWeek(Int16 month, Int16 day, Int16 year)
  month = ARG16;
  day = ARG16;
  year = ARG16;
  res = DayOfWeek(month, day, year);
  debug(DEBUG_TRACE, "EmuPalmOS", "DayOfWeek(%d, %d, %d): %d", month, day, year, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapDateSecondsToDate: {
  // void DateSecondsToDate(seconds, DateType *dateP)
  seconds = ARG32;
  dateP = ARG32;
  emupalmos_trap_in(dateP, trap, 1);
  DateSecondsToDate(seconds, dateP ? &date.fields : NULL);
  if (dateP) m68k_write_memory_16(dateP, date.bits);
  debug(DEBUG_TRACE, "EmuPalmOS", "DateSecondsToDate(%u, 0x%08X)", seconds, dateP);
}
break;
case sysTrapDateToDOWDMFormat: {
  // void DateToDOWDMFormat(UInt8 months, UInt8 days, UInt16 years, DateFormatType dateFormat, Char *pString)
  months = ARG8;
  days = ARG8;
  years = ARG16;
  dateFormat = ARG8;
  stringP = ARG32;
  string = (char *)emupalmos_trap_in(stringP, trap, 4);
  DateToDOWDMFormat(months, (uint8_t)days, years, dateFormat, string);
  debug(DEBUG_TRACE, "EmuPalmOS", "DateToDOWDMFormat(%u, %u, %u, %u, 0x%08X)", months, (uint8_t)days, years, dateFormat, stringP);
}
break;
case sysTrapDateToAscii: {
  // void DateToAscii(UInt8 months, UInt8 days, UInt16 years, DateFormatType dateFormat, Char *pString)
  months = ARG8;
  days = ARG8;
  years = ARG16;
  dateFormat = ARG8;
  stringP = ARG32;
  string = (char *)emupalmos_trap_in(stringP, trap, 4);
  DateToAscii(months, (uint8_t)days, years, dateFormat, string);
  debug(DEBUG_TRACE, "EmuPalmOS", "DateToAscii(%u, %u, %u, %u, 0x%08X)", months, (uint8_t)days, years, dateFormat, stringP);
}
break;
case sysTrapDateToDays: {
  // UInt32 DateToDays(DateType date)
  date.bits = ARG16;
  res = DateToDays(date.fields);
  debug(DEBUG_TRACE, "EmuPalmOS", "DateToDays(0x%04X [%04d-%02d-%02d]): %d", date.bits, date.fields.year+1904, date.fields.month, date.fields.day, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapDateDaysToDate: {
  // void DateDaysToDate(UInt32 days, DateType *dateP)
  days = ARG32;
  dateP = ARG32;
  emupalmos_trap_in(dateP, trap, 1);
  DateDaysToDate(days, dateP ? &date.fields : NULL);
  if (dateP) m68k_write_memory_16(dateP, date.bits);
  debug(DEBUG_TRACE, "EmuPalmOS", "DateDaysToDate(%u, 0x%08X)", days, dateP);
}
break;
case sysTrapTimSetSeconds: {
  // void TimSetSeconds(seconds)
  seconds = ARG32;
  TimSetSeconds(seconds);
  debug(DEBUG_TRACE, "EmuPalmOS", "TimSetSeconds(%u)", seconds);
}
break;
case sysTrapTimAdjust: {
  // void TimAdjust(DateTimeType *dateTimeP, Int32 adjustment)
  dateTimeP = ARG32;
  adjustment = ARG32;
  emupalmos_trap_in(dateTimeP, trap, 0);
  decode_datetime(dateTimeP, &dateTime);
  TimAdjust(&dateTime, adjustment);
  encode_datetime(dateTimeP, &dateTime);
  debug(DEBUG_TRACE, "EmuPalmOS", "TimAdjust(0x%08X, %d)", dateTimeP, adjustment);
}
break;
case sysTrapDateAdjust: {
  // void DateAdjust(DateType *dateP, Int32 adjustment)
  dateP = ARG32;
  adjustment = ARG32;
  emupalmos_trap_in(dateP, trap, 0);
  if (dateP) date.bits = m68k_read_memory_16(dateP);
  DateAdjust(&date.fields, adjustment);
  if (dateP) m68k_write_memory_16(dateP, date.bits);
  debug(DEBUG_TRACE, "EmuPalmOS", "DateAdjust(0x%08X, %d)", dateP, adjustment);
}
break;
case sysTrapTimeToAscii: {
  // void TimeToAscii(UInt8 hours, UInt8 minutes, TimeFormatType timeFormat, Char *pString)
  hours = ARG8;
  minutes = ARG8;
  timeFormat = ARG8;
  stringP = ARG32;
  string = (char *)emupalmos_trap_in(stringP, trap, 3);
  TimeToAscii(hours, minutes, timeFormat, string);
  debug(DEBUG_TRACE, "EmuPalmOS", "TimeToAscii(%u, %u, %u, 0x%08X \"%s\")", hours, minutes, timeFormat, stringP, string ? string : "");
}
break;
case sysTrapTimeZoneToAscii: {
  // void TimeZoneToAscii(Int16 timeZone, const LmLocaleType *localeP, string)
  timeZone = ARG16;
  localeP = ARG32;
  stringP = ARG32;
  emupalmos_trap_in(localeP, trap, 1);
  string = (char *)emupalmos_trap_in(stringP, trap, 2);
  decode_locale(localeP, &locale);
  TimeZoneToAscii(timeZone, localeP ? &locale : NULL, string);
  debug(DEBUG_TRACE, "EmuPalmOS", "TimeZoneToAscii(%d, 0x%08X, 0x%08X )", timeZone, localeP, stringP);
}
break;
case sysTrapDateTemplateToAscii: {
  // UInt16 DateTemplateToAscii(const templateP, UInt8 months, UInt8 days, UInt16 years, stringP, Int16 stringLen)
  templateP = ARG32;
  months = ARG8;
  days = ARG8;
  years = ARG16;
  stringP = ARG32;
  stringLen = ARG16;
  template = (char *)emupalmos_trap_in(templateP, trap, 0);
  string = (char *)emupalmos_trap_in(stringP, trap, 4);
  res16 = DateTemplateToAscii(template, months, (uint8_t)days, years, string, stringLen);
  debug(DEBUG_TRACE, "EmuPalmOS", "DateTemplateToAscii(0x%08X, %u, %u, %u, 0x%08X \"%s\", %d): %u", templateP, months, (uint8_t)days, years, stringP, string ? string : "", stringLen, res16);
  m68k_set_reg(M68K_REG_D0, res16);
}
break;
case sysTrapTimDateTimeToSeconds: {
  // UInt32 TimDateTimeToSeconds(const DateTimeType *dateTimeP)
  dateTimeP = ARG32;
  emupalmos_trap_in(dateTimeP, trap, 0);
  decode_datetime(dateTimeP, &dateTime);
  seconds = TimDateTimeToSeconds(&dateTime);
  debug(DEBUG_TRACE, "EmuPalmOS", "TimDateTimeToSeconds(0x%08X [%04d-%02d-%02d %02d:%02d:%02d]): %u", dateTimeP, dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.minute, dateTime.second, seconds);
  m68k_set_reg(M68K_REG_D0, seconds);
}
break;
case sysTrapTimSecondsToDateTime: {
  // void TimSecondsToDateTime(seconds, DateTimeType *dateTimeP)
  seconds = ARG32;
  dateTimeP = ARG32;
  emupalmos_trap_in(dateTimeP, trap, 1);
  TimSecondsToDateTime(seconds, &dateTime);
  encode_datetime(dateTimeP, &dateTime);
  debug(DEBUG_TRACE, "EmuPalmOS", "TimSecondsToDateTime(%u, 0x%08X [%04d-%02d-%02d %02d:%02d:%02d])", seconds, dateTimeP, dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.minute, dateTime.second);
}
break;
case sysTrapTimGetSeconds: {
  // UInt32 TimGetSeconds(void)
  tm = TimGetSeconds();
  debug(DEBUG_TRACE, "EmuPalmOS", "TimGetSeconds(): %u", tm);
  m68k_set_reg(M68K_REG_D0, tm);
}
break;
case sysTrapTimGetTicks: {
  // UInt32 TimGetTicks(void)
  tm = TimGetTicks();
  debug(DEBUG_TRACE, "EmuPalmOS", "TimGetTicks(): %u", tm);
  m68k_set_reg(M68K_REG_D0, tm);
}
break;
case sysTrapFplInit:
  // Err FplInit(void)
  debug(DEBUG_TRACE, "EmuPalmOS", "FplInit()");
  m68k_set_reg(M68K_REG_D0, 0);
break;
case sysTrapFplFree:
  // void FplFree(void)
  debug(DEBUG_TRACE, "EmuPalmOS", "FplFree()");
break;
case sysTrapWinScreenMode: {
  // Err WinScreenMode(WinScreenModeOperation operation, UInt32 *widthP, UInt32 *heightP, UInt32 *depthP, Boolean *enableColorP)
  operation = ARG8;
  widthP = ARG32;
  heightP = ARG32;
  depthP = ARG32;
  enableColorP = ARG32;
  emupalmos_trap_in(widthP, trap, 1);
  emupalmos_trap_in(heightP, trap, 2);
  emupalmos_trap_in(depthP, trap, 3);
  emupalmos_trap_in(enableColorP, trap, 4);
  if (widthP) width = m68k_read_memory_32(widthP);
  if (heightP) height = m68k_read_memory_32(heightP);
  if (depthP) depth = m68k_read_memory_32(depthP);
  if (enableColorP) enableColor = m68k_read_memory_8(enableColorP);
  err = WinScreenMode(operation, widthP ? &width : NULL, heightP ? &height : NULL, depthP ? &depth : NULL, enableColorP ? &enableColor : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinScreenMode(%d, 0x%08X [%d], 0x%08X [%d], 0x%08X [%d], 0x%08X [%d]): %d",
    operation, widthP, width, heightP, height, depthP, depth, enableColorP, enableColor, err);
  if (widthP) m68k_write_memory_32(widthP, width);
  if (heightP) m68k_write_memory_32(heightP, height);
  if (depthP) m68k_write_memory_32(depthP, depth);
  if (enableColorP) m68k_write_memory_8(enableColorP, enableColor);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapWinPalette: {
  // Err WinPalette(UInt8 operation, Int16 startIndex, UInt16 paletteEntries, RGBColorType *tableP)
  // operation:
  // 0: winPaletteGet
  // 1: winPaletteSet
  // 2: winPaletteSetToDefault
  operation = ARG8;
  startIndex = ARG16;
  paletteEntries = ARG16;
  tableP = ARG32;
  emupalmos_trap_in(tableP, trap, 3);
  MemSet(table, sizeof(table), 0);
  if (operation == winPaletteSet && tableP) {
    if (startIndex == WinUseTableIndexes) {
      for (i = 0; i < paletteEntries && i < 256; i++) {
        uint32_t index = m68k_read_memory_8(tableP + i*4);
        decode_rgb(tableP + i*4, &table[i]);
        debug(DEBUG_TRACE, "EmuPalmOS", "palette %d: %u,%u,%u (i=%d)", index, table[i].r, table[i].g, table[i].b, i);
      }
    } else {
      for (i = 0; i < paletteEntries && i < 256; i++) {
        if (startIndex+i >= 0 && startIndex+i < 256) {
          decode_rgb(tableP + i*4, &table[i]);
          debug(DEBUG_TRACE, "EmuPalmOS", "palette %d: %u,%u,%u (start=%d, i=%d)", startIndex+i, table[i].r, table[i].g, table[i].b, startIndex, i);
        }
      }
    }
  }
  err = WinPalette(operation, startIndex, paletteEntries, tableP ? table : NULL);
  if (operation == winPaletteGet && tableP && err == errNone) {
    for (i = 0; i < paletteEntries; i++) {
      encode_rgb(tableP + i*4, &table[i]);
    }
  }
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPalette(%d, %d, %d, 0x%08X): %d", operation, startIndex, paletteEntries, tableP, err);
  wh = WinGetDrawWindow();
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPalette draw window 0x%08X", emupalmos_trap_out(wh));
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapFntDefineFont: {
  // Err FntDefineFont(FontID font, fontp)
  font = ARG8;
  fontP = ARG32;
  fontp = (FontPtr)emupalmos_trap_in(fontP, trap, 1);
  err = FntDefineFont(font, fontp);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntDefineFont(%d, 0x%08X): %d", font, fontP, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapWinCreateWindow: {
  // WinHandle WinCreateWindow(const RectangleType *bounds, FrameType frame, Boolean modal, Boolean focusable, UInt16 *error)
  uint32_t boundsP = ARG32;
  frame = ARG16;
  modal = ARG8;
  focusable = ARG8;
  errorP = ARG32;
  emupalmos_trap_in(boundsP, trap, 0);
  emupalmos_trap_in(errorP, trap, 4);
  decode_rectangle(boundsP, &bounds);
  wh = WinCreateWindow(boundsP ? &bounds : NULL, frame, modal, focusable, errorP ? &error16 : NULL);
  encode_rectangle(boundsP, &bounds);
  if (errorP) m68k_write_memory_16(errorP, error16);
  w = emupalmos_trap_out(wh);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinCreateWindow([%d,%d,%d,%d], %d, %d, %d, 0x%08X [%d]): 0x%08X", bounds.topLeft.x, bounds.topLeft.y, bounds.extent.x, bounds.extent.y, frame, modal, focusable, errorP, error16, w);
  m68k_set_reg(M68K_REG_A0, w);
}
break;
case sysTrapWinCreateBitmapWindow: {
  // WinHandle WinCreateBitmapWindow(bitmapP, UInt16 *error)
  bitmapP = ARG32;
  errorP = ARG32;
  bitmap = (BitmapType *)emupalmos_trap_in(bitmapP, trap, 0);
  wh = WinCreateBitmapWindow(bitmap, &error16);
  if (errorP) m68k_write_memory_16(errorP, error16);
  w = emupalmos_trap_out(wh);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinCreateBitmapWindow(0x%08X, 0x%08X [%d]): 0x%08X", bitmapP, errorP, error16, w);
  m68k_set_reg(M68K_REG_A0, w);
}
break;
case sysTrapWinCreateOffscreenWindow: {
  // WinHandle WinCreateOffscreenWindow(Coord width, Coord height, WindowFormatType format, UInt16 *error)
  width = ARG16;
  height = ARG16;
  format = ARG8;
  errorP = ARG32;
  emupalmos_trap_in(errorP, trap, 3);
  wh = WinCreateOffscreenWindow((uint16_t)width, (uint16_t)height, (uint8_t)format, errorP ? &error16 : NULL);
  if (errorP) m68k_write_memory_16(errorP, error16);
  w = emupalmos_trap_out(wh);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinCreateOffscreenWindow(%d, %d, %d, 0x%08X [%d]): 0x%08X", (uint16_t)width, (uint16_t)height, format, errorP, error16, w);
  m68k_set_reg(M68K_REG_A0, w);
}
break;
case sysTrapWinDeleteWindow: {
  // void WinDeleteWindow(WinHandle winHandle, Boolean eraseIt)
  w = ARG32;
  eraseIt = ARG8;
  wh = (WinHandle)emupalmos_trap_in(w, trap, 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDeleteWindow(0x%08X, %d) ...", w, eraseIt);
  WinDeleteWindow(wh, eraseIt);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDeleteWindow(0x%08X, %d)", w, eraseIt);
}
break;
case sysTrapRctSetRectangle: {
  // void RctSetRectangle(RectangleType *rP, Coord left, Coord top, Coord width, Coord height)
  rP = ARG32;
  left = ARG16;
  top = ARG16;
  width = ARG16;
  height = ARG16;
  emupalmos_trap_in(rP, trap, 0);
  RctSetRectangle(rP ? &rect : NULL, left, top, (int16_t)width, (int16_t)height);
  encode_rectangle(rP, &rect);
  debug(DEBUG_TRACE, "EmuPalmOS", "RctSetRectangle(0x%08X [%d,%d,%d,%d], %d, %d, %d, %d)", rP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y, left, top, (int16_t)width, (int16_t)height);
}
break;
case sysTrapRctInsetRectangle: {
  // void RctInsetRectangle(RectangleType *rP, Coord insetAmt)
  rP = ARG32;
  insetAmt = ARG16;
  emupalmos_trap_in(rP, trap, 0);
  decode_rectangle(rP, &rect);
  RctInsetRectangle(rP ? &rect : NULL, insetAmt);
  encode_rectangle(rP, &rect);
  debug(DEBUG_TRACE, "EmuPalmOS", "RctInsetRectangle(0x%08X [%d,%d,%d,%d], %d)",
    rP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y, insetAmt);
}
break;
case sysTrapRctOffsetRectangle: {
  // void RctOffsetRectangle(RectangleType *rP, Coord deltaX, Coord deltaY)
  rP = ARG32;
  deltaX = ARG16;
  deltaY = ARG16;
  emupalmos_trap_in(rP, trap, 0);
  decode_rectangle(rP, &rect);
  RctOffsetRectangle(rP ? &rect : NULL, deltaX, deltaY);
  encode_rectangle(rP, &rect);
  debug(DEBUG_TRACE, "EmuPalmOS", "RctOffsetRectangle(0x%08X [%d,%d,%d,%d], %d, %d)",
    rP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y, deltaX, deltaY);
}
break;
case sysTrapRctCopyRectangle: {
  // void RctCopyRectangle(const RectangleType *srcRectP, RectangleType *dstRectP)
  srcRectP = ARG32;
  dstRectP = ARG32;
  emupalmos_trap_in(srcRectP, trap, 0);
  emupalmos_trap_in(dstRectP, trap, 1);
  decode_rectangle(srcRectP, &src);
  RctCopyRectangle(srcRectP ? &src : NULL, dstRectP ? &dst : NULL);
  encode_rectangle(dstRectP, &dst);
  debug(DEBUG_TRACE, "EmuPalmOS", "RctCopyRectangle(0x%08X [%d,%d,%d,%d], 0x%08X [%d,%d,%d,%d])",
    srcRectP, src.topLeft.x, src.topLeft.y, src.extent.x, src.extent.y,
    dstRectP, dst.topLeft.x, dst.topLeft.y, dst.extent.x, dst.extent.y);
}
break;
case sysTrapRctPtInRectangle: {
  // Boolean RctPtInRectangle(Coord x, Coord y, const RectangleType *rP)
  x = ARG16;
  y = ARG16;
  rP = ARG32;
  emupalmos_trap_in(rP, trap, 2);
  decode_rectangle(rP, &rect);
  resb = RctPtInRectangle((int16_t)x, (int16_t)y, rP ? &rect : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "RctPtInRectangle(%d, %d, 0x%08X [%d,%d,%d,%d]): %d", (int16_t)x, (int16_t)y, rP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y, resb);
  m68k_set_reg(M68K_REG_D0, resb);
}
break;
case sysTrapRctGetIntersection: {
  // void RctGetIntersection(const RectangleType *r1P, const RectangleType *r2P, RectangleType *r3P)
  r1P = ARG32;
  r2P = ARG32;
  r3P = ARG32;
  emupalmos_trap_in(r1P, trap, 0);
  emupalmos_trap_in(r2P, trap, 1);
  emupalmos_trap_in(r3P, trap, 2);
  decode_rectangle(r1P, &rect1);
  decode_rectangle(r2P, &rect2);
  decode_rectangle(r3P, &rect3);
  RctGetIntersection(r1P ? &rect1 : NULL, r2P ? &rect2 : NULL, r3P ? &rect3 : NULL);
  encode_rectangle(r1P, &rect1);
  encode_rectangle(r2P, &rect2);
  encode_rectangle(r3P, &rect3);
  debug(DEBUG_TRACE, "EmuPalmOS", "RctGetIntersection(0x%08X, 0x%08X, 0x%08X)", r1P, r2P, r3P);
}
break;
case sysTrapBmpCreate: {
  // BitmapType *BmpCreate(Coord width, Coord height, UInt8 depth, ColorTableType *colorTableP, UInt16 *error)
  width = ARG16;
  height = ARG16;
  depth = ARG8;
  colorTableP = ARG32;
  errorP = ARG32;
  emupalmos_trap_in(errorP, trap, 4);
  bitmap = BmpCreate((int16_t)width, (int16_t)height, (uint8_t)depth, (ColorTableType *)emupalmos_trap_in(colorTableP, trap, 3), errorP ? &error16 : NULL);
  a = emupalmos_trap_out(bitmap);
  if (errorP) m68k_write_memory_16(errorP, error16);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpCreate(width=%d, height=%d, depth=%d, colorTableP=0x%08X, error=0x%08X [%d]): 0x%08X", (int16_t)width, (int16_t)height, (uint8_t)depth, colorTableP, errorP, error16, a);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapBmpDelete: {
  // Err BmpDelete(bitmapP)
  bitmapP = ARG32;
  bitmap = (BitmapType *)emupalmos_trap_in(bitmapP, trap, 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpDelete(0x%08X) ...", bitmapP);
  err = BmpDelete(bitmap);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpDelete(0x%08X): %d", bitmapP, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSecSelectViewStatus: {
  // privateRecordViewEnum SecSelectViewStatus(void)
  rprv = SecSelectViewStatus();
  debug(DEBUG_TRACE, "EmuPalmOS", "SecSelectViewStatus(): %d", rprv);
  m68k_set_reg(M68K_REG_D0, rprv);
}
break;
case sysTrapFontSelect: {
  // FontID FontSelect(FontID fontID)
  fontID = ARG8;
  oldFontID = FontSelect(fontID);
  debug(DEBUG_TRACE, "EmuPalmOS", "FontID(%d): %d", fontID, oldFontID);
  m68k_set_reg(M68K_REG_D0, oldFontID);
}
break;
case sysTrapUIColorPushTable: {
  // Err UIColorPushTable(void)
  err = UIColorPushTable();
  debug(DEBUG_TRACE, "EmuPalmOS", "UIColorPushTable(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapUIColorPopTable: {
  // Err UIColorPopTable(void)
  err = UIColorPopTable();
  debug(DEBUG_TRACE, "EmuPalmOS", "UIColorPopTable(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapUIColorSetTableEntry: {
  // Err UIColorSetTableEntry(UIColorTableEntries which, const RGBColorType *rgbP)
  which = ARG8;
  rgbP = ARG32;
  emupalmos_trap_in(rgbP, trap, 1);
  decode_rgb(rgbP, &rgb);
  err = UIColorSetTableEntry(which, rgbP ? &rgb : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "UIColorSetTableEntry(%d, 0x%08X [%d,%d,%d,%d]): %d", which, rgbP, rgb.index, rgb.r, rgb.g, rgb.b, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapUIColorGetTableEntryRGB: {
  // void UIColorGetTableEntryRGB(UIColorTableEntries which, RGBColorType *rgbP)
  which = ARG8;
  rgbP = ARG32;
  emupalmos_trap_in(rgbP, trap, 1);
  UIColorGetTableEntryRGB(which, rgbP ? &rgb : NULL);
  encode_rgb(rgbP, &rgb);
  debug(DEBUG_TRACE, "EmuPalmOS", "UIColorGetTableEntryRGB(%d, 0x%08X [%d,%d,%d,%d])", which, rgbP, rgb.index, rgb.r, rgb.g, rgb.b);
}
break;
case sysTrapUIColorGetTableEntryIndex: {
  // IndexedColorType UIColorGetTableEntryIndex(UIColorTableEntries which)
  which = ARG8;
  cct = UIColorGetTableEntryIndex(which);
  debug(DEBUG_TRACE, "EmuPalmOS", "UIColorGetTableEntryIndex(%d): %d", which, cct);
  m68k_set_reg(M68K_REG_D0, cct);
}
break;
case sysTrapPrefGetPreferences: {
  // void PrefGetPreferences(SystemPreferencesPtr p)
  p32 = ARG32;
  emupalmos_trap_in(p32, trap, 0);
  PrefGetPreferences(p32 ? &prefs : NULL);
  // XXX decode prefs into p
  debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetPreferences(0x%08X)", p32);
}
break;
case sysTrapPrefSetPreferences: {
  // void PrefSetPreferences(SystemPreferencesPtr p)
  p32 = ARG32;
  emupalmos_trap_in(p32, trap, 0);
  // XXX encode p into prefs
  PrefSetPreferences(p32 ? &prefs : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrefSetPreferences(0x%08X)", p32);
}
break;
case sysTrapPrefGetPreference: {
  // UInt32 PrefGetPreference(SystemPreferencesChoice choice)
  choice = ARG8;
  value = PrefGetPreference(choice);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetPreference(%d): %d", choice, value);
  m68k_set_reg(M68K_REG_D0, value);
}
break;
case sysTrapPrefSetPreference: {
  //void PrefSetPreference(SystemPreferencesChoice choice, UInt32 value)
  choice = ARG8;
  value = ARG32;
  PrefSetPreference(choice, value);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrefSetPreference(%d, %d)", choice, value);
}
break;
case sysTrapPrefOpenPreferenceDB: {
  // DmOpenRef PrefOpenPreferenceDB(Boolean saved)
  saved = ARG8;
  dbRef = PrefOpenPreferenceDB(saved);
  a = emupalmos_trap_out(dbRef);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrefOpenPreferenceDB(%d): 0x%08X", saved, a);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapPrefOpenPreferenceDBV10: {
  // DmOpenRef PrefOpenPreferenceDBV10(void)
  dbRef = PrefOpenPreferenceDBV10();
  a = emupalmos_trap_out(dbRef);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrefOpenPreferenceDBV10(): 0x%08X", a);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapPrefSetAppPreferences: {
  // void PrefSetAppPreferences(UInt32 creator, UInt16 id, Int16 version, const void *prefs, prefsSize, Boolean saved)
  creator = ARG32;
  id = ARG16;
  version = ARG16;
  prefsP = ARG32;
  prefsSize = ARG16;
  saved = ARG8;
  PrefSetAppPreferences(creator, (uint16_t)id, version, emupalmos_trap_in(prefsP, trap, 3), prefsSize, saved);
  pumpkin_id2s(creator, buf);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrefSetAppPreferences('%s', %d, %d, 0x%08X, %d, %d)", buf, (uint16_t)id, version, prefsP, prefsSize, saved);
}
break;
case sysTrapPrefSetAppPreferencesV10: {
  // void PrefSetAppPreferencesV10(UInt32 creator, Int16 version, void *prefs, prefsSize)
  creator = ARG32;
  version = ARG16;
  prefsP = ARG32;
  prefsSize = ARG16;
  PrefSetAppPreferencesV10(creator, version, emupalmos_trap_in(prefsP, trap, 2), prefsSize);
  pumpkin_id2s(creator, buf);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrefSetAppPreferencesV10('%s', %d, 0x%08X, %d)", buf, version, prefsP, prefsSize);
}
break;
case sysTrapPrefGetAppPreferences: {
  // Int16 PrefGetAppPreferences(UInt32 creator, UInt16 id, void *prefs, UInt16 *prefsSize, Boolean saved)
  creator = ARG32;
  id = ARG16;
  prefsP = ARG32;
  prefsSizeP = ARG32;
  saved = ARG8;
  emupalmos_trap_in(prefsSizeP, trap, 3);
  prefsSize = prefsSizeP ? m68k_read_memory_16(prefsSizeP) : 0;
  versionu = PrefGetAppPreferences(creator, (uint16_t)id, emupalmos_trap_in(prefsP, trap, 2), prefsSizeP ? &prefsSize : NULL, saved);
  pumpkin_id2s(creator, buf);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetAppPreferences('%s', %d, 0x%08X, 0x%08X, %d): %d", buf, (uint16_t)id, prefsP, prefsSizeP, saved, versionu);
  if (prefsSizeP) m68k_write_memory_16(prefsSizeP, prefsSize);
  m68k_set_reg(M68K_REG_D0, version);
}
break;
case sysTrapPrefGetAppPreferencesV10: {
  // Boolean PrefGetAppPreferencesV10(UInt32 type, Int16 version, void *prefs, prefsSize)
  type = ARG32;
  versionu = ARG16;
  prefsP = ARG32;
  prefsSize = ARG16;
  b = PrefGetAppPreferencesV10(type, versionu, emupalmos_trap_in(prefsP, trap, 2), prefsSize);
  pumpkin_id2s(type, buf);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetAppPreferencesV10('%s', %d, 0x%08X, %d): %d", buf, versionu, prefsP, prefsSize, b);
  m68k_set_reg(M68K_REG_D0, b);
}
break;
case sysTrapMemSet: {
  // Err MemSet(void *dstP, Int32 numBytes, UInt8 value)
  dstP = ARG32;
  numBytes = ARG32;
  value = ARG8;
  WinLegacyGetAddr(&start, &end);
  if ((dstP >= start && dstP < end) ||
      (dstP+numBytes-1 >= start && dstP+numBytes-1 < end) ||
      (dstP < start && dstP+numBytes >= end)) {
    debug(DEBUG_TRACE, "EmuPalmOS", "MemSet(0x%08X, %d, 0x%02X) inside screen", dstP, numBytes, (uint8_t)value);
    for (i = 0; i < numBytes; i++) {
      m68k_write_memory_8(dstP+i, (uint8_t)value);
    }
    err = 0;
  } else {
    if (emupalmos_check_address(dstP, numBytes, 0)) {
      err = MemSet(emupalmos_trap_in(dstP, trap, 0), numBytes, (uint8_t)value);
    } else {
      err = dmErrInvalidParam;
    }
  }
  debug(DEBUG_TRACE, "EmuPalmOS", "MemSet(0x%08X, %d, 0x%02X): %d", dstP, numBytes, (uint8_t)value, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapMemMove: {
  // Err MemMove(void *dstP, const void *sP, Int32 numBytes)
  dstP = ARG32;
  sP = ARG32;
  numBytesi = ARG32;
  WinLegacyGetAddr(&start, &end);
  if ((dstP >= start && dstP < end) ||
      (dstP+numBytesi-1 >= start && dstP+numBytesi-1 < end) ||
      (dstP < start && dstP+numBytesi >= end) ||
      (sP >= start && sP < end) ||
      (sP+numBytesi-1 >= start && sP+numBytesi-1 < end) ||
      (sP < start && sP+numBytesi >= end)) {
    debug(DEBUG_TRACE, "EmuPalmOS", "MemMove(0x%08X, 0x%08X, %d) inside screen", dstP, sP, numBytesi);
    for (i = 0; i < numBytesi; i++) {
      value = m68k_read_memory_8(sP+i);
      m68k_write_memory_8(dstP+i, (uint8_t)value);
    }
    err = 0;
  } else {
    if (emupalmos_check_address(dstP, numBytes, 0) && emupalmos_check_address(sP, numBytesi, 1)) {
      err = MemMove(emupalmos_trap_in(dstP, trap, 0), emupalmos_trap_in(sP, trap, 1), numBytesi);
    } else {
      err = dmErrInvalidParam;
    }
  }
  debug(DEBUG_TRACE, "EmuPalmOS", "MemMove(0x%08X, 0x%08X, %d): %d", dstP, sP, numBytesi, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSysRandom: {
  // Int16 SysRandom(Int32 newSeed)
  newSeed = ARG32;
  res = SysRandom(newSeed);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysRandom(%d): %d", newSeed, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapDmDetachRecord: {
  // Err DmDetachRecord(DmOpenRef dbP, UInt16 index, MemHandle *oldHP)
  dbP = ARG32;
  index = ARG16;
  oldHP = ARG32;
  emupalmos_trap_in(oldHP, trap, 2);
  dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
  res = DmDetachRecord(dbRef, (uint16_t)index, oldHP ? &old : NULL);
  if (oldHP) m68k_write_memory_32(oldHP, emupalmos_trap_out(old));
  debug(DEBUG_TRACE, "EmuPalmOS", "DmDetachRecord(0x%08X, %d, 0x%08X): %d", dbP, (uint16_t)index, oldHP, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapDmDetachResource: {
  // Err DmDetachResource(DmOpenRef dbP, UInt16 index, MemHandle *oldHP)
  dbP = ARG32;
  index = ARG16;
  oldHP = ARG32;
  emupalmos_trap_in(oldHP, trap, 2);
  dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
  res = DmDetachResource(dbRef, (uint16_t)index, oldHP ? &old : NULL);
  if (oldHP) m68k_write_memory_32(oldHP, emupalmos_trap_out(old));
  debug(DEBUG_TRACE, "EmuPalmOS", "DmDetachsource(0x%08X, %d, 0x%08X): %d", dbP, (uint16_t)index, oldHP, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapDmSearchResource: {
  // UInt16 DmSearchResource(DmResType resType, DmResID resID, MemHandle resH, DmOpenRef *dbPP)
  type = ARG32;
  resID = ARG16;
  ih = ARG32;
  dbPP = ARG32;
  hm = emupalmos_trap_in(ih, trap, 2);
  emupalmos_trap_in(dbPP, trap, 3);
  UInt16 index = DmSearchResource(type, resID, hm, dbPP ? &dbPOR : NULL);
  if (dbPP) m68k_write_memory_32(dbPP, emupalmos_trap_out(dbPOR));
  pumpkin_id2s(type, buf);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSearchResource('%s', %d, 0x%08X, 0x%08X): %d", buf, resID, ih, dbPP, index);
  m68k_set_reg(M68K_REG_D0, index);
}
break;
case sysTrapMemHandleLock: {
  // MemPtr MemHandleLock(MemHandle h)
  ih = ARG32;
  hm = emupalmos_trap_in(ih, trap, 0);
  p = MemHandleLock(hm);
  a = emupalmos_trap_out(p);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleLock(0x%08X): 0x%08X (%p)", ih, a, p);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapDmGetNextDatabaseByTypeCreator: {
  // Err DmGetNextDatabaseByTypeCreator(Boolean newSearch, DmSearchStatePtr stateInfoP, UInt32 type, UInt32 creator, Boolean onlyLatestVers, UInt16 *cardNoP, LocalID *dbIDP)
  newSearch = ARG8;
  stateInfoP = ARG32;
  type = ARG32;
  creator = ARG32;
  onlyLatestVers = ARG8;
  cardNoP = ARG32;
  dbIDP = ARG32;
  emupalmos_trap_in(stateInfoP, trap, 1);
  emupalmos_trap_in(cardNoP, trap, 5);
  emupalmos_trap_in(dbIDP, trap, 6);
  dbIDL = 0;
  if (stateInfoP && !newSearch) {
    info = m68k_read_memory_32(stateInfoP);
    stateInfo.p = emupalmos_trap_in(info, trap, -1);
  }
  err = DmGetNextDatabaseByTypeCreator(newSearch, stateInfoP ? &stateInfo : NULL, type, creator, onlyLatestVers, cardNoP ? &cardNo : NULL, dbIDP ? &dbIDL : NULL);
  if (stateInfoP) {
    info = emupalmos_trap_out(stateInfo.p);
    m68k_write_memory_32(stateInfoP, info);
  }
  if (cardNoP) m68k_write_memory_16(cardNoP, cardNo);
  if (dbIDP) m68k_write_memory_32(dbIDP, dbIDL);
  pumpkin_id2s(type, buf);
  pumpkin_id2s(creator, buf2);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetNextDatabaseByTypeCreator(%d, 0x%08X, '%s', '%s', %d, 0x%08X, 0x%08X): %d", newSearch, stateInfoP, buf, buf2, onlyLatestVers, cardNoP, dbIDP, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapDmInsertionSort: {
  // Err DmInsertionSort(DmOpenRef dbP, DmComparF *comparF, Int16 other)
  dbP = ARG32;
  comparP = ARG32;
  other = ARG16;
  dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
  res = DmInsertionSort68K(dbRef, comparP, (int16_t)other);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmInsertionSort(0x%08X, 0x%08X, %d): %d", dbP, comparP, (int16_t)other, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapDmQuickSort: {
  // Err DmQuickSort(DmOpenRef dbP, DmComparF *comparF, Int16 other)
  dbP = ARG32;
  comparP = ARG32;
  other = ARG16;
  dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
  emupalmos_trap_in(comparP, trap, 1);
  res = DmQuickSort68K(dbRef, comparP, (int16_t)other);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmQuickSort(0x%08X, 0x%08X, %d): %d", dbP, comparP, (int16_t)other, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapDmFindSortPositionV10: {
  // UInt16 DmFindSortPositionV10(DmOpenRef dbP, void *newRecord, DmComparF *compar, Int16 other)
  dbP = ARG32;
  newRecordP = ARG32;
  comparP = ARG32;
  other = ARG16;
  dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
  emupalmos_trap_in(newRecordP, trap, 1);
  emupalmos_trap_in(comparP, trap, 2);
  UInt16 res = DmFindSortPosition68K(dbRef, newRecordP, 0, comparP, (int16_t)other);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmFindSortPositionV10(0x%08X, 0x%08X, 0x%08X, %d): %d", dbP, newRecordP, comparP, (int16_t)other, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapDmFindSortPosition: {
  // UInt16 DmFindSortPosition(DmOpenRef dbP, void *newRecord, SortRecordInfoPtr newRecordInfo, DmComparF *compar, Int16 other)
  dbP = ARG32;
  newRecordP = ARG32;
  newRecordInfoP = ARG32;
  comparP = ARG32;
  other = ARG16;
  dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
  emupalmos_trap_in(newRecordP, trap, 1);
  emupalmos_trap_in(newRecordInfoP, trap, 2);
  emupalmos_trap_in(comparP, trap, 3);
  UInt16 res = DmFindSortPosition68K(dbRef, newRecordP, newRecordInfoP, comparP, (int16_t)other);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmFindSortPosition(0x%08X, 0x%08X, 0x%08X, 0x%08X, %d): %d", dbP, newRecordP, newRecordInfoP, comparP, (int16_t)other, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapDmAttachRecord: {
  // Err DmAttachRecord(DmOpenRef dbP, UInt16 *atP, MemHandle newH, MemHandle *oldHP)
  dbP = ARG32;
  atP = ARG32;
  newH = ARG32;
  oldHP = ARG32;
  dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
  emupalmos_trap_in(atP, trap, 1);
  at = atP ? m68k_read_memory_16(atP) : 0;
  hm = emupalmos_trap_in(newH, trap, 2);
  emupalmos_trap_in(oldHP, trap, 3);
  res = DmAttachRecord(dbRef, atP ? &at : NULL, hm, oldHP ? &old : NULL);
  if (atP) m68k_write_memory_16(atP, at);
  if (oldHP) m68k_write_memory_32(oldHP, emupalmos_trap_out(old));
  debug(DEBUG_TRACE, "EmuPalmOS", "DmAttachRecord(0x%08X, 0x%08X, 0x%08X, 0x%08X): %d", dbP, atP, newH, oldHP, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapDmSync:
  // void DmSync(void)
  DmSync();
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSync()");
break;
case sysTrapDmSyncDatabase: {
  // Err DmSyncDatabase(dbRef)
  dbP = ARG32;
  dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
  res = DmSyncDatabase(dbRef);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSyncDatabase(0x%08X): %d", dbP, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapStrVPrintF:
case sysTrapStrPrintF: {
  // Int16 StrVPrintF(s, const Char *formatStr, _Palm_va_list arg)
  // Int16 StrPrintF(s, const Char *formatStr, ...)
  str = ARG32;
  formatStr = ARG32;
  s = emupalmos_trap_in(str, trap, 0);
  f = emupalmos_trap_in(formatStr, trap, 1);
  vararg = trap == sysTrapStrVPrintF;
  res = 0;
  if (s && f) {
    if (vararg) {
      v_arg = ARG32;
    } else {
      v_arg = 0;
    }
    for (i = 0, p = (uint8_t *)s; f[i]; i++) {
      switch (t) {
        case 0:
          if (f[i] == '%') {
            j = 0;
            fmt[j++] = f[i];
            arglen = -1;
            sz = 2;
            t = 1;
          } else {
            *p++ = f[i];
          }
          break;
        case 1:
          switch (f[i]) {
            case 'h':
            case 'H':
              fmt[j++] = f[i];
              sz = 2;
              break;
            case 'l':
            case 'L':
              fmt[j++] = f[i];
              sz = 4;
              break;
            case 'd':
            case 'i':
            case 'u':
            case 'x':
            case 'X':
              if (vararg) {
                switch (sz) {
                  case 1:  arg = m68k_read_memory_16(v_arg) & 0xff; v_arg += 2; break;
                  case 2:  arg = m68k_read_memory_16(v_arg); v_arg += 2; break;
                  case 4:  arg = m68k_read_memory_32(v_arg); v_arg += 4; break;
                  default: arg = m68k_read_memory_16(v_arg); v_arg += 2; break;
                }
              } else {
                switch (sz) {
                  case 1:  arg = ARG8;  break;
                  case 2:  arg = ARG16; break;
                  case 4:  arg = ARG32; break;
                  default: arg = ARG16; break;
                }
              }
              k++;
              fmt[j++] = f[i];
              fmt[j] = 0;
              sys_sprintf((char *)p, fmt, arg);
              p += sys_strlen((char *)p);
              t = 0;
              break;
            case 'c':
            case 'C':
              if (vararg) {
                arg = m68k_read_memory_16(v_arg) & 0xff;
                v_arg += 2;
              } else {
                arg = ARG8;
              }
              k++;
              fmt[j++] = f[i];
              fmt[j] = 0;
              sys_sprintf((char *)p, fmt, arg);
              p += sys_strlen((char *)p);
              t = 0;
              break;
            case 's':
              if (vararg) {
                arg = m68k_read_memory_32(v_arg);
                v_arg += 4;
              } else {
                arg = ARG32;
              }
              k++;
              q = emupalmos_trap_in(arg, trap, k);
              fmt[j++] = f[i];
              fmt[j] = 0;
              if (arglen < 0) {
                sys_sprintf((char *)p, fmt, q);
              } else {
                sys_sprintf((char *)p, fmt, arglen, q);
              }
              p += sys_strlen((char *)p);
              t = 0;
              break;
            case '*':
              if (vararg) {
                arglen = m68k_read_memory_16(v_arg);
                v_arg += 2;
              } else {
                arglen = ARG16;
              }
              k++;
              break;
            case '%':
              *p++ = f[i];
              t = 0;
              break;
            default:
              fmt[j++] = f[i];
              break;
          }
          break;
      }
    }
    *p = 0;
    res = sys_strlen(s);
  }
  debug(DEBUG_TRACE, "EmuPalmOS", "StrPrintF(0x%08X \"%s\", 0x%08X \"%s\", ...): %d", str, s ? s : "", formatStr, f ? f : "", res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapFrmNewForm: {
  // FormType *FrmNewForm(UInt16 formID, const titleStrP, Coord x, Coord y, Coord width, Coord height, Boolean modal, UInt16 defaultButton, UInt16 helpRscID, UInt16 menuRscID)
  formID = ARG16;
  titleStrP = ARG32;
  x = ARG16;
  y = ARG16;
  width = ARG16;
  height = ARG16;
  modal = ARG8;
  defaultButton = ARG16;
  helpRscID = ARG16;
  menuRscID = ARG16;
  titleStr = (char *)emupalmos_trap_in(titleStrP, trap, 1);
  form = FrmNewForm(formID, titleStr, (uint16_t)x, (uint16_t)y, (uint16_t)width, (uint16_t)height, modal, defaultButton, helpRscID, menuRscID);
  formP = emupalmos_trap_out(form);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmNewForm(%d, 0x%08X [%s], %d, %d, %d, %d, %d, %d, %d, %d): 0x%08X", formID, titleStrP, titleStr, x, y, width, height, modal, defaultButton, helpRscID, menuRscID, formP);
  m68k_set_reg(M68K_REG_A0, formP);
}
break;
case sysTrapFrmInitForm: {
  // FormType *FrmInitForm(UInt16 rscID)
  rscID = ARG16;
  form = FrmInitForm(rscID);
  f32 = emupalmos_trap_out(form);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmInitForm(%d): 0x%08X", rscID, f32);
  m68k_set_reg(M68K_REG_A0, f32);
}
break;
case sysTrapFrmDeleteForm: {
  // void FrmDeleteForm(formP)
  formP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  FrmDeleteForm(form);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmDeleteForm(0x%08X)", formP);
}
break;
case sysTrapFrmGetFormId: {
  // UInt16 FrmGetFormId(const formP)
  formP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  id = FrmGetFormId(form);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetformId(0x%08X): %d", formP, (UInt16)id);
  m68k_set_reg(M68K_REG_D0, (UInt16)id);
}
break;
case sysTrapFrmGetFirstForm: {
  // FormType *FrmGetFirstForm(void)
  form = FrmGetFirstForm();
  f32 = emupalmos_trap_out(form);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFirstForm(): 0x%08X", f32);
  m68k_set_reg(M68K_REG_A0, f32);
}
break;
case sysTrapFrmGetFormPtr: {
  // FormType *FrmGetFormPtr(UInt16 formID)
  formID = ARG16;
  form = FrmGetFormPtr(formID);
  f32 = emupalmos_trap_out(form);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFormPtr(%d): 0x%08X", formID, f32);
  m68k_set_reg(M68K_REG_A0, f32);
}
break;
case sysTrapFrmGetObjectIndexFromPtr: {
  // UInt16 FrmGetObjectIndexFromPtr(const formP, objP)
  formP = ARG32;
  objP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  obj = emupalmos_trap_in(objP, trap, 1);
  res16 = FrmGetObjectIndexFromPtr(form, obj);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectIndexFromPtr(0x%08X, 0x%08X): %d", formP, objP, res16);
  m68k_set_reg(M68K_REG_D0, res16);
}
break;
case sysTrapFrmGetActiveField: {
  // FieldType *FrmGetActiveField(const FormType* formP)
  formP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  fld = FrmGetActiveField(form);
  f32 = emupalmos_trap_out(fld);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetActiveField(0x%08X): 0x%08X", formP, f32);
  m68k_set_reg(M68K_REG_A0, f32);
}
break;
case sysTrapFrmGotoForm: {
  // void FrmGotoForm(UInt16 formID)
  formID = ARG16;
  FrmGotoForm(formID);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGotoForm(%d)", formID);
}
break;
case sysTrapFrmUpdateForm: {
  // void FrmUpdateForm(UInt16 formID, UInt16 updateCode)
  formID = ARG16;
  updateCode = ARG16;
  FrmUpdateForm(formID, updateCode);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmUpdateForm(%d, %d)", formID, updateCode);
}
break;
case sysTrapFrmDrawForm: {
  // void FrmDrawForm(formP)
  formP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmDrawForm begin");
  FrmDrawForm(form);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmDrawForm(0x%08X)", formP);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmDrawForm end");
}
break;
case sysTrapFrmEraseForm: {
  // void FrmEraseForm(formP)
  formP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  FrmEraseForm(form);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmEraseForm(0x%08X)", formP);
}
break;
case sysTrapFrmVisible: {
  // Boolean FrmVisible(const formP)
  formP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  resb = FrmVisible(form);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmVisible(0x%08X): %d", formP, resb);
  m68k_set_reg(M68K_REG_D0, resb);
}
break;
case sysTrapFrmHideObject: {
  // void FrmHideObject(formP, UInt16 objIndex)
  formP = ARG32;
  index = ARG16;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  FrmHideObject(form, (uint16_t)index);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmHideObject(0x%08X, %d)", formP, (uint16_t)index);
  // XXX must handle 68K code because of gadget handler
}
break;
case sysTrapFrmShowObject: {
  // void FrmShowObject(formP, UInt16 objIndex)
  formP = ARG32;
  index = ARG16;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  FrmShowObject(form, (uint16_t)index);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmShowObject(0x%08X, %d)", formP, (uint16_t)index);
  // XXX must handle 68K code because of gadget handler
}
break;
case sysTrapFrmGetFocus: {
  // UInt16 FrmGetFocus(const formP)
  formP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  res16 = FrmGetFocus(form);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFocus(0x%08X): %d", formP, res16);
  m68k_set_reg(M68K_REG_D0, res16);
}
break;
case sysTrapFrmSetMenu: {
  // void FrmSetMenu(formP, UInt16 menuRscID)
  formP = ARG32;
  menuRscID = ARG16;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  FrmSetMenu(form, menuRscID);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetMenu(0x%08X, %d)", formP, menuRscID);
}
break;
case sysTrapFrmGetTitle: {
  // const Char *FrmGetTitle(const formP)
  formP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  title = (char *)FrmGetTitle(form);
  s32 = emupalmos_trap_out(title);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetTitle(0x%08X): 0x%08X \"%s\"", formP, s32, title ? title : "");
  m68k_set_reg(M68K_REG_A0, s32);
}
break;
case sysTrapFrmCopyTitle: {
  // void FrmCopyTitle(formP, const newTitle)
  formP = ARG32;
  newTitleP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  newTitle = (char *)emupalmos_trap_in(newTitleP, trap, 1);
  FrmCopyTitle(form, newTitle);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmCopyTitle(0x%08X, 0x%08X \"%s\")", formP, newTitleP, newTitle ? newTitle : "");
}
break;
case sysTrapFrmSetTitle: {
  // void FrmSetTitle(formP, newTitle)
  formP = ARG32;
  newTitleP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  newTitle = (char *)emupalmos_trap_in(newTitleP, trap, 1);
  FrmSetTitle(form, newTitle);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetTitle(0x%08X, 0x%08X \"%s\")", formP, newTitleP, newTitle ? newTitle : "");
}
break;
case sysTrapFrmUpdateScrollers: {
  // void FrmUpdateScrollers(formP, UInt16 upIndex, UInt16 downIndex, Boolean scrollableUp, Boolean scrollableDown)
  formP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  upIndex = ARG16;
  downIndex = ARG16;
  scrollableUp = ARG8;
  scrollableDown = ARG8;
  FrmUpdateScrollers(form, upIndex, downIndex, scrollableUp, scrollableDown);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmUpdateScrollers(0x%08X, %d, %d, %d, %d)", formP, upIndex, downIndex, scrollableUp, scrollableDown);
}
break;
case sysTrapFrmSetActiveForm: {
  // void FrmSetActiveForm(formP)
  formP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  FrmSetActiveForm(form);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetActiveForm(0x%08X)", formP);
}
break;
case sysTrapFrmSetEventHandler: {
  // void FrmSetEventHandler(formP, FormEventHandlerType *handler)
  formP = ARG32;
  handlerP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  if (form) form->m68k_handler = handlerP;
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetEventHandler(0x%08X, 0x%08X)", formP, handlerP);
}
break;
case sysTrapFrmGetEventHandler68K: { // custom trap created for use in 68K code
  // FormEventHandlerType *FrmGetEventHandler68K(formP)
  formP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  handlerP = form ? form->m68k_handler : 0;
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetEventHandler68K(0x%08X): 0x%08X", formP, handlerP);
  m68k_set_reg(M68K_REG_A0, handlerP);
}
break;
case sysTrapFrmSetGadgetHandler: {
  // void FrmSetGadgetHandler(formP, UInt16 objIndex, FormGadgetHandlerType *attrP)
  formP = ARG32;
  objIndex = ARG16;
  handlerP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  if (form) {
    gadget = FrmGetObjectPtr(form, objIndex);
    if (gadget) gadget->m68k_handler = handlerP;
  }
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetGadgetHandler(0x%08X, %d, 0x%08X)", formP, objIndex, handlerP);
}
break;
case sysTrapFrmGetGadgetData: {
  // void *FrmGetGadgetData(const formP, UInt16 objIndex)
  formP = ARG32;
  objIndex = ARG16;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  dataP = 0;
  if (form) {
    gadget = FrmGetObjectPtr(form, objIndex);
    if (gadget) dataP = gadget->m68k_data;
}
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetGadgetData(0x%08X, %d): 0x%08X", formP, objIndex, dataP);
  m68k_set_reg(M68K_REG_A0, dataP);
  }
break;
case sysTrapFrmSetGadgetData: {
  // void FrmSetGadgetData(formP, UInt16 objIndex, const void *data)
  formP = ARG32;
  objIndex = ARG16;
  dataP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  if (form) {
    gadget = FrmGetObjectPtr(form, objIndex);
    if (gadget) gadget->m68k_data = dataP;
  }
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetGadgetData(0x%08X, %d, 0x%08X)", formP, objIndex, dataP);
}
break;
case sysTrapFrmGetGadgetPtr68K: {
  // FormGadgetTypeInCallback *FrmGetGadgetPtr68k(formP, UInt16 objIndex)
  formP = ARG32;
  objIndex = ARG16;
  gadgetP = 0;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  if (form) {
    gadget = FrmGetObjectPtr(form, objIndex);
    if (gadget) {
      gadgetP = emupalmos_trap_out(gadget);
      if (FrmGetObjectType(form, objIndex) == frmGadgetObj) {
        encode_gadget(gadgetP, gadget);
      }
    }
  }
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetGadgetPtr68k(0x%08X, %d): 0x%08X", formP, objIndex, gadgetP);
  m68k_set_reg(M68K_REG_A0, gadgetP);
}
break;
case sysTrapFrmGetWindowHandle: {
  // WinHandle FrmGetWindowHandle(const formP)
  formP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  wh = FrmGetWindowHandle(form);
  w = emupalmos_trap_out(wh);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetWindowHandle(0x%08X): 0x%08X", formP, w);
  m68k_set_reg(M68K_REG_A0, w);
}
break;
case sysTrapFrmGetFormBounds: {
  // void FrmGetFormBounds(const formP, RectangleType *rP)
  formP = ARG32;
  rP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  FrmGetFormBounds(form, &rect);
  encode_rectangle(rP, &rect);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFormBounds(0x%08X, 0x%08X)", formP, rP);
}
break;
case sysTrapFrmSetObjectBounds: {
  // void FrmSetObjectBounds(formP, UInt16 objIndex, const RectangleType *bounds)
  formP = ARG32;
  objIndex = ARG16;
  rP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  decode_rectangle(rP, &rect);
  FrmSetObjectBounds(form, objIndex, &rect);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetObjectBounds(0x%08X, %d, 0x%08X)", formP, objIndex, rP);
}
break;
case sysTrapFrmGetNumberOfObjects: {
  // UInt16 FrmGetNumberOfObjects(const formP)
  formP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  res16 = FrmGetNumberOfObjects(form);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetNumberOfObjects(0x%08X): %d", formP, res16);
  m68k_set_reg(M68K_REG_D0, res16);
}
break;
case sysTrapFrmSetObjectPosition: {
  // void FrmSetObjectPosition(formP, UInt16 objIndex, Coord x, Coord y)
  formP = ARG32;
  objIndex = ARG16;
  xc = ARG16;
  yc = ARG16;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  FrmSetObjectPosition(form, objIndex, xc, yc);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetObjectPosition(0x%08X, %u, %d, %d)", formP, objIndex, xc, yc);
}
break;
case sysTrapFrmGetObjectId: {
  // UInt16 FrmGetObjectId(const formP, UInt16 objIndex)
  formP = ARG32;
  objIndex = ARG16;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  res16 = FrmGetObjectId(form, objIndex);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectId(0x%08X, %d): %d", formP, objIndex, res16);
  m68k_set_reg(M68K_REG_D0, res16);
}
break;
case sysTrapFrmGetObjectPosition: {
  // void FrmGetObjectPosition(const formP, UInt16 objIndex, Coord *x, Coord *y)
  formP = ARG32;
  objIndex = ARG16;
  xP = ARG32;
  yP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  FrmGetObjectPosition(form, objIndex, xP ? &xc : NULL, yP ? &yc : NULL);
  if (xP) m68k_write_memory_16(xP, xc);
  if (yP) m68k_write_memory_16(yP, yc);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectPosition(0x%08X, %d, 0x%08X, 0x%08X)", formP, objIndex, xP, yP);
}
break;
case sysTrapFrmGetObjectBounds: {
  // void FrmGetObjectBounds(const formP, UInt16 objIndex, RectangleType *rP)
  formP = ARG32;
  objIndex = ARG16;
  rP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  FrmGetObjectBounds(form, objIndex, &rect);
  encode_rectangle(rP, &rect);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectBounds(0x%08X, %d, 0x%08X)", formP, objIndex, rP);
}
break;
case sysTrapFrmGetControlGroupSelection: {
  // UInt16 FrmGetControlGroupSelection(const formP, UInt8 groupNum)
  formP = ARG32;
  groupNum = ARG8;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  res16 = FrmGetControlGroupSelection(form, groupNum);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetControlGroupSelection(0x%08X, %u): %u", formP, groupNum, res16);
  m68k_set_reg(M68K_REG_D0, res16);
}
break;
case sysTrapFrmGetActiveForm: {
  // FormType *FrmGetActiveForm(void)
  form = FrmGetActiveForm();
  f32 = emupalmos_trap_out(form);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetActiveForm(): 0x%08X", f32);
  m68k_set_reg(M68K_REG_A0, f32);
}
break;
case sysTrapFrmGetActiveFormID: {
  // UInt16 FrmGetActiveFormId(void)
  id = FrmGetActiveFormID();
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetActiveFormID(): %d", (UInt16)id);
  m68k_set_reg(M68K_REG_D0, (UInt16)id);
}
break;
case sysTrapFrmGetObjectIndex: {
  // UInt16 FrmGetObjectIndex(const formP, UInt16 objID)
  formP = ARG32;
  objID = ARG16;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  index = FrmGetObjectIndex(form, objID);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectIndex(0x%08X, %d): %d", formP, objID, (UInt16)index);
  m68k_set_reg(M68K_REG_D0, (UInt16)index);
}
break;
case sysTrapFrmGetObjectPtr: {
  // void *FrmGetObjectPtr(const formP, UInt16 objIndex)
  formP = ARG32;
  objIndex = ARG16;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  p = FrmGetObjectPtr(form, objIndex);
  ptr = emupalmos_trap_out(p);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectPtr(0x%08X, %d): 0x%08X", formP, objIndex, ptr);
  m68k_set_reg(M68K_REG_A0, ptr);
}
break;
case sysTrapFrmGetObjectType: {
  // FormObjectKind FrmGetObjectType(const formP, UInt16 objIndex)
  formP = ARG32;
  objIndex = ARG16;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  FormObjectKind objType = FrmGetObjectType(form, objIndex);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectType(0x%08X, %d): %d", formP, objIndex, objType);
  m68k_set_reg(M68K_REG_D0, objType);
}
break;
case sysTrapFrmGetLabel: {
  // const Char *FrmGetLabel(const formP, UInt16 labelID)
  formP = ARG32;
  labelID = ARG16;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  label = FrmGetLabel(form, labelID);
  a = emupalmos_trap_out((void *)label);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetLabel(0x%08X, %d): 0x%08X", formP, labelID, a);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapFrmSetFocus: {
  // void FrmSetFocus(formP, UInt16 fieldIndex)
  formP = ARG32;
  fieldIndex = ARG16;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  FrmSetFocus(form, fieldIndex);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetFocus(0x%08X, %d)", formP, fieldIndex);
}
break;
case sysTrapFrmGetControlValue: {
  // Int16 FrmGetControlValue(const formP, UInt16 objIndex)
  formP = ARG32;
  objIndex = ARG16;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  value = (uint32_t)FrmGetControlValue(form, objIndex);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetControlValue(0x%08X, %d): %d", formP, objIndex, (Int16)value);
  m68k_set_reg(M68K_REG_D0, (Int16)value);
}
break;
case sysTrapFrmSetControlValue: {
  // void FrmSetControlValue(const formP, UInt16 objIndex, Int16 newValue)
  formP = ARG32;
  objIndex = ARG16;
  newValue = ARG16;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  FrmSetControlValue(form, objIndex, (int16_t)newValue);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetControlValue(0x%08X, %d, %d)", formP, objIndex, newValue);
}
break;
case sysTrapFrmSetControlGroupSelection: {
  // void FrmSetControlGroupSelection(const formP, UInt8 groupNum, UInt16 controlID)
  formP = ARG32;
  groupNum = ARG8;
  controlID = ARG16;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  FrmSetControlGroupSelection(form, groupNum, controlID);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetControlGroupSelection(0x%08X, %d, %d)", formP, groupNum, controlID);
}
break;
case sysTrapFrmDispatchEvent: {
  // Boolean FrmDispatchEvent(EventType *eventP)
  eventP = ARG32;

  if (eventP) decode_event(eventP, &l_event);
  resb = FrmDispatchEvent(&l_event);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmDispatchEvent(0x%08X): %d", eventP, resb);
  m68k_set_reg(M68K_REG_D0, resb);
}
break;
case sysTrapFrmHandleEvent: {
  // Boolean FrmHandleEvent(formP, EventType *eventP)
  formP = ARG32;
  eventP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);

  if (eventP) decode_event(eventP, &l_eventP);
  resb = FrmHandleEvent(form, &l_eventP);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmHandleEvent(0x%08X, 0x%08X): %d", formP, eventP, resb);
  m68k_set_reg(M68K_REG_D0, resb);
}
break;
case sysTrapFrmCopyLabel: {
  // void FrmCopyLabel(formP, UInt16 labelID, const Char *newLabel)
  formP = ARG32;
  labelID = ARG16;
  newLabelP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  s = (char *)emupalmos_trap_in(newLabelP, trap, 1);
  FrmCopyLabel(form, labelID, s);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmCopyLabel(0x%08X, %d, 0x%08X \"%s\")", formP, labelID, newLabelP, s ? s : "");
}
break;
case sysTrapFrmSaveAllForms:
  // void FrmSaveAllForms(void)
  FrmSaveAllForms();
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmSaveAllForms()");
break;
case sysTrapFrmCloseAllForms:
  // void FrmCloseAllForms(void)
  FrmCloseAllForms();
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmCloseAllForms()");
break;
case sysTrapFrmPopupForm: {
  // void FrmPopupForm(UInt16 formID)
  formID = ARG16;
  FrmPopupForm(formID);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmPopupForm(%d)", formID);
}
break;
case sysTrapFrmDoDialog: {
  // UInt16 FrmDoDialog(formP)
  formP = ARG32;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  res16 = FrmDoDialog(form);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmDoDialog(0x%08X): %d", formP, res16);
  m68k_set_reg(M68K_REG_D0, res16);
}
break;
case sysTrapFrmReturnToForm: {
  // void FrmReturnToForm(UInt16 formID)
  formID = ARG16;
  FrmReturnToForm(formID);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmReturnToForm(%d)", formID);
}
break;
case sysTrapFrmHelp: {
  // void FrmHelp(UInt16 helpMsgId)
  helpMsgId = ARG16;
  FrmHelp(helpMsgId);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmHelp(%d)", helpMsgId);
}
break;
case sysTrapAbtShowAbout: {
  // void AbtShowAbout(UInt32 creator)
  creator = ARG32;
  AbtShowAbout(creator);
  debug(DEBUG_TRACE, "EmuPalmOS", "AbtShowAbout(%d)", creator);
}
break;
case sysTrapFrmCustomAlert: {
  // UInt16 FrmCustomAlert(UInt16 alertId, const s1, const s2, const s3)
  alertId = ARG16;
  s1P = ARG32;
  s2P = ARG32;
  s3P = ARG32;
  s1 = (char *)emupalmos_trap_in(s1P, trap, 1);
  s2 = (char *)emupalmos_trap_in(s2P, trap, 2);
  s3 = (char *)emupalmos_trap_in(s3P, trap, 3);
  res16 = FrmCustomAlert(alertId, s1, s2, s3);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmCustomAlert(%d, 0x%08X, 0x%08X, 0x%08X): %d", alertId, s1P, s2P, s3P, res16);
  m68k_set_reg(M68K_REG_D0, res16);
}
break;
case sysTrapFrmCustomResponseAlert: {
  // UInt16 FrmCustomResponseAlert(UInt16 alertId, const s1, const s2, const s3, entryStringBuf, Int16 entryStringBufLength, FormCheckResponseFuncPtr callback)
  alertId = ARG16;
  s1P = ARG32;
  s2P = ARG32;
  s3P = ARG32;
  entryStringBufP = ARG32;
  entryStringBufLength = ARG16;
  callbackP = ARG32;
  s1 = (char *)emupalmos_trap_in(s1P, trap, 1);
  s2 = (char *)emupalmos_trap_in(s2P, trap, 2);
  s3 = (char *)emupalmos_trap_in(s3P, trap, 3);
  entryStringBuf = (char *)emupalmos_trap_in(entryStringBufP, trap, 4);
  callbackfp = (FormCheckResponseFuncPtr)emupalmos_trap_in(callbackP, trap, 6);
  res16 = FrmCustomResponseAlert(alertId, s1, s2, s3, entryStringBuf, entryStringBufLength, callbackfp);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmCustomResponseAlert(%d, 0x%08X, 0x%08X, 0x%08X, 0x%08X, %d, 0x%08X): %d", alertId, s1P, s2P, s3P, entryStringBufP, entryStringBufLength, callbackP, res16);
  m68k_set_reg(M68K_REG_D0, res16);
}
break;
case sysTrapFrmAlert: {
  // UInt16 FrmAlert(UInt16 alertId)
  alertId = ARG16;
  res16 = FrmAlert(alertId);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmAlert(%d): %d", alertId, res16);
  m68k_set_reg(M68K_REG_D0, res16);
}
break;
case sysTrapFrmNewBitmap: {
  // FormBitmapType *FrmNewBitmap(FormType **formPP, UInt16 ID, UInt16 rscID, Coord x, Coord y)
  formPP = ARG32;
  id = ARG16;
  rscID = ARG16;
  x = ARG16;
  y = ARG16;
  formP = formPP ? m68k_read_memory_32(formPP) : 0;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  Formbitmap = FrmNewBitmap(&form, (uint16_t)id, rscID, (uint16_t)x, (uint16_t)y);
  a = emupalmos_trap_out(Formbitmap);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmNewBitmap(0x%08X, %u, %u, %d, %d): 0x%08X", formPP, id, rscID, x, y, a);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapFrmNewGadget: {
  // FormGadgetType *FrmNewGadget(FormType **formPP, UInt16 id, Coord x, Coord y, Coord width, Coord height)
  formPP = ARG32;
  id = ARG16;
  x = ARG16;
  y = ARG16;
  width = ARG16;
  height = ARG16;
  formP = formPP ? m68k_read_memory_32(formPP) : 0;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  gadget = FrmNewGadget(&form, (uint16_t)id, (uint16_t)x, (uint16_t)y, (uint16_t)width, (uint16_t)height);
  a = emupalmos_trap_out(gadget);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmNewGadget(0x%08X, %u, %d, %d, %d, %d): 0x%08X", formPP, id, x, y, width, height, a);
  m68k_set_reg(M68K_REG_A0, a);
  }
break;
case sysTrapFrmActiveState: {
  // Err FrmActiveState(FormActiveStateType *stateP, Boolean save)
  stateP = ARG32;
  save = ARG8;
  statef = (FormActiveStateType *)emupalmos_trap_in(stateP, trap, 0);
  err = FrmActiveState(statef, save);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmActiveState(0x%08X, %d)", stateP, save);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapFrmNewGsi: {
  // FrmGraffitiStateType *FrmNewGsi(FormType **formPP, Coord x, Coord y)
  formPP = ARG32;
  x = ARG16;
  y = ARG16;
  formP = formPP ? m68k_read_memory_32(formPP) : 0;
  form = (FormType *)emupalmos_trap_in(formP, trap, 0);
  FrmGraffitiStateType *gsi = FrmNewGsi(&form, (uint16_t)x, (uint16_t)y);
  a = emupalmos_trap_out(gsi);
  debug(DEBUG_TRACE, "EmuPalmOS", "FrmNewGsi(0x%08X, %d, %d): 0x%08X", formPP, x, y, a);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapCtlNewControl: {
  // ctlNewControl(void **formPP, UInt16 ID, ControlStyleType style, const textP, Coord x, Coord y, Coord width, Coord height, FontID font, UInt8 group, Boolean leftAnchor)
  formPP = ARG32;
  id = ARG16;
  style = ARG8;
  textP = ARG32;
  x = ARG16;
  y = ARG16;
  width = ARG16;
  height = ARG16;
  font = ARG8;
  group = ARG8;
  leftAnchor = ARG8;
  formP = formPP ? m68k_read_memory_32(formPP) : 0;
  formv = emupalmos_trap_in(formP, trap, 0);
  text = (char *)emupalmos_trap_in(textP, trap, 3);
  s_ctlP = CtlNewControl(&formv, (uint16_t)id, style, text, (int16_t)x, (int16_t)y, (uint16_t)width, (uint16_t)height, font, group, leftAnchor);
  a = emupalmos_trap_out(s_ctlP);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlNewControl(0x%08X, %u, %d, 0x%08X [%s], %d, %d, %d, %d, %d, %d, %d): 0x%08X", formPP, id, style, textP, text, x, y, width, height, font, group, leftAnchor, a);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapCtlGetStyle68K: { // custom trap created for use in 68K code
  // ControlStyleType CtlGetStyle(controlP)
  controlP = ARG32;
  control = (ControlType *)emupalmos_trap_in(controlP, trap, 0);
  stylest = control ? control->style : 0;
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlGetStyle(0x%08X): %d", controlP, stylest);
  m68k_set_reg(M68K_REG_D0, stylest);
}
break;
case sysTrapCtlGetLabel: {
  // const Char *CtlGetLabel(controlP)
  controlP = ARG32;
  s_controlP = (ControlType *)emupalmos_trap_in(controlP, trap, 0);
  resc = (Char *)CtlGetLabel(s_controlP);
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlGetLabel(controlP=0x%08X): 0x%08X", controlP, r_res);
}
break;
case sysTrapLstSetDrawFunction: {
  // void LstSetDrawFunction(listP, ListDrawDataFuncPtr func)
  listP = ARG32;
  funcP = ARG32;
  list = (ListType *)emupalmos_trap_in(listP, trap, 0);
  emupalmos_trap_in(funcP, trap, 1);
  if (list) list->m68k_drawfunc = funcP;
  debug(DEBUG_TRACE, "EmuPalmOS", "LstSetDrawFunction(0x%08X, 0x%08X)", listP, funcP);
}
break;
case sysTrapLstDrawList: {
  // void LstDrawList(listP)
  listP = ARG32;
  list = (ListType *)emupalmos_trap_in(listP, trap, 0);
  LstDrawList(list);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstDrawList(0x%08X)", listP);
  }
break;
case sysTrapTblSetCustomDrawProcedure: {
  // void TblSetCustomDrawProcedure(TableType *tableP, Int16 column, TableDrawItemFuncPtr drawCallback)
  tableP = ARG32;
  column = ARG16;
  funcP = ARG32;
  tablet = (TableType *)emupalmos_trap_in(tableP, trap, 0);
  emupalmos_trap_in(funcP, trap, 2);
  if (tablet && column >= 0 && column < tablet->numColumns) {
    tablet->columnAttrs[column].m68k_drawfunc = funcP;
  }
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetCustomDrawProcedure(0x%08X, %d, 0x%08X)", tableP, column, funcP);
}
break;
case sysTrapTblSetLoadDataProcedure: {
  // void TblSetLoadDataProcedure(TableType *tableP, Int16 column, TableLoadDataFuncPtr loadDataCallback)
  tableP = ARG32;
  column = ARG16;
  funcP = ARG32;
  tablet = (TableType *)emupalmos_trap_in(tableP, trap, 0);
  emupalmos_trap_in(funcP, trap, 2);
  if (tablet && column >= 0 && column < tablet->numColumns) {
    tablet->columnAttrs[column].m68k_loadfunc = funcP;
  }
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetLoadDataProcedure(0x%08X, %d, 0x%08X)", tableP, column, funcP);
}
break;
case sysTrapTblSetSaveDataProcedure: {
  // void TblSetSaveDataProcedure(TableType *tableP, Int16 column, TableSaveDataFuncPtr saveDataCallback)
  tableP = ARG32;
  column = ARG16;
  funcP = ARG32;
  tablet = (TableType *)emupalmos_trap_in(tableP, trap, 0);
  emupalmos_trap_in(funcP, trap, 2);
  if (tablet && column >= 0 && column < tablet->numColumns) {
    tablet->columnAttrs[column].m68k_savefunc = funcP;
  }
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetSaveDataProcedure(0x%08X, %d, 0x%08X)", tableP, column, funcP);
}
break;
case sysTrapSclSetScrollBar: {
  // void SclSetScrollBar(bar, Int16 value, Int16 min, Int16 max, Int16 pageSize)
  barP = ARG32;
  value16 = ARG16;
  min = ARG16;
  max = ARG16;
  pageSize = ARG16;
  bar = (ScrollBarType *)emupalmos_trap_in(barP, trap, 0);
  SclSetScrollBar(bar, value16, min, max, pageSize);
  debug(DEBUG_TRACE, "EmuPalmOS", "SclSetScrollBar(0x%08X, %d, %d, %d, %d)", barP, value16, min, max, pageSize);
}
break;
case sysTrapSclGetScrollBar: {
  // void SclGetScrollBar(bar, Int16 *valueP, Int16 *minP, Int16 *maxP, Int16 *pageSizeP)
  barP = ARG32;
  valueP = ARG32;
  minP = ARG32;
  maxP = ARG32;
  pageSizeP = ARG32;
  bar = (ScrollBarType *)emupalmos_trap_in(barP, trap, 0);
  emupalmos_trap_in(valueP, trap, 1);
  emupalmos_trap_in(minP, trap, 2);
  emupalmos_trap_in(maxP, trap, 3);
  emupalmos_trap_in(pageSizeP, trap, 4);
  SclGetScrollBar(bar, (Int16 *)&value16, &min, &max, &pageSize);
  if (valueP) m68k_write_memory_16(valueP, (Int16)value16);
  if (minP) m68k_write_memory_16(minP, min);
  if (maxP) m68k_write_memory_16(maxP, max);
  if (pageSizeP) m68k_write_memory_16(pageSizeP, pageSize);
}
break;
case sysTrapSclDrawScrollBar: {
  // void SclDrawScrollBar(bar)
  barP = ARG32;
  bar = (ScrollBarType *)emupalmos_trap_in(barP, trap, 0);
  SclDrawScrollBar(bar);
}
break;
case sysTrapSclHandleEvent: {
  // Boolean SclHandleEvent(bar, EventType *event)
  barP = ARG32;
  eventP = ARG32;

  if (eventP) decode_event(eventP, &l_eventP);
  bar = (ScrollBarType *)emupalmos_trap_in(barP, trap, 0);
  resb = SclHandleEvent(bar, &l_eventP);
  m68k_set_reg(M68K_REG_D0, resb);
}
break;
case sysTrapEvtEnableGraffiti: {
  // void EvtEnableGraffiti(Boolean enable)
  enable = ARG8;
  EvtEnableGraffiti(enable);
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtEnableGraffiti(%d)", enable);
}
break;
case sysTrapEvtResetAutoOffTimer: {
  // Err EvtResetAutoOffTimer(void)
  err = EvtResetAutoOffTimer();
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtResetAutoOffTimer(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapEvtAddUniqueEventToQueue: {
  // void EvtAddUniqueEventToQueue(const EventType *eventP, UInt32 id, Boolean inPlace)
  eventP = ARG32;
  id = ARG32;
  inPlace = ARG8;
  emupalmos_trap_in(eventP, trap, 0);

  if (eventP) decode_event(eventP, &l_eventP);
  EvtAddUniqueEventToQueue(eventP ? &l_eventP : NULL, id, inPlace);
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtAddUniqueEventToQueue(0x%08X [0x%04X], %d, %d)", eventP, l_eventP.eType, id, inPlace);
}
break;
case sysTrapEvtAddEventToQueue: {
  // void EvtAddEventToQueue(const EventType *event)
  eventP = ARG32;
  emupalmos_trap_in(eventP, trap, 0);

  if (eventP) decode_event(eventP, &l_eventP);
  EvtAddEventToQueue(eventP ? &l_eventP : NULL);
  eventName = EvtGetEventName(l_eventP.eType);
  if (eventName) {
    debug(DEBUG_TRACE, "EmuPalmOS", "EvtAddEventToQueue(0x%08X [%s])", eventP, eventName);
  } else {
    debug(DEBUG_TRACE, "EmuPalmOS", "EvtAddEventToQueue(0x%08X [0x%04X])", eventP, l_eventP.eType);
  }
  if (eventP) encode_event(eventP, &l_eventP);
}
break;
case sysTrapEvtEnqueueKey: {
  // Err EvtEnqueueKey(WChar ascii, UInt16 keycode, UInt16 modifiers)
  ascii = ARG16;
  keycode = ARG16;
  modifiers = ARG16;
  err = EvtEnqueueKey(ascii, keycode, modifiers);
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtEnqueueKey(0x%04X, 0x%04X, 0x%04X): %d", ascii, keycode, modifiers, err);
}
break;
case sysTrapEvtEventAvail: {
  // Boolean EvtEventAvail(void)
  resb = EvtEventAvail();
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtEventAvail(): %d", resb);
  m68k_set_reg(M68K_REG_D0, resb);
}
break;
case sysTrapEvtWakeup: {
  // Err EvtWakeup(void)
  err = EvtWakeup();
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtWakeup(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapEvtGetEvent: {
  // void EvtGetEvent(EventType *event, Int32 timeout)
  eventP = ARG32;
  timeout = ARG32;
  timeout = pumpkin_event_timeout(timeout);
  emupalmos_trap_in(eventP, trap, 0);

  MemSet(&l_eventP, sizeof(EventType), 0);
  EvtGetEvent(eventP ? &l_eventP : NULL, timeout);
  eventName = EvtGetEventName(l_eventP.eType);
  if (eventName) {
    debug(DEBUG_TRACE, "EmuPalmOS", "EvtGetEvent(0x%08X [%s], %d)", eventP, eventName, timeout);
  } else {
    debug(DEBUG_TRACE, "EmuPalmOS", "EvtGetEvent(0x%08X [0x%04X], %d)", eventP, l_eventP.eType, timeout);
  }
  if (eventP) encode_event(eventP, &l_eventP);
}
break;
case sysTrapEvtCopyEvent: {
  // void EvtCopyEvent(const EventType *source, EventType *dest)
  sourceP = ARG32;
  destP = ARG32;
  emupalmos_trap_in(sourceP, trap, 0);
  emupalmos_trap_in(destP, trap, 1);
  if (sourceP) decode_event(sourceP, &source);
  EvtCopyEvent(&source, &dest);
  if (destP) encode_event(destP, &dest);
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtCopyEvent(0x%08X [0x%04X], 0x%08X)", sourceP, source.eType, destP);
}
break;
case sysTrapPenResetCalibration: {
  // Err PenResetCalibration(void)
  err = PenResetCalibration();
  debug(DEBUG_TRACE, "EmuPalmOS", "PenResetCalibration(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapPenCalibrate: {
  // Err PenCalibrate(PointType *digTopLeftP, PointType *digBotRightP, PointType *scrTopLeftP, PointType *scrBotRightP)
  err = PenCalibrate(NULL, NULL, NULL, NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "PenCalibrate %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapPenSleep: {
  // Err PenSleep(void)
  err = PenSleep();
  debug(DEBUG_TRACE, "EmuPalmOS", "PenSleep(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapPenWake: {
  // Err PenWake(void)
  err = PenWake();
  debug(DEBUG_TRACE, "EmuPalmOS", "PenWake(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapEvtGetPen: {
  // void EvtGetPen(Int16 *pScreenX, Int16 *pScreenY, Boolean *pPenDown)
  pScreenX = ARG32;
  pScreenY = ARG32;
  pPenDown = ARG32;
  emupalmos_trap_in(pScreenX, trap, 0);
  emupalmos_trap_in(pScreenY, trap, 1);
  emupalmos_trap_in(pPenDown, trap, 2);
  EvtGetPen(pScreenX ? &screenX : NULL, pScreenY ? &screenY : NULL, pPenDown ? &penDown : NULL);
  if (pScreenX) m68k_write_memory_16(pScreenX, screenX);
  if (pScreenY) m68k_write_memory_16(pScreenY, screenY);
  if (pPenDown) m68k_write_memory_8(pPenDown, penDown);
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtGetPen(0x%08X, 0x%08X, 0x%08X)", pScreenX, pScreenY, pPenDown);
}
break;
case sysTrapEvtSysEventAvail: {
  // Boolean EvtSysEventAvail(ignorePenUps)
  ignorePenUps = ARG8;
  resb = EvtSysEventAvail(ignorePenUps);
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtSysEventAvail(): %d", resb);
  m68k_set_reg(M68K_REG_D0, resb);
}
break;
case sysTrapEvtFlushKeyQueue:
  // Err EvtFlushKeyQueue(void)
  err = EvtFlushKeyQueue();
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtFlushKeyQueue(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
break;
case sysTrapEvtFlushPenQueue:
  // Err EvtFlushPenQueue(void)
  err = EvtFlushPenQueue();
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtFlushPenQueue(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
break;
case sysTrapEvtSetNullEventTick: {
  // Boolean EvtSetNullEventTick(tick)
  tick = ARG32;
  resb = EvtSetNullEventTick(tick);
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtSetNullEventTick(%u): %d", tick, resb);
  m68k_set_reg(M68K_REG_D0, resb);
}
break;
case sysTrapEvtFlushNextPenStroke:
  // Err EvtFlushNextPenStroke(void)
  err = EvtFlushNextPenStroke();
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtFlushNextPenStroke(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
break;
case sysTrapEvtKeyQueueEmpty: {
  // Boolean EvtKeyQueueEmpty(void)
  resb = EvtKeyQueueEmpty();
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtKeyQueueEmpty(): %d", resb);
  m68k_set_reg(M68K_REG_D0, resb);
}
break;
case sysTrapClipboardAddItem: {
  // void ClipboardAddItem(const ClipboardFormatType format, const void *ptr, UInt16 length)
  format = ARG8;
  ptrP = ARG32;
  length = ARG16;
  ptrv = emupalmos_trap_in(ptrP, trap, 1);
  ClipboardAddItem((uint8_t)format, ptrv, length);
  debug(DEBUG_TRACE, "EmuPalmOS", "ClipboardAddItem(%d, 0x%08X, %d)", format, ptrP, length);
}
break;
case sysTrapClipboardGetItem: {
  // MemHandle ClipboardGetItem(const ClipboardFormatType format, UInt16 *length)
  format = ARG8;
  lengthP = ARG32;
  emupalmos_trap_in(lengthP, trap, 1);
  hm = ClipboardGetItem((uint8_t)format, &length);
  r = emupalmos_trap_out(hm);
  debug(DEBUG_TRACE, "EmuPalmOS", "ClipboardGetItem(%d, 0x%08X): 0x%08X", format, lengthP, r);
  m68k_set_reg(M68K_REG_A0, r);
}
break;
case sysTrapExgInit:
case sysTrapExgConnect:
case sysTrapExgPut:
case sysTrapExgGet:
case sysTrapExgAccept:
case sysTrapExgDisconnect:
case sysTrapExgRegisterData:
case sysTrapExgNotifyReceiveV35:
case sysTrapExgDBRead:
case sysTrapExgDBWrite:
case sysTrapExgDoDialog:
case sysTrapExgRegisterDatatype:
case sysTrapExgNotifyReceive:
case sysTrapExgNotifyGoto:
case sysTrapExgRequest:
case sysTrapExgSetDefaultApplication:
case sysTrapExgGetDefaultApplication:
case sysTrapExgGetTargetApplication:
case sysTrapExgGetRegisteredApplications:
case sysTrapExgGetRegisteredTypes:
case sysTrapExgNotifyPreview:
case sysTrapExgControl:
  m68k_set_reg(M68K_REG_D0, sysErrParamErr);
break;
case sysTrapExgSend:
case sysTrapExgReceive:
  m68k_set_reg(M68K_REG_D0, 0);
break;
case sysTrapEvtGetSilkscreenAreaList:
case sysTrapEvtGetPenBtnList: {
  // const SilkscreenAreaType *EvtGetSilkscreenAreaList(UInt16* numAreas)
  // const PenBtnInfoType *EvtGetPenBtnList(UInt16* numButtons)
  numP = ARG32;
  emupalmos_trap_in(numP, trap, 0);
  if (numP) m68k_write_memory_16(numP, 0);
  m68k_set_reg(M68K_REG_A0, 0);
}
break;
case sysTrapSysSetAutoOffTime:
  m68k_set_reg(M68K_REG_D0, 0);
break;
case sysTrapSysCreateDataBaseList: {
  // Boolean SysCreateDataBaseList(UInt32 type, UInt32 creator, UInt16 *dbCount, MemHandle *dbIDs, Boolean lookupName)
  type = ARG32;
  creator = ARG32;
  countP = ARG32;
  listP = ARG32;
  lookupName = ARG8;
  emupalmos_trap_in(countP, trap, 2);
  emupalmos_trap_in(listP, trap, 3);
  resb = SysCreateDataBaseList68K(type, creator, &wCount, &listm, lookupName);
  if (countP) m68k_write_memory_16(countP, wCount);
  if (listP) m68k_write_memory_32(listP, emupalmos_trap_out(listm));
  debug(DEBUG_TRACE, "EmuPalmOS", "SysCreateDataBaseList(0x%08X, 0x%08X, 0x%08X, 0x%08X, %d)", type, creator, countP, listP, lookupName);
  m68k_set_reg(M68K_REG_D0, resb);
}
break;
case sysTrapUIPickColor: {
  // Boolean UIPickColor(IndexedColorType *indexP, RGBColorType *rgbP, UIPickColorStartType start, const titleP, const tipP)
  indexP = ARG32;
  rgbP = ARG32;
  start = ARG16;
  titleP = ARG32;
  tipP = ARG32;
  emupalmos_trap_in(indexP, trap, 0);
  emupalmos_trap_in(rgbP, trap, 1);
  if (indexP) indexc = m68k_read_memory_8(indexP);
  decode_rgb(rgbP, &rgb);
  title = (char *)emupalmos_trap_in(titleP, trap, 3);
  tip = (char *)emupalmos_trap_in(tipP, trap, 4);
  resb = UIPickColor(indexP ? &indexc : NULL, rgbP ? &rgb : NULL, (uint16_t)start, title, tip);
  if (indexP) m68k_write_memory_8(indexP, indexc);
  encode_rgb(rgbP, &rgb);
  debug(DEBUG_TRACE, "EmuPalmOS", "UIPickColor(indexP=0x%08X, rgbP=0x%08X, start=%d, title=%s, tip=%s)", indexP, rgbP, start, title ? title : "", tip ? tip : "");
  m68k_set_reg(M68K_REG_D0, resb);
}
break;
case sysTrapUIBrightnessAdjust:
  // void UIBrightnessAdjust(void)
  UIBrightnessAdjust();
  debug(DEBUG_TRACE, "EmuPalmOS", "UIBrightnessAdjust()");
break;
case sysTrapUIContrastAdjust:
  // void UIContrastAdjust(void)
  UIContrastAdjust();
  debug(DEBUG_TRACE, "EmuPalmOS", "UIContrastAdjust()");
break;
case sysTrapLocGetNumberSeparators: {
  // void LocGetNumberSeparators(NumberFormatType numberFormat, Char *thousandSeparator, Char *decimalSeparator)
  numberFormat = ARG8;
  thousandSeparatorP = ARG32;
  decimalSeparatorP = ARG32;
  emupalmos_trap_in(thousandSeparatorP, trap, 0);
  emupalmos_trap_in(decimalSeparatorP, trap, 1);
  LocGetNumberSeparators(numberFormat, &thousandSeparator, &decimalSeparator);
  if (thousandSeparatorP) m68k_write_memory_8(thousandSeparatorP, thousandSeparator);
  if (decimalSeparatorP) m68k_write_memory_8(decimalSeparatorP, decimalSeparator);
  debug(DEBUG_TRACE, "EmuPalmOS", "LocGetNumberSeparators(%d, %u, %u)", numberFormat, thousandSeparatorP, decimalSeparatorP);
}
break;
case sysTrapSndPlaySmf: {
  // Err SndPlaySmf(void *chanP, SndSmfCmdEnum cmd, UInt8 *smfP, SndSmfOptionsType *selP, SndSmfChanRangeType *chanRangeP, SndSmfCallbacksType *callbacksP, Boolean bNoWait)
  chanP = ARG32;
  cmd = ARG8;
  smfP = ARG32;
  selP = ARG32;
  chanRangeP = ARG32;
  callbacksP = ARG32;
  bNoWait = ARG8;
  emupalmos_trap_in(chanP, trap, 0);
  emupalmos_trap_in(selP, trap, 3);
  emupalmos_trap_in(chanRangeP, trap, 4);
  emupalmos_trap_in(callbacksP, trap, 5);
  SndSmfOptionsType options;
  decode_smfoptions(selP, &options);
  res = SndPlaySmf(NULL, (uint8_t)cmd, (UInt8 *)emupalmos_trap_in(smfP, trap, 2), selP ? &options : NULL, NULL, NULL, bNoWait);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndPlaySmf(0x%08X, %d, 0x%08X, 0x%08X, 0x%08X, 0x%08X, %d): %d", chanP, (uint8_t)cmd, smfP, selP, chanRangeP, callbacksP, bNoWait, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapSndPlaySmfResource: {
  //Err SndPlaySmfResource(UInt32 resType, Int16 resID, SystemPreferencesChoice volumeSelector)
  resType = ARG32;
  resID = ARG32;
  volumeSelector = ARG8;
  res = SndPlaySmfResource(resType, resID, volumeSelector);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndPlaySmfResource(0x%08X, %d, %d): %d", resType, resID, volumeSelector, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapSndCreateMidiList: {
  // Boolean SndCreateMidiList(UInt32 creator, Boolean multipleDBs, UInt16 *wCountP, MemHandle *entHP)
  creator = ARG32;
  multipleDBs = ARG8;
  wCountP = ARG32;
  entHP = ARG32;
  emupalmos_trap_in(wCountP, trap, 2);
  emupalmos_trap_in(entHP, trap, 3);
  UInt16 wCount;
  MemHandle entH;
  resb = SndCreateMidiList(creator, multipleDBs, wCountP ? &wCount : NULL, entHP ? &entH : NULL);
  if (wCountP) m68k_write_memory_16(wCountP, wCount);
  if (entHP) m68k_write_memory_32(entHP, emupalmos_trap_out(entH));
  pumpkin_id2s(creator, buf);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndCreateMidiList('%s', %d, 0x%08X, 0x%08X): %d", buf, multipleDBs, wCountP, entHP, resb);
  m68k_set_reg(M68K_REG_D0, resb);
}
break;
case sysTrapSndPlaySystemSound: {
  // void SndPlaySystemSound(SndSysBeepType beepID)
  uint8_t beepID = ARG8;
  SndPlaySystemSound(beepID);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndSysBeepType(%d)", beepID);
}
break;
case sysTrapSndPlayResource: {
  // Err SndPlayResource(SndPtr sndP, Int32 volume, UInt32 flags)
  uint32_t sndP = ARG32;
  int32_t volume = ARG32;
  uint32_t flags = ARG32;
  void *sndPtr = (void *)emupalmos_trap_in(sndP, trap, 0);
  res = SndPlayResource(sndPtr, volume, flags);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndPlayResource(0x%08X, %d, 0x%08X): %d", sndP, volume, flags, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapSysKeyboardDialogV10: {
  // void SysKeyboardDialogV10(void)
  SysKeyboardDialogV10();
  debug(DEBUG_TRACE, "EmuPalmOS", "SysKeyboardDialogV10()");
}
break;
case sysTrapSysKeyboardDialog: {
  // void SysKeyboardDialog(KeyboardType kbd)
  kbd = ARG8;
  SysKeyboardDialog(kbd);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysKeyboardDialog(%d)", kbd);
}
break;
case sysTrapSndDoCmd: {
  // Err SndDoCmd(void *channelP, SndCommandPtr cmdP, Boolean noWait)
  channelP = ARG32;
  cmdP = ARG32;
  noWait = ARG8;
  emupalmos_trap_in(channelP, trap, 0);
  SndCommandType cmd;
  decode_sndcmd(cmdP, &cmd);
  err = SndDoCmd(NULL, cmdP ? &cmd : NULL, noWait);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndDoCmd(0x%08X, 0x%08X, %d): %d", channelP, cmdP, noWait, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSndGetDefaultVolume: {
  // void SndGetDefaultVolume(UInt16 *alarmAmpP, UInt16 *sysAmpP, UInt16 *masterAmpP)
  alarmAmpP = ARG32;
  sysAmpP = ARG32;
  masterAmpP = ARG32;
  emupalmos_trap_in(alarmAmpP, trap, 0);
  emupalmos_trap_in(sysAmpP, trap, 1);
  emupalmos_trap_in(masterAmpP, trap, 2);
  SndGetDefaultVolume(alarmAmpP ? &alarmAmp : NULL, sysAmpP ? &sysAmp : NULL, masterAmpP ? &masterAmp : NULL);
  if (alarmAmpP) m68k_write_memory_16(alarmAmpP, alarmAmp);
  if (sysAmpP) m68k_write_memory_16(sysAmpP, sysAmp);
  if (masterAmpP) m68k_write_memory_16(masterAmpP, masterAmp);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndGetDefaultVolume(0x%08X, 0x%08X, 0x%08X)", alarmAmpP, sysAmpP, masterAmpP);
}
break;
case sysTrapSndSetDefaultVolume: {
  // void SndSetDefaultVolume(UInt16 *alarmAmpP, UInt16 *sysAmpP, UInt16 *defAmpP)
  alarmAmpP = ARG32;
  sysAmpP = ARG32;
  defAmpP = ARG32;
  emupalmos_trap_in(alarmAmpP, trap, 0);
  emupalmos_trap_in(sysAmpP, trap, 1);
  emupalmos_trap_in(defAmpP, trap, 2);
  alarmAmp = alarmAmpP ? m68k_read_memory_16(alarmAmpP) : 0;
  sysAmp = sysAmpP ? m68k_read_memory_16(sysAmpP) : 0;
  defAmp = defAmpP ? m68k_read_memory_16(defAmpP) : 0;
  SndSetDefaultVolume(alarmAmpP ? &alarmAmp : NULL, sysAmpP ? &sysAmp : NULL, defAmpP ? &defAmp : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndSetDefaultVolume(0x%08X, 0x%08X, 0x%08X)", alarmAmpP, sysAmpP, defAmpP);
}
break;
case sysTrapSndStreamCreate: {
  // Err SndStreamCreate(SndStreamRef *channel, SndStreamMode mode, UInt32 samplerate, SndSampleType type, SndStreamWidth width, func, userData, UInt32 buffsize, Boolean armNative)
  channelP = ARG32;
  mode = ARG8;
  samplerate = ARG32;
  type = ARG16;
  width = ARG8;
  funcP = ARG32;
  userDataP = ARG32;
  buffsize = ARG32;
  armNative = ARG8;
  channelr = (SndStreamRef *)emupalmos_trap_in(channelP, trap, 0);
  func = (SndStreamBufferCallback)emupalmos_trap_in(funcP, trap, 5);
  userData = emupalmos_trap_in(userDataP, trap, 6);
  err = SndStreamCreate(channelr, mode, samplerate, type, width, func, userData, buffsize, armNative);
  if (channelP) m68k_write_memory_32(channelP, *channelr);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamCreate(0x%08X, %d, %d, %d, %d, 0x%08X, 0x%08X, %d, %d): %d",
    channelP, mode, samplerate, type, width, funcP, userDataP, buffsize, armNative, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSndStreamCreateExtended: {
  // Err SndStreamCreateExtended(SndStreamRef *channel, SndStreamMode mode, SndFormatType format, UInt32 samplerate, SndSampleType type, SndStreamWidth width, SndStreamVariableBufferCallback func, userData, UInt32 buffsize, Boolean armNative)
  channelP = ARG32;
  mode = ARG8;
  format = ARG32;
  samplerate = ARG32;
  type = ARG16;
  width = ARG8;
  funcP = ARG32;
  userDataP = ARG32;
  buffsize = ARG32;
  armNative = ARG8;
  channelr = (SndStreamRef *)emupalmos_trap_in(channelP, trap, 0);
  funcv = (SndStreamVariableBufferCallback)emupalmos_trap_in(funcP, trap, 6);
  userData = emupalmos_trap_in(userDataP, trap, 7);
  err = SndStreamCreateExtended(channelr, mode, format, samplerate, type, (uint8_t)width, funcv, userData, buffsize, armNative);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamCreateExtented(0x%08X, %d, %d, %d %d, %d, 0x%08X, 0x%08X, %d, %d): %d",
    channelP, mode, format, samplerate, type, width, funcP, userDataP, buffsize, armNative, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSndStreamDelete: {
  // Err SndStreamDelete(SndStreamRef channel)
  channel = ARG32;
  err = SndStreamDelete(channel);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamDelete(0x%08X): %d", channel, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSndStreamSetVolume: {
  // Err SndStreamSetVolume(SndStreamRef channel, Int32 volume)
  channel = ARG32;
  uint32_t volume = ARG32;
  err = SndStreamSetVolume(channel, volume);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamSetVolume(0x%08X, %d): %d", channel, volume, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSndStreamStart: {
  // Err SndStreamStart(SndStreamRef channel)
  channel = ARG32;
  err = SndStreamStart(channel);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamStart(0x%08X): %d", channel, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSndStreamStop: {
  // Err SndStreamStop(SndStreamRef channel)
  channel = ARG32;
  err = SndStreamStop(channel);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamStop(0x%08X): %d", channel, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapGrfGetState: {
  // Err GrfGetState(Boolean *capsLockP, Boolean *numLockP, UInt16 *tempShiftP, Boolean *autoShiftedP)
  capsLockP = ARG32;
  numLockP = ARG32;
  tempShiftP = ARG32;
  autoShiftedP = ARG32;
  emupalmos_trap_in(capsLockP, trap, 0);
  emupalmos_trap_in(numLockP, trap, 1);
  emupalmos_trap_in(tempShiftP, trap, 2);
  emupalmos_trap_in(autoShiftedP, trap, 3);
  tempShift = 0;
  err = GrfGetState(&capsLock, &numLock, &tempShift, &autoShifted);
  debug(DEBUG_TRACE, "EmuPalmOS", "GrfGetState(%d, %d, %d, %d): %d", capsLock, numLock, tempShift, autoShifted, err);
  if (capsLockP) m68k_write_memory_8(capsLockP, capsLock);
  if (numLockP) m68k_write_memory_8(numLockP, numLock);
  if (tempShiftP) m68k_write_memory_16(tempShiftP, tempShift);
  if (autoShiftedP) m68k_write_memory_8(autoShiftedP, autoShifted);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapGrfSetState: {
  // Err GrfSetState(Boolean capsLock, Boolean numLock, Boolean upperShift)
  capsLock = ARG8;
  numLock = ARG8;
  upperShift = ARG8;
  err = GrfSetState(capsLock, numLock, upperShift);
  debug(DEBUG_TRACE, "EmuPalmOS", "GrfSetState(%d, %d, %d): %d", capsLock, numLock, upperShift, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSysNotifyBroadcastDeferred: {
  // Err SysNotifyBroadcastDeferred(SysNotifyParamType *notify, Int16 paramSize)
  notifyP = ARG32;
  paramSize = ARG16;
  emupalmos_trap_in(notifyP, trap, 0);
  decode_notify(notifyP, &notify);
  err = SysNotifyBroadcastDeferred(notifyP ? &notify : NULL, paramSize);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysNotifyBroadcastDeferred(0x%08X, %d): %d", notifyP, paramSize, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapCrc16CalcBlock: {
  // UInt16 Crc16CalcBlock(const void *bufP, UInt16 count, UInt16 crc)
  bufP = ARG32;
  count = ARG16;
  crc = ARG16;
  bufp = emupalmos_trap_in(bufP, trap, 0);
  res16 = Crc16CalcBlock(bufp, count, crc);
  debug(DEBUG_TRACE, "EmuPalmOS", "Crc16CalcBlock(0x%08X, %d, 0x%04X): 0x%04X", bufP, count, crc, res16);
  m68k_set_reg(M68K_REG_D0, res16);
}
break;
case sysTrapGsiInitialize: {
  // void GsiInitialize(void)
  GsiInitialize();
  debug(DEBUG_TRACE, "EmuPalmOS", "GsiInitialize()");
}
break;
case sysTrapGsiSetShiftState: {
  // void GsiSetShiftState(const UInt16 lockFlags, const UInt16 tempShift)
  lockFlags = ARG16;
  tempShift = ARG16;
  GsiSetShiftState(lockFlags, tempShift);
  debug(DEBUG_TRACE, "EmuPalmOS", "GsiSetShiftState(0x%04X, 0x%04X)", lockFlags, tempShift);
}
break;
case sysTrapGsiEnable: {
  // void GsiEnable(const Boolean enableIt)
  enableIt = ARG8;
  GsiEnable((const Boolean)enableIt);
  debug(DEBUG_TRACE, "EmuPalmOS", "GsiEnable(%d)", enableIt);
}
break;
case sysTrapGsiSetLocation: {
  // void GsiSetLocation(const Int16 x, const Int16 y)
  x = ARG16;
  y = ARG16;
  GsiSetLocation(x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "GsiSetLocation(%d, %d)", x, y);
}
break;
case sysTrapPrgStartDialogV31: {
  // prgStartDialogV31(const title, PrgCallbackFunc textCallback)
  titleP = ARG32;
  textCallbackP = ARG32;
  title = emupalmos_trap_in(titleP, trap, 0);
  textCallback = emupalmos_trap_in(textCallbackP, trap, 1);
  prg = PrgStartDialogV31(title, textCallback);
  prgP = emupalmos_trap_out(prg);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrgStartDialogV31(0x%08X [%s], 0x%08X): 0x%08X", titleP, title ? title : "", textCallbackP, prgP);
  m68k_set_reg(M68K_REG_A0, prgP);
}
break;
case sysTrapPrgStartDialog: {
  // prgStartDialog(const title, PrgCallbackFunc textCallback, userDataP)
  titleP = ARG32;
  textCallbackP = ARG32;
  userDataP = ARG32;
  title = emupalmos_trap_in(titleP, trap, 0);
  textCallback = emupalmos_trap_in(textCallbackP, trap, 1);
  userData = emupalmos_trap_in(userDataP, trap, 2);
  prg = PrgStartDialog(title, textCallback, userData);
  prgP = emupalmos_trap_out(prg);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrgStartDialog(0x%08X [%s], 0x%08X, 0x%08X): 0x%08X", titleP, title ? title : "", textCallbackP, userDataP, prgP);
  m68k_set_reg(M68K_REG_A0, prgP);
}
break;
case sysTrapPrgStopDialog: {
  // void PrgStopDialog(prgP, Boolean force)
  prgP = ARG32;
  force = ARG8;
  prgv = emupalmos_trap_in(prgP, trap, 0);
  PrgStopDialog(prgv, force);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrgStopDialog(0x%08X)", prgP);
}
break;
case sysTrapPrgHandleEvent: {
  // Boolean PrgHandleEvent(prgGP, EventType *eventP)
  prgP = ARG32;
  eventP = ARG32;
  prgv = emupalmos_trap_in(prgP, trap, 0);
  emupalmos_trap_in(eventP, trap, 1);

  if (eventP) decode_event(eventP, &l_eventP);
  resb = PrgHandleEvent(prgv, &l_eventP);
  eventName = EvtGetEventName(l_eventP.eType);
  if (eventName) {
    debug(DEBUG_TRACE, "EmuPalmOS", "PrgHandleEvent(0x%08X, 0x%08X [%s]): %d", prgP, eventP, eventName, resb);
  } else {
    debug(DEBUG_TRACE, "EmuPalmOS", "PrgHandleEvent(0x%08X, 0x%08X [0x%04X]): %d", prgP, eventP, l_eventP.eType, resb);
  }
  m68k_set_reg(M68K_REG_D0, resb);
}
break;
case sysTrapPrgUpdateDialog: {
  // void PrgUpdateDialog(prgGP, UInt16 err, UInt16 stage, const Char *messageP, Boolean updateNow)
  prgP = ARG32;
  err = ARG16;
  stage = ARG16;
  messageP = ARG32;
  updateNow = ARG8;
  void *prg = emupalmos_trap_in(prgP, trap, 0);
  char *message = emupalmos_trap_in(messageP, trap, 3);
  PrgUpdateDialog(prg, err, stage, message, updateNow);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrgUpdateDialog(0x%08X, %d, %d, 0x%08X [%s], %d)", prgP, err, stage, messageP, messageP ? message : "", updateNow);
}
break;
case sysTrapEncDigestMD5: {
  // Err EncDigestMD5(UInt8 *strP, UInt16 strLen, UInt8 digestP[16])
  strP = ARG32;
  strLen = ARG16;
  digestP = ARG32;
  strc = emupalmos_trap_in(strP, trap, 0);
  digest = emupalmos_trap_in(digestP, trap, 2);
  res = EncDigestMD5((uint8_t *)strc, strLen, digest);
  debug(DEBUG_TRACE, "EmuPalmOS", "EncDigestMD5(0x%08X, %u, 0x%08X): %d", strP, strLen, digestP, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapGetCharCaselessValue: {
  // const UInt8 *GetCharCaselessValue(void)
  res8 = (UInt8 *)GetCharCaselessValue();
  a = emupalmos_trap_out(res8);
  debug(DEBUG_TRACE, "EmuPalmOS", "GetCharCaselessValue(): 0x%08X", a);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapGetCharSortValue: {
  // const UInt8 *GetCharSortValue(void) 
  res8 = (UInt8 *)GetCharSortValue();
  a = emupalmos_trap_out(res8);
  debug(DEBUG_TRACE, "EmuPalmOS", "GetCharSortValue(): 0x%08X", a);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapAlmSetAlarm: {
  // Err AlmSetAlarm(UInt16 cardNo, LocalID dbID, UInt32 ref, UInt32 alarmSeconds, Boolean quiet)
  cardNo = ARG16;
  dbID = ARG32;
  uint32_t ref = ARG32;
  alarmSeconds = ARG32;
  uint8_t quiet = ARG8;
  err = AlmSetAlarm(cardNo, dbID, ref, alarmSeconds, quiet);
  debug(DEBUG_TRACE, "EmuPalmOS", "AlmSetAlarm(%d, 0x%08X, %u, %u, %u): %d", cardNo, dbID, ref, alarmSeconds, quiet, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapAlmGetAlarm: {
  // UInt32 AlmGetAlarm(UInt16 cardNo, LocalID dbID, UInt32 *refP)
  cardNo = ARG16;
  dbID = ARG32;
  refP = ARG32;
  emupalmos_trap_in(refP, trap, 2);
  ref = 0;
  res = AlmGetAlarm(cardNo, dbID, refP ? &ref : NULL);
  if (refP) m68k_write_memory_32(refP, ref);
  debug(DEBUG_TRACE, "EmuPalmOS", "AlmGetAlarm(%d, 0x%08X, 0x%08X): %u", cardNo, dbID, refP, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapErrExceptionList: {
  // MemPtr *ErrExceptionList(void)
  e = (uint8_t *)ErrExceptionList();
  a = emupalmos_trap_out(e);
  debug(DEBUG_TRACE, "EmuPalmOS", "ErrExceptionList(): 0x%08X", a);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapErrThrow: {
  // void ErrThrow(Int32 err)
  code = ARG32;
  e = (uint8_t *)ErrExceptionList();
  a = emupalmos_trap_out(e);
  exceptionP = m68k_read_memory_32(a);

  // typedef struct ErrExceptionType {
  //   struct ErrExceptionType *nextP;  // next exception type
  //   ErrJumpBuf state;                // setjmp/longjmp storage
  //   Int32 err;                       // Error code
  // } ErrExceptionType;
  nextP = m68k_read_memory_32(exceptionP);
  m68k_write_memory_32(a, nextP);
  bufP = exceptionP + 4;
  aux = m68k_read_memory_32(bufP);
  m68k_set_reg(M68K_REG_D3, aux);
  aux = m68k_read_memory_32(bufP + 4);
  m68k_set_reg(M68K_REG_D4, aux);
  aux = m68k_read_memory_32(bufP + 8);
  m68k_set_reg(M68K_REG_D5, aux);
  aux = m68k_read_memory_32(bufP + 12);
  m68k_set_reg(M68K_REG_D6, aux);
  aux = m68k_read_memory_32(bufP + 16);
  m68k_set_reg(M68K_REG_D7, aux);
  aux = m68k_read_memory_32(bufP + 20);
  m68k_set_reg(M68K_REG_PC, aux);
  aux = m68k_read_memory_32(bufP + 24);
  m68k_set_reg(M68K_REG_A2, aux);
  aux = m68k_read_memory_32(bufP + 28);
  m68k_set_reg(M68K_REG_A3, aux);
  aux = m68k_read_memory_32(bufP + 32);
  m68k_set_reg(M68K_REG_A4, aux);
  aux = m68k_read_memory_32(bufP + 36);
  m68k_set_reg(M68K_REG_A5, aux);
  aux = m68k_read_memory_32(bufP + 40);
  m68k_set_reg(M68K_REG_A6, aux);
  aux = m68k_read_memory_32(bufP + 44);
  m68k_set_reg(M68K_REG_A7, aux);
  debug(DEBUG_TRACE, "EmuPalmOS", "ErrThrow(%d)", code);
  m68k_set_reg(M68K_REG_D0, code);
}
break;
case sysTrapErrSetJump: {
  // Int16 ErrSetJump(ErrJumpBuf buf)
  bufP = ARG32;
  emupalmos_trap_in(bufP, trap, 0);
  // typedef long *ErrJumpBuf[12];  // D3-D7,PC,A2-A7
  aux = m68k_get_reg(NULL, M68K_REG_D3);
  m68k_write_memory_32(bufP, aux);
  aux = m68k_get_reg(NULL, M68K_REG_D4);
  m68k_write_memory_32(bufP + 4, aux);
  aux = m68k_get_reg(NULL, M68K_REG_D5);
  m68k_write_memory_32(bufP + 8, aux);
  aux = m68k_get_reg(NULL, M68K_REG_D6);
  m68k_write_memory_32(bufP + 12, aux);
  aux = m68k_get_reg(NULL, M68K_REG_D7);
  m68k_write_memory_32(bufP + 16, aux);
  aux = m68k_get_reg(NULL, M68K_REG_PC);
  m68k_write_memory_32(bufP + 20, aux);
  aux = m68k_get_reg(NULL, M68K_REG_A2);
  m68k_write_memory_32(bufP + 24, aux);
  aux = m68k_get_reg(NULL, M68K_REG_A3);
  m68k_write_memory_32(bufP + 28, aux);
  aux = m68k_get_reg(NULL, M68K_REG_A4);
  m68k_write_memory_32(bufP + 32, aux);
  aux = m68k_get_reg(NULL, M68K_REG_A5);
  m68k_write_memory_32(bufP + 36, aux);
  aux = m68k_get_reg(NULL, M68K_REG_A6);
  m68k_write_memory_32(bufP + 40, aux);
  aux = m68k_get_reg(NULL, M68K_REG_A7);
  m68k_write_memory_32(bufP + 44, aux);
  debug(DEBUG_TRACE, "EmuPalmOS", "ErrSetJump(0x%08X): %d", bufP, 0);
  m68k_set_reg(M68K_REG_D0, 0); // XXX not calling ErrSetJump()
}
break;
case sysTrapErrLongJump: {
  // void ErrLongJump(ErrJumpBuf buf, Int16 result)
  bufP = ARG32;
  result = ARG16;
  emupalmos_trap_in(bufP, trap, 0);
  aux = m68k_read_memory_32(bufP);
  m68k_set_reg(M68K_REG_D3, aux);
  aux = m68k_read_memory_32(bufP + 4);
  m68k_set_reg(M68K_REG_D4, aux);
  aux = m68k_read_memory_32(bufP + 8);
  m68k_set_reg(M68K_REG_D5, aux);
  aux = m68k_read_memory_32(bufP + 12);
  m68k_set_reg(M68K_REG_D6, aux);
  aux = m68k_read_memory_32(bufP + 16);
  m68k_set_reg(M68K_REG_D7, aux);
  aux = m68k_read_memory_32(bufP + 20);
  m68k_set_reg(M68K_REG_PC, aux);
  aux = m68k_read_memory_32(bufP + 24);
  m68k_set_reg(M68K_REG_A2, aux);
  aux = m68k_read_memory_32(bufP + 28);
  m68k_set_reg(M68K_REG_A3, aux);
  aux = m68k_read_memory_32(bufP + 32);
  m68k_set_reg(M68K_REG_A4, aux);
  aux = m68k_read_memory_32(bufP + 36);
  m68k_set_reg(M68K_REG_A5, aux);
  aux = m68k_read_memory_32(bufP + 40);
  m68k_set_reg(M68K_REG_A6, aux);
  aux = m68k_read_memory_32(bufP + 44);
  m68k_set_reg(M68K_REG_A7, aux);
  debug(DEBUG_TRACE, "EmuPalmOS", "ErrLongJump(0x%08X, %d)", bufP, result);
  m68k_set_reg(M68K_REG_D0, result);
}
break;
case sysTrapErrAlertCustom: {
  // Int16 ErrAlertCustom(Err errCode, Char *errMsgP, Char *preMsgP, Char *postMsgP)
  errCode = ARG16;
  errMsgP = ARG32;
  preMsgP = ARG32;
  postMsgP = ARG32;
  errMsg = emupalmos_trap_in(errMsgP, trap, 1);
  preMsg = emupalmos_trap_in(preMsgP, trap, 2);
  postMsg = emupalmos_trap_in(postMsgP, trap, 3);
  res = ErrAlertCustom(errCode, errMsg, preMsg, postMsg);
  debug(DEBUG_TRACE, "EmuPalmOS", "ErrAlertCustom(%u, 0x%08X, 0x%08X, 0x%08X): %d", errCode, errMsgP, preMsgP, postMsgP, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapMemInit: {
  // Err MemInit(void)
  res = MemInit();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemInit(): %d", res);
}
break;
case sysTrapMemKernelInit: {
  // Err MemKernelInit(void)
  res = MemKernelInit();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemKernelInit(): %d", res);
}
break;
case sysTrapMemInitHeapTable: {
  // Err MemInitHeapTable(UInt16 cardNo)
  cardNo = ARG16;
  res = MemInitHeapTable(cardNo);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemInitHeapTable(cardNo=%d): %d", cardNo, res);
}
break;
case sysTrapMemNumCards: {
  // UInt16 MemNumCards(void)
  res = (UInt32)MemNumCards();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemNumCards(): %d", res);
}
break;
case sysTrapMemCardInfo: {
  // Err MemCardInfo(UInt16 cardNo, out Char *cardNameP, out Char *manufNameP, out UInt16 *versionP, out UInt32 *crDateP, out UInt32 *romSizeP, out UInt32 *ramSizeP, out UInt32 *freeBytesP)
  cardNo = ARG16;
  cardNameP = ARG32;
  s_cardNameP = emupalmos_trap_in(cardNameP, trap, 1);
  manufNameP = ARG32;
  s_manufNameP = emupalmos_trap_in(manufNameP, trap, 2);
  versionP = ARG32;
  crDateP = ARG32;
  romSizeP = ARG32;
  ramSizeP = ARG32;
  freeBytesP = ARG32;
  res = MemCardInfo(cardNo, cardNameP ? s_cardNameP : NULL, manufNameP ? s_manufNameP : NULL, versionP ? &l_versionP : NULL, crDateP ? &l_crDateP : NULL, romSizeP ? &l_romSizeP : NULL, ramSizeP ? &l_ramSizeP : NULL, freeBytesP ? &l_freeBytesP : NULL);
  if (versionP) m68k_write_memory_16(versionP, l_versionP);
  if (crDateP) m68k_write_memory_32(crDateP, l_crDateP);
  if (romSizeP) m68k_write_memory_32(romSizeP, l_romSizeP);
  if (ramSizeP) m68k_write_memory_32(ramSizeP, l_ramSizeP);
  if (freeBytesP) m68k_write_memory_32(freeBytesP, l_freeBytesP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemCardInfo(cardNo=%d, cardNameP=0x%08X [%s], manufNameP=0x%08X [%s], versionP=0x%08X [%d], crDateP=0x%08X [%d], romSizeP=0x%08X [%d], ramSizeP=0x%08X [%d], freeBytesP=0x%08X [%d]): %d", cardNo, cardNameP, s_cardNameP, manufNameP, s_manufNameP, versionP, l_versionP, crDateP, l_crDateP, romSizeP, l_romSizeP, ramSizeP, l_ramSizeP, freeBytesP, l_freeBytesP, res);
}
break;
case sysTrapMemNumHeaps: {
  // UInt16 MemNumHeaps(UInt16 cardNo)
  cardNo = ARG16;
  res = (UInt32)MemNumHeaps(cardNo);
  m68k_set_reg(M68K_REG_D0, (UInt16)res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemNumHeaps(cardNo=%d): %d", cardNo, res);
}
break;
case sysTrapMemNumRAMHeaps: {
  // UInt16 MemNumRAMHeaps(UInt16 cardNo)
  cardNo = ARG16;
  res = (UInt32)MemNumRAMHeaps(cardNo);
  m68k_set_reg(M68K_REG_D0, (uint16_t)res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemNumRAMHeaps(cardNo=%d): %d", cardNo, res);
}
break;
case sysTrapMemHeapID: {
  // UInt16 MemHeapID(UInt16 cardNo, UInt16 heapIndex)
  cardNo = ARG16;
  heapIndex = ARG16;
  res = (UInt32)MemHeapID(cardNo, heapIndex);
  m68k_set_reg(M68K_REG_D0, (uint16_t)res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapID(cardNo=%d, heapIndex=%d): %d", cardNo, heapIndex, res);
}
break;
case sysTrapMemHeapDynamic: {
  // Boolean MemHeapDynamic(UInt16 heapID)
  heapID = ARG16;
  res = (UInt32)MemHeapDynamic(heapID);
  m68k_set_reg(M68K_REG_D0, (Boolean)res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapDynamic(heapID=%d): %d", heapID, res);
}
break;
case sysTrapMemHeapFreeBytes: {
  // Err MemHeapFreeBytes(UInt16 heapID, out UInt32 *freeP, out UInt32 *maxP)
  heapID = ARG16;
  freeP = ARG32;
  maxP = ARG32;
  res = MemHeapFreeBytes(heapID, freeP ? &l_freeP : NULL, maxP ? &l_maxP : NULL);
  if (freeP) m68k_write_memory_32(freeP, l_freeP);
  if (maxP) m68k_write_memory_32(maxP, l_maxP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapFreeBytes(heapID=%d, freeP=0x%08X [%d], maxP=0x%08X [%d]): %d", heapID, freeP, l_freeP, maxP, l_maxP, res);
}
break;
case sysTrapMemHeapSize: {
  // UInt32 MemHeapSize(UInt16 heapID)
  heapID = ARG16;
  res = (UInt32)MemHeapSize(heapID);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapSize(heapID=%d): %d", heapID, res);
}
break;
case sysTrapMemHeapFlags: {
  // UInt16 MemHeapFlags(UInt16 heapID)
  heapID = ARG16;
  res = (UInt32)MemHeapFlags(heapID);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapFlags(heapID=%d): %d", heapID, res);
}
break;
case sysTrapMemHeapCompact: {
  // Err MemHeapCompact(UInt16 heapID)
  heapID = ARG16;
  res = MemHeapCompact(heapID);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapCompact(heapID=%d): %d", heapID, res);
}
break;
case sysTrapMemHeapInit: {
  // Err MemHeapInit(UInt16 heapID, Int16 numHandles, Boolean initContents)
  heapID = ARG16;
  numHandles = ARG16;
  initContents = ARG8;
  res = MemHeapInit(heapID, numHandles, initContents);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapInit(heapID=%d, numHandles=%d, initContents=%d): %d", heapID, numHandles, initContents, res);
}
break;
case sysTrapMemHeapFreeByOwnerID: {
  // Err MemHeapFreeByOwnerID(UInt16 heapID, UInt16 ownerID)
  heapID = ARG16;
  ownerID = ARG16;
  res = MemHeapFreeByOwnerID(heapID, ownerID);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapFreeByOwnerID(heapID=%d, ownerID=%d): %d", heapID, ownerID, res);
}
break;
case sysTrapMemChunkNew: {
  // MemPtr MemChunkNew(UInt16 heapID, UInt32 size, UInt16 attr)
  heapID = ARG16;
  size = ARG32;
  attr = ARG16;
  resm = MemChunkNew(heapID, size, attr);
  r_res = emupalmos_trap_out(resm);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemChunkNew(heapID=%d, size=%d, attr=%d): 0x%08X", heapID, size, attr, r_res);
}
break;
case sysTrapMemChunkFree: {
  // Err MemChunkFree(MemPtr chunkDataP)
  chunkDataP = ARG32;
  l_chunkDataP = emupalmos_trap_in(chunkDataP, trap, 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemChunkFree(chunkDataP=0x%08X) ...", chunkDataP);
  res = MemChunkFree(chunkDataP ? l_chunkDataP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemChunkFree(chunkDataP=0x%08X): %d", chunkDataP, res);
}
break;
case sysTrapMemPtrNew: {
  // MemPtr MemPtrNew(UInt32 size)
  size = ARG32;
  resm = MemPtrNew(size);
  r_res = emupalmos_trap_out(resm);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrNew(size=%u): 0x%08X to 0x%08X", size, r_res, r_res + size - 1);
}
break;
case sysTrapMemPtrRecoverHandle: {
  // MemHandle MemPtrRecoverHandle(MemPtr p)
  pP = ARG32;
  l_p = emupalmos_trap_in(pP, trap, 0);
  resmem = MemPtrRecoverHandle(pP ? l_p : 0);
  r_res = emupalmos_trap_out(resmem);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrRecoverHandle(p=0x%08X): %p", pP, resmem);
}
break;
case sysTrapMemPtrFlags: {
  // UInt16 MemPtrFlags(MemPtr p)
  pP = ARG32;
  l_p = emupalmos_trap_in(pP, trap, 0);
  res = (UInt32)MemPtrFlags(pP ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrFlags(p=0x%08X): %d", pP, res);
}
break;
case sysTrapMemPtrSize: {
  // UInt32 MemPtrSize(MemPtr p)
  pP = ARG32;
  l_p = emupalmos_trap_in(pP, trap, 0);
  res = (UInt32)MemPtrSize(pP ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrSize(p=0x%08X): %d", pP, res);
}
break;
case sysTrapMemPtrOwner: {
  // UInt16 MemPtrOwner(MemPtr p)
  pP = ARG32;
  l_p = emupalmos_trap_in(pP, trap, 0);
  res = (UInt32)MemPtrOwner(pP ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrOwner(p=0x%08X): %d", pP, res);
}
break;
case sysTrapMemPtrHeapID: {
  // UInt16 MemPtrHeapID(MemPtr p)
  pP = ARG32;
  l_p = emupalmos_trap_in(pP, trap, 0);
  res = (UInt32)MemPtrHeapID(pP ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrHeapID(p=0x%08X): %d", pP, res);
}
break;
case sysTrapMemPtrDataStorage: {
  // Boolean MemPtrDataStorage(MemPtr p)
  pP = ARG32;
  l_p = emupalmos_trap_in(pP, trap, 0);
  res = (UInt32)MemPtrDataStorage(pP ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrDataStorage(p=0x%08X): %d", pP, res);
}
break;
case sysTrapMemPtrCardNo: {
  // UInt16 MemPtrCardNo(MemPtr p)
  pP = ARG32;
  l_p = emupalmos_trap_in(pP, trap, 0);
  res = (UInt32)MemPtrCardNo(pP ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrCardNo(p=0x%08X): %d", pP, res);
}
break;
case sysTrapMemPtrToLocalID: {
  // LocalID MemPtrToLocalID(MemPtr p)
  pP = ARG32;
  l_p = emupalmos_trap_in(pP, trap, 0);
  resid = MemPtrToLocalID(pP ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, resid);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrToLocalID(p=0x%08X): 0x%08X", pP, resid);
}
break;
case sysTrapMemPtrSetOwner: {
  // Err MemPtrSetOwner(MemPtr p, UInt16 owner)
  pP = ARG32;
  l_p = emupalmos_trap_in(pP, trap, 0);
  owner = ARG16;
  res = MemPtrSetOwner(pP ? l_p : 0, owner);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrSetOwner(p=0x%08X, owner=%d): %d", pP, owner, res);
}
break;
case sysTrapMemPtrResize: {
  // Err MemPtrResize(MemPtr p, UInt32 newSize)
  pP = ARG32;
  l_p = emupalmos_trap_in(pP, trap, 0);
  newSize = ARG32;
  res = MemPtrResize(pP ? l_p : 0, newSize);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrResize(p=0x%08X, newSize=%d): %d", pP, newSize, res);
}
break;
case sysTrapMemPtrResetLock: {
  // Err MemPtrResetLock(MemPtr p)
  pP = ARG32;
  l_p = emupalmos_trap_in(pP, trap, 0);
  res = MemPtrResetLock(pP ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrResetLock(p=0x%08X): %d", pP, res);
}
break;
case sysTrapMemPtrUnlock: {
  // Err MemPtrUnlock(MemPtr p)
  pP = ARG32;
  l_p = emupalmos_trap_in(pP, trap, 0);
  res = MemPtrUnlock(pP ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrUnlock(p=0x%08X): %d", pP, res);
}
break;
case sysTrapMemHandleNew: {
  // MemHandle MemHandleNew(UInt32 size)
  size = ARG32;
  resmem = MemHandleNew(size);
  r_res = emupalmos_trap_out(resmem);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleNew(size=%d): 0x%08X", size, r_res);
}
break;
case sysTrapMemHandleFree: {
  // Err MemHandleFree(MemHandle h)
  h = ARG32;
  l_h = emupalmos_trap_in(h, trap, 0);
  res = MemHandleFree(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleFree(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleFlags: {
  // UInt16 MemHandleFlags(MemHandle h)
  h = ARG32;
  l_h = emupalmos_trap_in(h, trap, 0);
  res = (UInt32)MemHandleFlags(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleFlags(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleSize: {
  // UInt32 MemHandleSize(MemHandle h)
  h = ARG32;
  l_h = emupalmos_trap_in(h, trap, 0);
  res = (UInt32)MemHandleSize(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleSize(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleOwner: {
  // UInt16 MemHandleOwner(MemHandle h)
  h = ARG32;
  l_h = emupalmos_trap_in(h, trap, 0);
  res = (UInt32)MemHandleOwner(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleOwner(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleLockCount: {
  // UInt16 MemHandleLockCount(MemHandle h)
  h = ARG32;
  l_h = emupalmos_trap_in(h, trap, 0);
  res = (UInt32)MemHandleLockCount(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleLockCount(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleHeapID: {
  // UInt16 MemHandleHeapID(MemHandle h)
  h = ARG32;
  l_h = emupalmos_trap_in(h, trap, 0);
  res = (UInt32)MemHandleHeapID(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleHeapID(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleDataStorage: {
  // Boolean MemHandleDataStorage(MemHandle h)
  h = ARG32;
  l_h = emupalmos_trap_in(h, trap, 0);
  res = (UInt32)MemHandleDataStorage(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleDataStorage(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleCardNo: {
  // UInt16 MemHandleCardNo(MemHandle h)
  h = ARG32;
  l_h = emupalmos_trap_in(h, trap, 0);
  res = (UInt32)MemHandleCardNo(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleCardNo(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleToLocalID: {
  // LocalID MemHandleToLocalID(MemHandle h)
  h = ARG32;
  l_h = emupalmos_trap_in(h, trap, 0);
  resid = MemHandleToLocalID(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, resid);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleToLocalID(h=0x%08X): 0x%08X", h, resid);
}
break;
case sysTrapMemHandleSetOwner: {
  // Err MemHandleSetOwner( MemHandle h, UInt16 owner)
  h = ARG32;
  l_h = emupalmos_trap_in(h, trap, 0);
  owner = ARG16;
  res = MemHandleSetOwner(h ? l_h : 0, owner);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleSetOwner(h=0x%08X, owner=%d): %d", h, owner, res);
}
break;
case sysTrapMemHandleResize: {
  // Err MemHandleResize(MemHandle h, UInt32 newSize)
  h = ARG32;
  l_h = emupalmos_trap_in(h, trap, 0);
  newSize = ARG32;
  res = MemHandleResize(h ? l_h : 0, newSize);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleResize(h=0x%08X, newSize=%d): %d", h, newSize, res);
}
break;
case sysTrapMemHandleUnlock: {
  // Err MemHandleUnlock(MemHandle h)
  h = ARG32;
  l_h = emupalmos_trap_in(h, trap, 0);
  res = MemHandleUnlock(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleUnlock(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleResetLock: {
  // Err MemHandleResetLock(MemHandle h)
  h = ARG32;
  l_h = emupalmos_trap_in(h, trap, 0);
  res = MemHandleResetLock(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleResetLock(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemLocalIDToGlobal: {
  // MemPtr MemLocalIDToGlobal(local, UInt16 cardNo)
  local = ARG32;
  cardNo = ARG16;
  resmp = MemLocalIDToGlobal(local, cardNo);
  r_res = emupalmos_trap_out(resmp);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemLocalIDToGlobal(local=0x%08X, cardNo=%d): 0x%08X", local, cardNo, r_res);
}
break;
case sysTrapMemLocalIDKind: {
  // LocalIDKind MemLocalIDKind(local)
  local = ARG32;
  reslid = MemLocalIDKind(local);
  m68k_set_reg(M68K_REG_D0, reslid);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemLocalIDKind(local=0x%08X): %d", local, reslid);
}
break;
case sysTrapMemLocalIDToPtr: {
  // MemPtr MemLocalIDToPtr(local, UInt16 cardNo)
  local = ARG32;
  cardNo = ARG16;
  resmp = MemLocalIDToPtr(local, cardNo);
  r_res = emupalmos_trap_out(resmp);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemLocalIDToPtr(local=0x%08X, cardNo=%d): 0x%08X", local, cardNo, r_res);
}
break;
case sysTrapMemLocalIDToLockedPtr: {
  // MemPtr MemLocalIDToLockedPtr(local, UInt16 cardNo)
  local = ARG32;
  cardNo = ARG16;
  resmp = MemLocalIDToLockedPtr(local, cardNo);
  r_res = emupalmos_trap_out(resmp);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemLocalIDToLockedPtr(local=0x%08X, cardNo=%d): 0x%08X", local, cardNo, r_res);
}
break;
case sysTrapMemCmp: {
  // Int16 MemCmp(in void *s1, in void *s2, Int32 numBytes)
  s1_32 = ARG32;
  s_s1 = emupalmos_trap_in(s1_32, trap, 0);
  s2_32 = ARG32;
  s_s2 = emupalmos_trap_in(s2_32, trap, 1);
  numBytes = ARG32;
  res = MemCmp(s1 ? s_s1 : NULL, s2 ? s_s2 : NULL, numBytes);
  m68k_set_reg(M68K_REG_D0, (Int16)res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemCmp(s1=0x%08X, s2=0x%08X, numBytes=%d): %d", s1_32, s2_32, numBytes, res);
}
break;
case sysTrapMemSemaphoreReserve: {
  // Err MemSemaphoreReserve(Boolean writeAccess)
  writeAccess = ARG8;
  res = MemSemaphoreReserve(writeAccess);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemSemaphoreReserve(writeAccess=%d): %d", writeAccess, res);
}
break;
case sysTrapMemSemaphoreRelease: {
  // Err MemSemaphoreRelease(Boolean writeAccess)
  writeAccess = ARG8;
  res = MemSemaphoreRelease(writeAccess);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemSemaphoreRelease(writeAccess=%d): %d", writeAccess, res);
}
break;
case sysTrapMemDebugMode: {
  // UInt16 MemDebugMode(void)
  res = (UInt32)MemDebugMode();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemDebugMode(): %d", res);
}
break;
case sysTrapMemSetDebugMode: {
  // Err MemSetDebugMode(UInt16 flags)
  uint16_t flags = ARG16;
  res = MemSetDebugMode(flags);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemSetDebugMode(flags=%d): %d", flags, res);
}
break;
case sysTrapMemHeapScramble: {
  // Err MemHeapScramble(UInt16 heapID)
  heapID = ARG16;
  res = MemHeapScramble(heapID);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapScramble(heapID=%d): %d", heapID, res);
}
break;
case sysTrapMemHeapCheck: {
  // Err MemHeapCheck(UInt16 heapID)
  heapID = ARG16;
  res = MemHeapCheck(heapID);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapCheck(heapID=%d): %d", heapID, res);
}
break;
case sysTrapDmInit: {
  // Err DmInit(void)
  res = DmInit();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmInit(): %d", res);
}
break;
case sysTrapDmCreateDatabase: {
  // Err DmCreateDatabase(UInt16 cardNo, in Char *nameP, UInt32 creator, UInt32 type, Boolean resDB)
  cardNo = ARG16;
  nameP = ARG32;
  s_nameP = emupalmos_trap_in(nameP, trap, 1);
  creator = ARG32;
  type = ARG32;
  resDB = ARG8;
  res = DmCreateDatabase(cardNo, nameP ? s_nameP : NULL, creator, type, resDB);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmCreateDatabase(cardNo=%d, nameP=0x%08X [%s], creator=%d, type=%d, resDB=%d): %d", cardNo, nameP, s_nameP, creator, type, resDB, res);
}
break;
case sysTrapDmCreateDatabaseFromImage: {
  // Err DmCreateDatabaseFromImage(MemPtr bufferP)
  bufferP = ARG32;
  l_bufferP = emupalmos_trap_in(bufferP, trap, 0);
  res = DmCreateDatabaseFromImage(bufferP ? l_bufferP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmCreateDatabaseFromImage(bufferP=0x%08X): %d", bufferP, res);
}
break;
case sysTrapDmDeleteDatabase: {
  // Err DmDeleteDatabase(UInt16 cardNo, dbID)
  cardNo = ARG16;
  dbID = ARG32;
  res = DmDeleteDatabase(cardNo, dbID);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmDeleteDatabase(cardNo=%d, dbID=0x%08X): %d", cardNo, dbID, res);
}
break;
case sysTrapDmNumDatabases: {
  // UInt16 DmNumDatabases(UInt16 cardNo)
  cardNo = ARG16;
  res = (UInt32)DmNumDatabases(cardNo);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNumDatabases(cardNo=%d): %d", cardNo, res);
}
break;
case sysTrapDmGetDatabase: {
  // LocalID DmGetDatabase(UInt16 cardNo, UInt16 index)
  cardNo = ARG16;
  index = ARG16;
  resid = DmGetDatabase(cardNo, index);
  m68k_set_reg(M68K_REG_D0, resid);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetDatabase(cardNo=%d, index=%d): 0x%08X", cardNo, index, resid);
}
break;
case sysTrapDmFindDatabase: {
  // LocalID DmFindDatabase(UInt16 cardNo, in Char *nameP)
  cardNo = ARG16;
  nameP = ARG32;
  s_nameP = emupalmos_trap_in(nameP, trap, 1);
  resid = DmFindDatabase(cardNo, nameP ? s_nameP : NULL);
  m68k_set_reg(M68K_REG_D0, resid);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmFindDatabase(cardNo=%d, nameP=0x%08X [%s]): 0x%08X", cardNo, nameP, s_nameP, resid);
}
break;
case sysTrapDmDatabaseInfo: {
  // Err DmDatabaseInfo(UInt16 cardNo, dbID, out Char *nameP, out UInt16 *attributesP, out UInt16 *versionP, out UInt32 *crDateP, out UInt32 *modDateP, out UInt32 *bckUpDateP, out UInt32 *modNumP, out LocalID *appInfoIDP, out LocalID *sortInfoIDP, out UInt32 *typeP, out UInt32 *creatorP)
  cardNo = ARG16;
  dbID = ARG32;
  nameP = ARG32;
  s_nameP = emupalmos_trap_in(nameP, trap, 2);
  attributesP = ARG32;
  versionP = ARG32;
  crDateP = ARG32;
  modDateP = ARG32;
  bckUpDateP = ARG32;
  modNumP = ARG32;
  appInfoIDP = ARG32;
  sortInfoIDP = ARG32;
  typeP = ARG32;
  creatorP = ARG32;
  res = DmDatabaseInfo(cardNo, dbID, nameP ? s_nameP : NULL, attributesP ? &l_attributesP : NULL, versionP ? &l_versionP : NULL, crDateP ? &l_crDateP : NULL, modDateP ? &l_modDateP : NULL, bckUpDateP ? &l_bckUpDateP : NULL, modNumP ? &l_modNumP : NULL, appInfoIDP ? &l_appInfoIDP : NULL, sortInfoIDP ? &l_sortInfoIDP : NULL, typeP ? &l_typeP : NULL, creatorP ? &l_creatorP : NULL);
  if (attributesP) m68k_write_memory_16(attributesP, l_attributesP);
  if (versionP) m68k_write_memory_16(versionP, l_versionP);
  if (crDateP) m68k_write_memory_32(crDateP, l_crDateP);
  if (modDateP) m68k_write_memory_32(modDateP, l_modDateP);
  if (bckUpDateP) m68k_write_memory_32(bckUpDateP, l_bckUpDateP);
  if (modNumP) m68k_write_memory_32(modNumP, l_modNumP);
  if (appInfoIDP) m68k_write_memory_32(appInfoIDP, l_appInfoIDP);
  if (sortInfoIDP) m68k_write_memory_32(sortInfoIDP, l_sortInfoIDP);
  if (typeP) m68k_write_memory_32(typeP, l_typeP);
  if (creatorP) m68k_write_memory_32(creatorP, l_creatorP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmDatabaseInfo(cardNo=%d, dbID=0x%08X, nameP=0x%08X [%s], attributesP=0x%08X [%d], versionP=0x%08X [%d], crDateP=0x%08X [%d], modDateP=0x%08X [%d], bckUpDateP=0x%08X [%d], modNumP=0x%08X [%d], appInfoIDP=0x%08X, sortInfoIDP=0x%08X, typeP=0x%08X [%d], creatorP=0x%08X [%d]): %d", cardNo, dbID, nameP, s_nameP, attributesP, l_attributesP, versionP, l_versionP, crDateP, l_crDateP, modDateP, l_modDateP, bckUpDateP, l_bckUpDateP, modNumP, l_modNumP, appInfoIDP, sortInfoIDP, typeP, l_typeP, creatorP, l_creatorP, res);
}
break;
case sysTrapDmSetDatabaseInfo: {
  // Err DmSetDatabaseInfo(UInt16 cardNo, dbID, in Char *nameP, in UInt16 *attributesP, in UInt16 *versionP, in UInt32 *crDateP, in UInt32 *modDateP, in UInt32 *bckUpDateP, in UInt32 *modNumP, in LocalID *appInfoIDP, in LocalID *sortInfoIDP, in UInt32 *typeP, in UInt32 *creatorP)
  cardNo = ARG16;
  dbID = ARG32;
  nameP = ARG32;
  s_nameP = emupalmos_trap_in(nameP, trap, 2);
  attributesP = ARG32;
  if (attributesP) l_attributesP = m68k_read_memory_16(attributesP);
  versionP = ARG32;
  if (versionP) l_versionP = m68k_read_memory_16(versionP);
  crDateP = ARG32;
  if (crDateP) l_crDateP = m68k_read_memory_32(crDateP);
  modDateP = ARG32;
  if (modDateP) l_modDateP = m68k_read_memory_32(modDateP);
  bckUpDateP = ARG32;
  if (bckUpDateP) l_bckUpDateP = m68k_read_memory_32(bckUpDateP);
  modNumP = ARG32;
  if (modNumP) l_modNumP = m68k_read_memory_32(modNumP);
  appInfoIDP = ARG32;
  if (appInfoIDP) l_appInfoIDP = m68k_read_memory_32(appInfoIDP);
  sortInfoIDP = ARG32;
  if (sortInfoIDP) l_sortInfoIDP = m68k_read_memory_32(sortInfoIDP);
  typeP = ARG32;
  if (typeP) l_typeP = m68k_read_memory_32(typeP);
  creatorP = ARG32;
  if (creatorP) l_creatorP = m68k_read_memory_32(creatorP);
  res = DmSetDatabaseInfo(cardNo, dbID, nameP ? s_nameP : NULL, attributesP ? &l_attributesP : NULL, versionP ? &l_versionP : NULL, crDateP ? &l_crDateP : NULL, modDateP ? &l_modDateP : NULL, bckUpDateP ? &l_bckUpDateP : NULL, modNumP ? &l_modNumP : NULL, appInfoIDP ? &l_appInfoIDP : NULL, sortInfoIDP ? &l_sortInfoIDP : NULL, typeP ? &l_typeP : NULL, creatorP ? &l_creatorP : NULL);
  if (attributesP) m68k_write_memory_16(attributesP, l_attributesP);
  if (versionP) m68k_write_memory_16(versionP, l_versionP);
  if (crDateP) m68k_write_memory_32(crDateP, l_crDateP);
  if (modDateP) m68k_write_memory_32(modDateP, l_modDateP);
  if (bckUpDateP) m68k_write_memory_32(bckUpDateP, l_bckUpDateP);
  if (modNumP) m68k_write_memory_32(modNumP, l_modNumP);
  if (typeP) m68k_write_memory_32(typeP, l_typeP);
  if (creatorP) m68k_write_memory_32(creatorP, l_creatorP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSetDatabaseInfo(cardNo=%d, dbID=0x%08X, nameP=0x%08X [%s], attributesP=0x%08X [%d], versionP=0x%08X [%d], crDateP=0x%08X [%d], modDateP=0x%08X [%d], bckUpDateP=0x%08X [%d], modNumP=0x%08X [%d], appInfoIDP=0x%08X, sortInfoIDP=0x%08X, typeP=0x%08X [%d], creatorP=0x%08X [%d]): %d", cardNo, dbID, nameP, s_nameP, attributesP, l_attributesP, versionP, l_versionP, crDateP, l_crDateP, modDateP, l_modDateP, bckUpDateP, l_bckUpDateP, modNumP, l_modNumP, appInfoIDP, sortInfoIDP, typeP, l_typeP, creatorP, l_creatorP, res);
}
break;
case sysTrapDmDatabaseSize: {
  // Err DmDatabaseSize(UInt16 cardNo, dbID, out UInt32 *numRecordsP, out UInt32 *totalBytesP, out UInt32 *dataBytesP)
  cardNo = ARG16;
  dbID = ARG32;
  numRecordsP = ARG32;
  totalBytesP = ARG32;
  dataBytesP = ARG32;
  res = DmDatabaseSize(cardNo, dbID, numRecordsP ? &l_numRecordsP : NULL, totalBytesP ? &l_totalBytesP : NULL, dataBytesP ? &l_dataBytesP : NULL);
  if (numRecordsP) m68k_write_memory_32(numRecordsP, l_numRecordsP);
  if (totalBytesP) m68k_write_memory_32(totalBytesP, l_totalBytesP);
  if (dataBytesP) m68k_write_memory_32(dataBytesP, l_dataBytesP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmDatabaseSize(cardNo=%d, dbID=0x%08X, numRecordsP=0x%08X [%d], totalBytesP=0x%08X [%d], dataBytesP=0x%08X [%d]): %d", cardNo, dbID, numRecordsP, l_numRecordsP, totalBytesP, l_totalBytesP, dataBytesP, l_dataBytesP, res);
}
break;
case sysTrapDmDatabaseProtect: {
  // Err DmDatabaseProtect(UInt16 cardNo, dbID, Boolean protect)
  cardNo = ARG16;
  dbID = ARG32;
  protect = ARG8;
  res = DmDatabaseProtect(cardNo, dbID, protect);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmDatabaseProtect(cardNo=%d, dbID=0x%08X, protect=%d): %d", cardNo, dbID, protect, res);
}
break;
case sysTrapDmOpenDatabase: {
  // DmOpenRef DmOpenDatabase(UInt16 cardNo, dbID, UInt16 mode)
  cardNo = ARG16;
  dbID = ARG32;
  mode = ARG16;
  resor = DmOpenDatabase(cardNo, dbID, mode);
  r_res = emupalmos_trap_out(resor);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmOpenDatabase(cardNo=%d, dbID=0x%08X, mode=%d): 0x%08X", cardNo, dbID, mode, r_res);
}
break;
case sysTrapDmOpenDatabaseByTypeCreator: {
  // DmOpenRef DmOpenDatabaseByTypeCreator(UInt32 type, UInt32 creator, UInt16 mode)
  type = ARG32;
  creator = ARG32;
  mode = ARG16;
  resor = DmOpenDatabaseByTypeCreator(type, creator, mode);
  r_res = emupalmos_trap_out(resor);
  m68k_set_reg(M68K_REG_A0, r_res);
  pumpkin_id2s(type, stype);
  pumpkin_id2s(creator, screator);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmOpenDatabaseByTypeCreator(type='%s', creator='%s', mode=%d): 0x%08X", stype, screator, mode, r_res);
}
break;
case sysTrapDmOpenDBNoOverlay: {
  // DmOpenRef DmOpenDBNoOverlay(UInt16 cardNo, dbID, UInt16 mode)
  cardNo = ARG16;
  dbID = ARG32;
  mode = ARG16;
  resor = DmOpenDBNoOverlay(cardNo, dbID, mode);
  r_res = emupalmos_trap_out(resor);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmOpenDBNoOverlay(cardNo=%d, dbID=0x%08X, mode=%d): 0x%08X", cardNo, dbID, mode, r_res);
}
break;
case sysTrapDmCloseDatabase: {
  // Err DmCloseDatabase(DmOpenRef dbP)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  res = DmCloseDatabase(dbP ? l_dbP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmCloseDatabase(dbP=0x%08X): %d", dbP, res);
}
break;
case sysTrapDmNextOpenDatabase: {
  // DmOpenRef DmNextOpenDatabase(DmOpenRef currentP)
  uint32_t currentP = ARG32;
  l_currentP = emupalmos_trap_in(currentP, trap, 0);
  resor = DmNextOpenDatabase(currentP ? l_currentP : 0);
  r_res = emupalmos_trap_out(resor);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNextOpenDatabase(currentP=0x%08X): 0x%08X", currentP, r_res);
}
break;
case sysTrapDmOpenDatabaseInfo: {
  // Err DmOpenDatabaseInfo(DmOpenRef dbP, out LocalID *dbIDP, out UInt16 *openCountP, out UInt16 *modeP, out UInt16 *cardNoP, out Boolean *resDBP)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  dbIDP = ARG32;
  openCountP = ARG32;
  modeP = ARG32;
  cardNoP = ARG32;
  resDBP = ARG32;
  res = DmOpenDatabaseInfo(dbP ? l_dbP : 0, dbIDP ? &l_dbIDP : NULL, openCountP ? &l_openCountP : NULL, modeP ? &l_modeP : NULL, cardNoP ? &l_cardNoP : NULL, resDBP ? &l_resDBP : NULL);
  if (dbIDP) m68k_write_memory_32(dbIDP, l_dbIDP);
  if (openCountP) m68k_write_memory_16(openCountP, l_openCountP);
  if (modeP) m68k_write_memory_16(modeP, l_modeP);
  if (cardNoP) m68k_write_memory_16(cardNoP, l_cardNoP);
  if (resDBP) m68k_write_memory_8(resDBP, l_resDBP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmOpenDatabaseInfo(dbP=0x%08X, dbIDP=0x%08X, openCountP=0x%08X [%d], modeP=0x%08X [%d], cardNoP=0x%08X [%d], resDBP=0x%08X): %d", dbP, dbIDP, openCountP, l_openCountP, modeP, l_modeP, cardNoP, l_cardNoP, resDBP, res);
}
break;
case sysTrapDmGetAppInfoID: {
  // LocalID DmGetAppInfoID(DmOpenRef dbP)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  resid = DmGetAppInfoID(dbP ? l_dbP : 0);
  m68k_set_reg(M68K_REG_D0, resid);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetAppInfoID(dbP=0x%08X): 0x%08X", dbP, resid);
}
break;
case sysTrapDmGetDatabaseLockState: {
  // void DmGetDatabaseLockState(DmOpenRef dbR, out UInt8 *highest, out UInt32 *count, out UInt32 *busy)
  dbR = ARG32;
  l_dbR = emupalmos_trap_in(dbR, trap, 0);
  highest = ARG32;
  count = ARG32;
  busy = ARG32;
  DmGetDatabaseLockState(dbR ? l_dbR : 0, highest ? &l_highest : NULL, count ? &l_count : NULL, busy ? &l_busy : NULL);
  if (highest) m68k_write_memory_8(highest, l_highest);
  if (count) m68k_write_memory_32(count, l_count);
  if (busy) m68k_write_memory_32(busy, l_busy);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetDatabaseLockState(dbR=0x%08X, highest=0x%08X, count=0x%08X [%d], busy=0x%08X [%d])", dbR, highest, count, l_count, busy, l_busy);
}
break;
case sysTrapDmResetRecordStates: {
  // Err DmResetRecordStates(DmOpenRef dbP)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  res = DmResetRecordStates(dbP ? l_dbP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmResetRecordStates(dbP=0x%08X): %d", dbP, res);
}
break;
case sysTrapDmGetLastErr: {
  // Err DmGetLastErr(void)
  res = DmGetLastErr();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetLastErr(): %d", res);
}
break;
case sysTrapDmNumRecords: {
  // UInt16 DmNumRecords(DmOpenRef dbP)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  res = (UInt32)DmNumRecords(dbP ? l_dbP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNumRecords(dbP=0x%08X): %d", dbP, res);
}
break;
case sysTrapDmNumRecordsInCategory: {
  // UInt16 DmNumRecordsInCategory(DmOpenRef dbP, UInt16 category)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  category = ARG16;
  res = (UInt32)DmNumRecordsInCategory(dbP ? l_dbP : 0, (uint16_t)category);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNumRecordsInCategory(dbP=0x%08X, category=%d): %d", dbP, category, res);
}
break;
case sysTrapDmRecordInfo: {
  // Err DmRecordInfo(DmOpenRef dbP, UInt16 index, out UInt16 *attrP, out UInt32 *uniqueIDP, out LocalID *chunkIDP)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  index = ARG16;
  attrP = ARG32;
  uniqueIDP = ARG32;
  chunkIDP = ARG32;
  res = DmRecordInfo(dbP ? l_dbP : 0, index, attrP ? &l_attrP : NULL, uniqueIDP ? &l_uniqueIDP : NULL, chunkIDP ? &l_chunkIDP : NULL);
  if (attrP) m68k_write_memory_16(attrP, l_attrP);
  if (uniqueIDP) m68k_write_memory_32(uniqueIDP, l_uniqueIDP);
  if (chunkIDP) m68k_write_memory_32(chunkIDP, l_chunkIDP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmRecordInfo(dbP=0x%08X, index=%d, attrP=0x%08X [%d], uniqueIDP=0x%08X [%d], chunkIDP=0x%08X): %d", dbP, index, attrP, l_attrP, uniqueIDP, l_uniqueIDP, chunkIDP, res);
}
break;
case sysTrapDmSetRecordInfo: {
  // Err DmSetRecordInfo(DmOpenRef dbP, UInt16 index, in UInt16 *attrP, in UInt32 *uniqueIDP)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  index = ARG16;
  attrP = ARG32;
  if (attrP) l_attrP = m68k_read_memory_16(attrP);
  uniqueIDP = ARG32;
  if (uniqueIDP) l_uniqueIDP = m68k_read_memory_32(uniqueIDP);
  res = DmSetRecordInfo(dbP ? l_dbP : 0, index, attrP ? &l_attrP : NULL, uniqueIDP ? &l_uniqueIDP : NULL);
  if (attrP) m68k_write_memory_16(attrP, l_attrP);
  if (uniqueIDP) m68k_write_memory_32(uniqueIDP, l_uniqueIDP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSetRecordInfo(dbP=0x%08X, index=%d, attrP=0x%08X [%d], uniqueIDP=0x%08X [%d]): %d", dbP, index, attrP, l_attrP, uniqueIDP, l_uniqueIDP, res);
}
break;
case sysTrapDmMoveRecord: {
  // Err DmMoveRecord(DmOpenRef dbP, UInt16 from, UInt16 to)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  from = ARG16;
  to = ARG16;
  res = DmMoveRecord(dbP ? l_dbP : 0, from, to);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmMoveRecord(dbP=0x%08X, from=%d, to=%d): %d", dbP, from, to, res);
}
break;
case sysTrapDmNewRecord: {
  // MemHandle DmNewRecord(DmOpenRef dbP, inout UInt16 *atP, UInt32 size)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  atP = ARG32;
  if (atP) l_atP = m68k_read_memory_16(atP);
  size = ARG32;
  resmem = DmNewRecord(dbP ? l_dbP : 0, atP ? &l_atP : NULL, size);
  if (atP) m68k_write_memory_16(atP, l_atP);
  r_res = emupalmos_trap_out(resmem);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNewRecord(dbP=0x%08X, atP=0x%08X [%d], size=%d): 0x%08X", dbP, atP, l_atP, size, r_res);
}
break;
case sysTrapDmRemoveRecord: {
  // Err DmRemoveRecord(DmOpenRef dbP, UInt16 index)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  index = ARG16;
  res = DmRemoveRecord(dbP ? l_dbP : 0, index);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmRemoveRecord(dbP=0x%08X, index=%d): %d", dbP, index, res);
}
break;
case sysTrapDmDeleteRecord: {
  // Err DmDeleteRecord(DmOpenRef dbP, UInt16 index)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  index = ARG16;
  res = DmDeleteRecord(dbP ? l_dbP : 0, index);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmDeleteRecord(dbP=0x%08X, index=%d): %d", dbP, index, res);
}
break;
case sysTrapDmArchiveRecord: {
  // Err DmArchiveRecord(DmOpenRef dbP, UInt16 index)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  index = ARG16;
  res = DmArchiveRecord(dbP ? l_dbP : 0, index);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmArchiveRecord(dbP=0x%08X, index=%d): %d", dbP, index, res);
}
break;
case sysTrapDmNewHandle: {
  // MemHandle DmNewHandle(DmOpenRef dbP, UInt32 size)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  size = ARG32;
  resmem = DmNewHandle(dbP ? l_dbP : 0, size);
  r_res = emupalmos_trap_out(resmem);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNewHandle(dbP=0x%08X, size=%d): 0x%08X", dbP, size, r_res);
}
break;
case sysTrapDmRemoveSecretRecords: {
  // Err DmRemoveSecretRecords(DmOpenRef dbP)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  res = DmRemoveSecretRecords(dbP ? l_dbP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmRemoveSecretRecords(dbP=0x%08X): %d", dbP, res);
}
break;
case sysTrapDmFindRecordByID: {
  // Err DmFindRecordByID(DmOpenRef dbP, UInt32 uniqueID, out UInt16 *indexP)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uniqueID = ARG32;
  indexP = ARG32;
  res = DmFindRecordByID(dbP ? l_dbP : 0, uniqueID, indexP ? &l_indexP : NULL);
  if (indexP) m68k_write_memory_16(indexP, l_indexP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmFindRecordByID(dbP=0x%08X, uniqueID=%d, indexP=0x%08X [%d]): %d", dbP, uniqueID, indexP, l_indexP, res);
}
break;
case sysTrapDmQueryRecord: {
  // MemHandle DmQueryRecord(DmOpenRef dbP, UInt16 index)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  index = ARG16;
  resmem = DmQueryRecord(dbP ? l_dbP : 0, index);
  r_res = emupalmos_trap_out(resmem);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmQueryRecord(dbP=0x%08X, index=%d): 0x%08X", dbP, index, r_res);
}
break;
case sysTrapDmGetRecord: {
  // MemHandle DmGetRecord(DmOpenRef dbP, UInt16 index)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  index = ARG16;
  resmem = DmGetRecord(dbP ? l_dbP : 0, index);
  r_res = emupalmos_trap_out(resmem);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetRecord(dbP=0x%08X, index=%d): 0x%08X", dbP, index, r_res);
}
break;
case sysTrapDmQueryNextInCategory: {
  // MemHandle DmQueryNextInCategory(DmOpenRef dbP, inout UInt16 *indexP, UInt16 category)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  indexP = ARG32;
  if (indexP) l_indexP = m68k_read_memory_16(indexP);
  category = ARG16;
  resmem = DmQueryNextInCategory(dbP ? l_dbP : 0, indexP ? &l_indexP : NULL, (uint16_t)category);
  if (indexP) m68k_write_memory_16(indexP, l_indexP);
  r_res = emupalmos_trap_out(resmem);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmQueryNextInCategory(dbP=0x%08X, indexP=0x%08X [%d], category=%d): 0x%08X", dbP, indexP, l_indexP, category, r_res);
}
break;
case sysTrapDmPositionInCategory: {
  // UInt16 DmPositionInCategory(DmOpenRef dbP, UInt16 index, UInt16 category)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  index = ARG16;
  category = ARG16;
  res = (UInt32)DmPositionInCategory(dbP ? l_dbP : 0, index, (uint16_t)category);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmPositionInCategory(dbP=0x%08X, index=%d, category=%d): %d", dbP, index, category, res);
}
break;
case sysTrapDmSeekRecordInCategory: {
  // Err DmSeekRecordInCategory(DmOpenRef dbP, inout UInt16 *indexP, UInt16 offset, Int16 direction, UInt16 category)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  indexP = ARG32;
  if (indexP) l_indexP = m68k_read_memory_16(indexP);
  offset = ARG16;
  direction = ARG16;
  category = ARG16;
  res = DmSeekRecordInCategory(dbP ? l_dbP : 0, indexP ? &l_indexP : NULL, (uint16_t)offset, direction, (uint16_t)category);
  if (indexP) m68k_write_memory_16(indexP, l_indexP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSeekRecordInCategory(dbP=0x%08X, indexP=0x%08X [%d], offset=%d, direction=%d, category=%d): %d", dbP, indexP, l_indexP, offset, direction, category, res);
}
break;
case sysTrapDmResizeRecord: {
  // MemHandle DmResizeRecord(DmOpenRef dbP, UInt16 index, UInt32 newSize)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  index = ARG16;
  newSize = ARG32;
  resmem = DmResizeRecord(dbP ? l_dbP : 0, index, newSize);
  r_res = emupalmos_trap_out(resmem);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmResizeRecord(dbP=0x%08X, index=%d, newSize=%d): %p", dbP, index, newSize, resmem);
}
break;
case sysTrapDmReleaseRecord: {
  // Err DmReleaseRecord(DmOpenRef dbP, UInt16 index, Boolean dirty)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  index = ARG16;
  dirty = ARG8;
  res = DmReleaseRecord(dbP ? l_dbP : 0, index, dirty);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmReleaseRecord(dbP=0x%08X, index=%d, dirty=%d): %d", dbP, index, dirty, res);
}
break;
case sysTrapDmMoveCategory: {
  // Err DmMoveCategory(DmOpenRef dbP, UInt16 toCategory, UInt16 fromCategory, Boolean dirty)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  toCategory = ARG16;
  fromCategory = ARG16;
  dirty = ARG8;
  res = DmMoveCategory(dbP ? l_dbP : 0, toCategory, fromCategory, dirty);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmMoveCategory(dbP=0x%08X, toCategory=%d, fromCategory=%d, dirty=%d): %d", dbP, toCategory, fromCategory, dirty, res);
}
break;
case sysTrapDmDeleteCategory: {
  // Err DmDeleteCategory(DmOpenRef dbR, UInt16 categoryNum)
  dbR = ARG32;
  l_dbR = emupalmos_trap_in(dbR, trap, 0);
  categoryNum = ARG16;
  res = DmDeleteCategory(dbR ? l_dbR : 0, categoryNum);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmDeleteCategory(dbR=0x%08X, categoryNum=%d): %d", dbR, categoryNum, res);
}
break;
case sysTrapDmWriteCheck: {
  // Err DmWriteCheck(out void *recordP, UInt32 offset, UInt32 bytes)
  recordP = ARG32;
  s_recordP = emupalmos_trap_in(recordP, trap, 0);
  offset = ARG32;
  bytes = ARG32;
  res = DmWriteCheck(recordP ? s_recordP : NULL, offset, bytes);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmWriteCheck(recordP=0x%08X, offset=%d, bytes=%d): %d", recordP, offset, bytes, res);
}
break;
case sysTrapDmWrite: {
  // Err DmWrite(out void *recordP, UInt32 offset, in void *srcP, UInt32 bytes)
  recordP = ARG32;
  s_recordP = emupalmos_trap_in(recordP, trap, 0);
  offset = ARG32;
  srcP = ARG32;
  s_srcP = emupalmos_trap_in(srcP, trap, 2);
  bytes = ARG32;
  if (emupalmos_check_address(recordP + offset, bytes, 0) && emupalmos_check_address(srcP, bytes, 1)) {
    res = DmWrite(recordP ? s_recordP : NULL, offset, srcP ? s_srcP : NULL, bytes);
  } else {
    res = dmErrInvalidParam;
  }
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmWrite(recordP=0x%08X, offset=%d, srcP=0x%08X, bytes=%d): %d", recordP, offset, srcP, bytes, res);
}
break;
case sysTrapDmStrCopy: {
  // Err DmStrCopy(out void *recordP, UInt32 offset, in srcP)
  recordP = ARG32;
  s_recordP = emupalmos_trap_in(recordP, trap, 0);
  offset = ARG32;
  srcP = ARG32;
  s_srcP = emupalmos_trap_in(srcP, trap, 2);
  res = DmStrCopy(recordP ? s_recordP : NULL, offset, srcP ? (char *)s_srcP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmStrCopy(recordP=0x%08X, offset=%d, srcP=0x%08X [%s]): %d", recordP, offset, srcP, (char *)s_srcP, res);
}
break;
case sysTrapDmSet: {
  // Err DmSet(out void *recordP, UInt32 offset, UInt32 bytes, UInt8 value)
  recordP = ARG32;
  s_recordP = emupalmos_trap_in(recordP, trap, 0);
  offset = ARG32;
  bytes = ARG32;
  value = ARG8;
  res = DmSet(recordP ? s_recordP : NULL, offset, bytes, (uint8_t)value);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSet(recordP=0x%08X, offset=%d, bytes=%d, value=%d): %d", recordP, offset, bytes, value, res);
}
break;
case sysTrapDmGetResource: {
  // MemHandle DmGetResource(DmResType type, DmResID resID)
  type = ARG32;
  pumpkin_id2s(type, buf);
  resID = ARG16;
  resmem = DmGetResource(type, resID);
  r_res = emupalmos_trap_out(resmem);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetResource(type=%s, resID=%d): 0x%08X", buf, resID, r_res);
}
break;
case sysTrapDmGet1Resource: {
  // MemHandle DmGet1Resource(DmResType type, DmResID resID)
  type = ARG32;
  pumpkin_id2s(type, buf);
  resID = ARG16;
  resmem = DmGet1Resource(type, resID);
  r_res = emupalmos_trap_out(resmem);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGet1Resource(type=%s, resID=%d): 0x%08X", buf, resID, r_res);
}
break;
case sysTrapDmReleaseResource: {
  // Err DmReleaseResource(MemHandle resourceH)
  resourceH = ARG32;
  l_resourceH = emupalmos_trap_in(resourceH, trap, 0);
  res = DmReleaseResource(resourceH ? l_resourceH : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmReleaseResource(resourceH=0x%08X): %d", resourceH, res);
}
break;
case sysTrapDmResizeResource: {
  // MemHandle DmResizeResource(MemHandle resourceH, UInt32 newSize)
  resourceH = ARG32;
  l_resourceH = emupalmos_trap_in(resourceH, trap, 0);
  newSize = ARG32;
  resmem = DmResizeResource(resourceH ? l_resourceH : 0, newSize);
  r_res = emupalmos_trap_out(resmem);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmResizeResource(resourceH=0x%08X, newSize=%d): %p", resourceH, newSize, resmem);
}
break;
case sysTrapDmNextOpenResDatabase: {
  // DmOpenRef DmNextOpenResDatabase(DmOpenRef dbP)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  resor = DmNextOpenResDatabase(dbP ? l_dbP : 0);
  r_res = emupalmos_trap_out(resor);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNextOpenResDatabase(dbP=0x%08X): 0x%08X", dbP, r_res);
}
break;
case sysTrapDmFindResourceType: {
  // UInt16 DmFindResourceType(DmOpenRef dbP, DmResType resType, UInt16 typeIndex)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  resType = ARG32;
  typeIndex = ARG16;
  res = (UInt32)DmFindResourceType(dbP ? l_dbP : 0, resType, typeIndex);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmFindResourceType(dbP=0x%08X, resType=%d, typeIndex=%d): %d", dbP, resType, typeIndex, res);
}
break;
case sysTrapDmFindResource: {
  // UInt16 DmFindResource(DmOpenRef dbP, DmResType resType, DmResID resID, MemHandle resH)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  resType = ARG32;
  resID = ARG16;
  resH = ARG32;
  l_resH = emupalmos_trap_in(resH, trap, 3);
  res = (UInt32)DmFindResource(dbP ? l_dbP : 0, resType, resID, resH ? l_resH : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmFindResource(dbP=0x%08X, resType=%d, resID=%d, resH=%d): %d", dbP, resType, resID, resH, res);
}
break;
case sysTrapDmNumResources: {
  // UInt16 DmNumResources(DmOpenRef dbP)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  res = (UInt32)DmNumResources(dbP ? l_dbP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNumResources(dbP=0x%08X): %d", dbP, res);
}
break;
case sysTrapDmResourceInfo: {
  // Err DmResourceInfo(DmOpenRef dbP, UInt16 index, out DmResType *resTypeP, out DmResID *resIDP, out LocalID *chunkLocalIDP)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  index = ARG16;
  resTypeP = ARG32;
  resIDP = ARG32;
  chunkLocalIDP = ARG32;
  res = DmResourceInfo(dbP ? l_dbP : 0, index, resTypeP ? &l_resTypeP : NULL, resIDP ? &l_resIDP : NULL, chunkLocalIDP ? &l_chunkLocalIDP : NULL);
  if (resTypeP) m68k_write_memory_32(resTypeP, l_resTypeP);
  if (resIDP) m68k_write_memory_16(resIDP, l_resIDP);
  if (chunkLocalIDP) m68k_write_memory_32(chunkLocalIDP, l_chunkLocalIDP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmResourceInfo(dbP=0x%08X, index=%d, resTypeP=0x%08X, resIDP=0x%08X, chunkLocalIDP=0x%08X): %d", dbP, index, resTypeP, resIDP, chunkLocalIDP, res);
}
break;
case sysTrapDmSetResourceInfo: {
  // Err DmSetResourceInfo(DmOpenRef dbP, UInt16 index, in DmResType *resTypeP, in DmResID *resIDP)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  index = ARG16;
  resTypeP = ARG32;
  if (resTypeP) l_resTypeP = m68k_read_memory_32(resTypeP);
  resIDP = ARG32;
  if (resIDP) l_resIDP = m68k_read_memory_16(resIDP);
  res = DmSetResourceInfo(dbP ? l_dbP : 0, index, resTypeP ? &l_resTypeP : NULL, resIDP ? &l_resIDP : NULL);
  if (resTypeP) m68k_write_memory_32(resTypeP, l_resTypeP);
  if (resIDP) m68k_write_memory_16(resIDP, l_resIDP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSetResourceInfo(dbP=0x%08X, index=%d, resTypeP=0x%08X, resIDP=0x%08X): %d", dbP, index, resTypeP, resIDP, res);
}
break;
case sysTrapDmNewResource: {
  // MemHandle DmNewResource(DmOpenRef dbP, DmResType resType, DmResID resID, UInt32 size)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  resType = ARG32;
  resID = ARG16;
  size = ARG32;
  resmem = DmNewResource(dbP ? l_dbP : 0, resType, resID, size);
  r_res = emupalmos_trap_out(resmem);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNewResource(dbP=0x%08X, resType=%d, resID=%d, size=%d): %p", dbP, resType, resID, size, resmem);
}
break;
case sysTrapDmRemoveResource: {
  // Err DmRemoveResource(DmOpenRef dbP, UInt16 index)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  index = ARG16;
  res = DmRemoveResource(dbP ? l_dbP : 0, index);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmRemoveResource(dbP=0x%08X, index=%d): %d", dbP, index, res);
}
break;
case sysTrapDmGetResourceIndex: {
  // MemHandle DmGetResourceIndex(DmOpenRef dbP, UInt16 index)
  dbP = ARG32;
  l_dbP = emupalmos_trap_in(dbP, trap, 0);
  index = ARG16;
  resmem = DmGetResourceIndex(dbP ? l_dbP : 0, index);
  r_res = emupalmos_trap_out(resmem);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetResourceIndex(dbP=0x%08X, index=%d): 0x%08X", dbP, index, r_res);
}
break;
case sysTrapWinValidateHandle: {
  // Boolean WinValidateHandle(WinHandle winHandle)
  winHandle = ARG32;
  l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  res = (UInt32)WinValidateHandle(winHandle ? l_winHandle : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinValidateHandle(winHandle=0x%08X): %d", winHandle, res);
}
break;
case sysTrapWinInitializeWindow: {
  // void WinInitializeWindow(WinHandle winHandle)
  winHandle = ARG32;
  l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  WinInitializeWindow(winHandle ? l_winHandle : 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinInitializeWindow(winHandle=0x%08X)", winHandle);
}
break;
case sysTrapWinAddWindow: {
  // void WinAddWindow(WinHandle winHandle)
  winHandle = ARG32;
  l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  WinAddWindow(winHandle ? l_winHandle : 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinAddWindow(winHandle=0x%08X)", winHandle);
}
break;
case sysTrapWinRemoveWindow: {
  // void WinRemoveWindow(WinHandle winHandle)
  winHandle = ARG32;
  l_winHandle = winHandle ? (WinHandle)(ram + winHandle) : NULL;
  WinRemoveWindow(winHandle ? l_winHandle : 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinRemoveWindow(winHandle=0x%08X)", winHandle);
}
break;
case sysTrapWinSetActiveWindow: {
  // void WinSetActiveWindow(WinHandle winHandle)
  winHandle = ARG32;
  l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  WinSetActiveWindow(winHandle ? l_winHandle : 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetActiveWindow(winHandle=0x%08X)", winHandle);
}
break;
case sysTrapWinSetDrawWindow: {
  // WinHandle WinSetDrawWindow(WinHandle winHandle)
  winHandle = ARG32;
  l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  reswh = WinSetDrawWindow(winHandle ? l_winHandle : 0);
  r_res = emupalmos_trap_out(reswh);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetDrawWindow(winHandle=0x%08X): 0x%08X", winHandle, r_res);
}
break;
case sysTrapWinGetDrawWindow: {
  // WinHandle WinGetDrawWindow(void)
  reswh = WinGetDrawWindow();
  r_res = emupalmos_trap_out(reswh);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetDrawWindow(): 0x%08X", r_res);
}
break;
case sysTrapWinGetActiveWindow: {
  // WinHandle WinGetActiveWindow(void)
  reswh = WinGetActiveWindow();
  r_res = emupalmos_trap_out(reswh);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetActiveWindow(): 0x%08X", r_res);
}
break;
case sysTrapWinGetDisplayWindow: {
  // WinHandle WinGetDisplayWindow(void)
  reswh = WinGetDisplayWindow();
  r_res = emupalmos_trap_out(reswh);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetDisplayWindow(): 0x%08X", r_res);
}
break;
case sysTrapWinGetFirstWindow: {
  // WinHandle WinGetFirstWindow(void)
  reswh = WinGetFirstWindow();
  r_res = emupalmos_trap_out(reswh);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetFirstWindow(): 0x%08X", r_res);
}
break;
case sysTrapWinEnableWindow: {
  // void WinEnableWindow(WinHandle winHandle)
  winHandle = ARG32;
  l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  WinEnableWindow(winHandle ? l_winHandle : 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinEnableWindow(winHandle=0x%08X)", winHandle);
}
break;
case sysTrapWinDisableWindow: {
  // void WinDisableWindow(WinHandle winHandle)
  winHandle = ARG32;
  l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  WinDisableWindow(winHandle ? l_winHandle : 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDisableWindow(winHandle=0x%08X)", winHandle);
}
break;
case sysTrapWinGetWindowFrameRect: {
  // void WinGetWindowFrameRect(WinHandle winHandle, out RectangleType *r)
  winHandle = ARG32;
  l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  r = ARG32;
  WinGetWindowFrameRect(winHandle ? l_winHandle : 0, r ? &l_r : NULL);
  encode_rectangle(r, &l_r);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetWindowFrameRect(winHandle=0x%08X, r=0x%08X [%d,%d,%d,%d])", winHandle, r, l_r.topLeft.x, l_r.topLeft.y, l_r.extent.x, l_r.extent.y);
}
break;
case sysTrapWinDrawWindowFrame: {
  // void WinDrawWindowFrame(void)
  WinDrawWindowFrame();
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawWindowFrame()");
}
break;
case sysTrapWinEraseWindow: {
  // void WinEraseWindow(void)
  WinEraseWindow();
  debug(DEBUG_TRACE, "EmuPalmOS", "WinEraseWindow()");
}
break;
case sysTrapWinSaveBits: {
  // WinHandle WinSaveBits(in RectangleType *source, out UInt16 *error)
  source32 = ARG32;
  decode_rectangle(source32, &l_source);
  error = ARG32;
  reswh = WinSaveBits(source32 ? &l_source : NULL, error ? &l_error : NULL);
  if (error) m68k_write_memory_16(error, l_error);
  r_res = emupalmos_trap_out(reswh);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSaveBits(source=0x%08X [%d,%d,%d,%d], error=0x%08X [%d]): 0x%08X", source32, l_source.topLeft.x, l_source.topLeft.y, l_source.extent.x, l_source.extent.y, error, l_error, r_res);
}
break;
case sysTrapWinRestoreBits: {
  // void WinRestoreBits(WinHandle winHandle, Coord destX, Coord destY)
  winHandle = ARG32;
  l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  destX = ARG16;
  destY = ARG16;
  WinRestoreBits(winHandle ? l_winHandle : 0, destX, destY);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinRestoreBits(winHandle=0x%08X, destX=%d, destY=%d)", winHandle, destX, destY);
}
break;
case sysTrapWinCopyRectangle: {
  // void WinCopyRectangle(WinHandle srcWin, WinHandle dstWin, in RectangleType *srcRect, Coord destX, Coord destY, WinDrawOperation mode)
  srcWin = ARG32;
  l_srcWin = emupalmos_trap_in(srcWin, trap, 0);
  dstWin = ARG32;
  l_dstWin = emupalmos_trap_in(dstWin, trap, 1);
  srcRect = ARG32;
  decode_rectangle(srcRect, &l_srcRect);
  destX = ARG16;
  destY = ARG16;
  mode = ARG8;
  WinCopyRectangle(srcWin ? l_srcWin : 0, dstWin ? l_dstWin : 0, srcRect ? &l_srcRect : NULL, destX, destY, (uint8_t)mode);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinCopyRectangle(srcWin=0x%08X, dstWin=0x%08X, srcRect=0x%08X [%d,%d,%d,%d], destX=%d, destY=%d, mode=%d)", srcWin, dstWin, srcRect, l_srcRect.topLeft.x, l_srcRect.topLeft.y, l_srcRect.extent.x, l_srcRect.extent.y, destX, destY, mode);
}
break;
case sysTrapWinScrollRectangle: {
  // void WinScrollRectangle(in RectangleType *rP, WinDirectionType direction, Coord distance, out RectangleType *vacatedP)
  rP = ARG32;
  decode_rectangle(rP, &l_rP);
  direction = ARG8;
  distance = ARG16;
  vacatedP = ARG32;
  WinScrollRectangle(rP ? &l_rP : NULL, (uint8_t)direction, distance, vacatedP ? &l_vacatedP : NULL);
  encode_rectangle(vacatedP, &l_vacatedP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinScrollRectangle(rP=0x%08X [%d,%d,%d,%d], direction=%d, distance=%d, vacatedP=0x%08X [%d,%d,%d,%d])", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y, direction, distance, vacatedP, l_vacatedP.topLeft.x, l_vacatedP.topLeft.y, l_vacatedP.extent.x, l_vacatedP.extent.y);
}
break;
case sysTrapWinGetDisplayExtent: {
  // void WinGetDisplayExtent(out Coord *extentX, out Coord *extentY)
  extentX = ARG32;
  extentY = ARG32;
  WinGetDisplayExtent(extentX ? &l_extentX : NULL, extentY ? &l_extentY : NULL);
  if (extentX) m68k_write_memory_16(extentX, l_extentX);
  if (extentY) m68k_write_memory_16(extentY, l_extentY);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetDisplayExtent(extentX=0x%08X [%d], extentY=0x%08X [%d])", extentX, l_extentX, extentY, l_extentY);
}
break;
case sysTrapWinGetDrawWindowBounds: {
  // void WinGetDrawWindowBounds(out RectangleType *rP)
  rP = ARG32;

  WinGetDrawWindowBounds(rP ? &l_rP : NULL);
  encode_rectangle(rP, &l_rP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetDrawWindowBounds(rP=0x%08X [%d,%d,%d,%d])", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinGetBounds: {
  // void WinGetBounds(WinHandle winH, out RectangleType *rP)
  winH = ARG32;
  l_winH = emupalmos_trap_in(winH, trap, 0);
  rP = ARG32;

  WinGetBounds(winH ? l_winH : 0, rP ? &l_rP : NULL);
  encode_rectangle(rP, &l_rP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetBounds(winH=0x%08X, rP=0x%08X [%d,%d,%d,%d])", winH, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinSetBounds: {
  // void WinSetBounds(WinHandle winHandle, in RectangleType *rP)
  winHandle = ARG32;
  l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  WinSetBounds(winHandle ? l_winHandle : 0, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetBounds(winHandle=0x%08X, rP=0x%08X [%d,%d,%d,%d])", winHandle, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinGetWindowExtent: {
  // void WinGetWindowExtent(out Coord *extentX, out Coord *extentY)
  extentX = ARG32;
  extentY = ARG32;
  WinGetWindowExtent(extentX ? &l_extentX : NULL, extentY ? &l_extentY : NULL);
  if (extentX) m68k_write_memory_16(extentX, l_extentX);
  if (extentY) m68k_write_memory_16(extentY, l_extentY);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetWindowExtent(extentX=0x%08X [%d], extentY=0x%08X [%d])", extentX, l_extentX, extentY, l_extentY);
}
break;
case sysTrapWinDisplayToWindowPt: {
  // void WinDisplayToWindowPt(inout Coord *extentX, inout Coord *extentY)
  extentX = ARG32;

  if (extentX) l_extentX = m68k_read_memory_16(extentX);
  extentY = ARG32;

  if (extentY) l_extentY = m68k_read_memory_16(extentY);
  WinDisplayToWindowPt(extentX ? &l_extentX : NULL, extentY ? &l_extentY : NULL);
  if (extentX) m68k_write_memory_16(extentX, l_extentX);
  if (extentY) m68k_write_memory_16(extentY, l_extentY);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDisplayToWindowPt(extentX=0x%08X [%d], extentY=0x%08X [%d])", extentX, l_extentX, extentY, l_extentY);
}
break;
case sysTrapWinWindowToDisplayPt: {
  // void WinWindowToDisplayPt(inout Coord *extentX, inout Coord *extentY)
  extentX = ARG32;

  if (extentX) l_extentX = m68k_read_memory_16(extentX);
  extentY = ARG32;

  if (extentY) l_extentY = m68k_read_memory_16(extentY);
  WinWindowToDisplayPt(extentX ? &l_extentX : NULL, extentY ? &l_extentY : NULL);
  if (extentX) m68k_write_memory_16(extentX, l_extentX);
  if (extentY) m68k_write_memory_16(extentY, l_extentY);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinWindowToDisplayPt(extentX=0x%08X [%d], extentY=0x%08X [%d])", extentX, l_extentX, extentY, l_extentY);
}
break;
case sysTrapWinGetBitmap: {
  // BitmapType *WinGetBitmap(WinHandle winHandle)
  winHandle = ARG32;
  l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  resbm = WinGetBitmap(winHandle ? l_winHandle : 0);
  r_res = emupalmos_trap_out(resbm);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetBitmap(winHandle=0x%08X): 0x%08X", winHandle, r_res);
}
break;
case sysTrapWinGetClip: {
  // void WinGetClip(out RectangleType *rP)
  rP = ARG32;

  WinGetClip(rP ? &l_rP : NULL);
  encode_rectangle(rP, &l_rP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetClip(rP=0x%08X [%d,%d,%d,%d])", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinSetClip: {
  // void WinSetClip(in RectangleType *rP)
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  WinSetClip(rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetClip(rP=0x%08X [%d,%d,%d,%d])", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinResetClip: {
  // void WinResetClip(void)
  WinResetClip();
  debug(DEBUG_TRACE, "EmuPalmOS", "WinResetClip()");
}
break;
case sysTrapWinClipRectangle: {
  // void WinClipRectangle(out RectangleType *rP)
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  WinClipRectangle(rP ? &l_rP : NULL);
  encode_rectangle(rP, &l_rP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinClipRectangle(rP=0x%08X [%d,%d,%d,%d])", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinModal: {
  // Boolean WinModal(WinHandle winHandle)
  winHandle = ARG32;
  l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  res = (UInt32)WinModal(winHandle ? l_winHandle : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinModal(winHandle=0x%08X): %d", winHandle, res);
}
break;
case sysTrapWinGetPixel: {
  // IndexedColorType WinGetPixel(Coord x, Coord y)
  x = ARG16;
  y = ARG16;
  resic = WinGetPixel((uint16_t)x, (uint16_t)y);
  m68k_set_reg(M68K_REG_D0, resic);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetPixel(x=%d, y=%d): %d", x, y, resic);
}
break;
case sysTrapWinGetPixelRGB: {
  // Err WinGetPixelRGB(Coord x, Coord y, out RGBColorType *rgbP)
  x = ARG16;
  y = ARG16;
  rgbP = ARG32;

  res = WinGetPixelRGB((uint16_t)x, (uint16_t)y, rgbP ? &l_rgbP : NULL);
  encode_rgb(rgbP, &l_rgbP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetPixelRGB(x=%d, y=%d, rgbP=0x%08X): %d", x, y, rgbP, res);
}
break;
case sysTrapWinPaintPixel: {
  // void WinPaintPixel(Coord x, Coord y)
  x = ARG16;
  y = ARG16;
  WinPaintPixel((uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintPixel(x=%d, y=%d)", x, y);
}
break;
case sysTrapWinPaintPixels: {
  // void WinPaintPixels(UInt16 numPoints, PointType pts[])
  numPoints = ARG16;
  pts = ARG32;
  emupalmos_trap_in(pts, trap, 1);
  for (i = 0; i < numPoints; i++) {
    x = m68k_read_memory_16(pts);
    pts += 2;
    y = m68k_read_memory_16(pts);
    pts += 2;
    WinPaintPixel(x, y);
  }
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintPixels(numPoints=%u, pts=0x%08X)", numPoints, pts);
}
break;
case sysTrapWinDrawPixel: {
  // void WinDrawPixel(Coord x, Coord y)
  x = ARG16;
  y = ARG16;
  WinDrawPixel((uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawPixel(x=%d, y=%d)", x, y);
}
break;
case sysTrapWinErasePixel: {
  // void WinErasePixel(Coord x, Coord y)
  x = ARG16;
  y = ARG16;
  WinErasePixel((uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinErasePixel(x=%d, y=%d)", x, y);
}
break;
case sysTrapWinInvertPixel: {
  // void WinInvertPixel(Coord x, Coord y)
  x = ARG16;
  y = ARG16;
  WinInvertPixel((uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinInvertPixel(x=%d, y=%d)", x, y);
}
break;
case sysTrapWinPaintLine: {
  // void WinPaintLine(Coord x1, Coord y1, Coord x2, Coord y2)
  x1 = ARG16;
  y1 = ARG16;
  x2 = ARG16;
  y2 = ARG16;
  WinPaintLine(x1, y1, x2, y2);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintLine(x1=%d, y1=%d, x2=%d, y2=%d)", x1, y1, x2, y2);
}
break;
case sysTrapWinDrawLine: {
  // void WinDrawLine(Coord x1, Coord y1, Coord x2, Coord y2)
  x1 = ARG16;
  y1 = ARG16;
  x2 = ARG16;
  y2 = ARG16;
  WinDrawLine(x1, y1, x2, y2);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawLine(x1=%d, y1=%d, x2=%d, y2=%d)", x1, y1, x2, y2);
}
break;
case sysTrapWinDrawGrayLine: {
  // void WinDrawGrayLine(Coord x1, Coord y1, Coord x2, Coord y2)
  x1 = ARG16;
  y1 = ARG16;
  x2 = ARG16;
  y2 = ARG16;
  WinDrawGrayLine(x1, y1, x2, y2);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawGrayLine(x1=%d, y1=%d, x2=%d, y2=%d)", x1, y1, x2, y2);
}
break;
case sysTrapWinEraseLine: {
  // void WinEraseLine(Coord x1, Coord y1, Coord x2, Coord y2)
  x1 = ARG16;
  y1 = ARG16;
  x2 = ARG16;
  y2 = ARG16;
  WinEraseLine(x1, y1, x2, y2);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinEraseLine(x1=%d, y1=%d, x2=%d, y2=%d)", x1, y1, x2, y2);
}
break;
case sysTrapWinInvertLine: {
  // void WinInvertLine(Coord x1, Coord y1, Coord x2, Coord y2)
  x1 = ARG16;
  y1 = ARG16;
  x2 = ARG16;
  y2 = ARG16;
  WinInvertLine(x1, y1, x2, y2);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinInvertLine(x1=%d, y1=%d, x2=%d, y2=%d)", x1, y1, x2, y2);
}
break;
case sysTrapWinFillLine: {
  // void WinFillLine(Coord x1, Coord y1, Coord x2, Coord y2)
  x1 = ARG16;
  y1 = ARG16;
  x2 = ARG16;
  y2 = ARG16;
  WinFillLine(x1, y1, x2, y2);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinFillLine(x1=%d, y1=%d, x2=%d, y2=%d)", x1, y1, x2, y2);
}
break;
case sysTrapWinPaintRectangle: {
  // void WinPaintRectangle(in RectangleType *rP, UInt16 cornerDiam)
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  cornerDiam = ARG16;
  WinPaintRectangle(rP ? &l_rP : NULL, cornerDiam);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintRectangle(rP=0x%08X [%d,%d,%d,%d], cornerDiam=%d)", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y, cornerDiam);
}
break;
case sysTrapWinDrawRectangle: {
  // void WinDrawRectangle(in RectangleType *rP, UInt16 cornerDiam)
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  cornerDiam = ARG16;
  WinDrawRectangle(rP ? &l_rP : NULL, cornerDiam);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawRectangle(rP=0x%08X [%d,%d,%d,%d], cornerDiam=%d)", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y, cornerDiam);
}
break;
case sysTrapWinEraseRectangle: {
  // void WinEraseRectangle(in RectangleType *rP, UInt16 cornerDiam)
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  cornerDiam = ARG16;
  WinEraseRectangle(rP ? &l_rP : NULL, cornerDiam);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinEraseRectangle(rP=0x%08X [%d,%d,%d,%d], cornerDiam=%d)", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y, cornerDiam);
}
break;
case sysTrapWinInvertRectangle: {
  // void WinInvertRectangle(in RectangleType *rP, UInt16 cornerDiam)
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  cornerDiam = ARG16;
  WinInvertRectangle(rP ? &l_rP : NULL, cornerDiam);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinInvertRectangle(rP=0x%08X [%d,%d,%d,%d], cornerDiam=%d)", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y, cornerDiam);
}
break;
case sysTrapWinFillRectangle: {
  // void WinFillRectangle(in RectangleType *rP, UInt16 cornerDiam)
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  cornerDiam = ARG16;
  WinFillRectangle(rP ? &l_rP : NULL, cornerDiam);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinFillRectangle(rP=0x%08X [%d,%d,%d,%d], cornerDiam=%d)", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y, cornerDiam);
}
break;
case sysTrapWinPaintRectangleFrame: {
  // void WinPaintRectangleFrame(FrameType frame, in RectangleType *rP)
  frame = ARG16;
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  WinPaintRectangleFrame(frame, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintRectangleFrame(frame=%d, rP=0x%08X [%d,%d,%d,%d])", frame, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinDrawRectangleFrame: {
  // void WinDrawRectangleFrame(FrameType frame, in RectangleType *rP)
  frame = ARG16;
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  WinDrawRectangleFrame(frame, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawRectangleFrame(frame=%d, rP=0x%08X [%d,%d,%d,%d])", frame, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinDrawGrayRectangleFrame: {
  // void WinDrawGrayRectangleFrame(FrameType frame, in RectangleType *rP)
  frame = ARG16;
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  WinDrawGrayRectangleFrame(frame, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawGrayRectangleFrame(frame=%d, rP=0x%08X [%d,%d,%d,%d])", frame, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinEraseRectangleFrame: {
  // void WinEraseRectangleFrame(FrameType frame, in RectangleType *rP)
  frame = ARG16;
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  WinEraseRectangleFrame(frame, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinEraseRectangleFrame(frame=%d, rP=0x%08X [%d,%d,%d,%d])", frame, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinInvertRectangleFrame: {
  // void WinInvertRectangleFrame(FrameType frame, in RectangleType *rP)
  frame = ARG16;
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  WinInvertRectangleFrame(frame, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinInvertRectangleFrame(frame=%d, rP=0x%08X [%d,%d,%d,%d])", frame, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinDrawBitmap: {
  // void WinDrawBitmap(in bitmapP, Coord x, Coord y)
  bitmapP = ARG32;
  //l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  l_bitmapP = emupalmos_trap_in(bitmapP, trap, 0);
  x = ARG16;
  y = ARG16;
  WinDrawBitmap(bitmapP ? l_bitmapP : NULL, (uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawBitmap(bitmapP=0x%08X, x=%d, y=%d)", bitmapP, x, y);
}
break;
case sysTrapWinPaintBitmap: {
  // void WinPaintBitmap(in bitmapP, Coord x, Coord y)
  bitmapP = ARG32;
  //l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  l_bitmapP = emupalmos_trap_in(bitmapP, trap, 0);
  x = ARG16;
  y = ARG16;
  WinPaintBitmap(bitmapP ? l_bitmapP : NULL, (uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintBitmap(bitmapP=0x%08X, x=%d, y=%d)", bitmapP, x, y);
}
break;
case sysTrapWinDrawChar: {
  // void WinDrawChar(WChar theChar, Coord x, Coord y)
  theChar = ARG16;
  x = ARG16;
  y = ARG16;
  WinDrawChar(theChar, (uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawChar(theChar=%d, x=%d, y=%d)", theChar, x, y);
}
break;
case sysTrapWinDrawChars: {
  // void WinDrawChars(in Char *chars, Int16 len, Coord x, Coord y)
  chars = ARG32;
  s_chars = emupalmos_trap_in(chars, trap, 0);
  len = ARG16;
  x = ARG16;
  y = ARG16;
  if (s_chars) WinDrawChars(s_chars, len, (uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawChars(chars=0x%08X %p [%.*s], len=%d, x=%d, y=%d)", chars, s_chars, len, s_chars, len, x, y);
}
break;
case sysTrapWinPaintChar: {
  // void WinPaintChar(WChar theChar, Coord x, Coord y)
  theChar = ARG16;
  x = ARG16;
  y = ARG16;
  WinPaintChar(theChar, (uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintChar(theChar=%d, x=%d, y=%d)", theChar, x, y);
}
break;
case sysTrapWinPaintChars: {
  // void WinPaintChars(in Char *chars, Int16 len, Coord x, Coord y)
  chars = ARG32;
  s_chars = emupalmos_trap_in(chars, trap, 0);
  len = ARG16;
  x = ARG16;
  y = ARG16;
  if (s_chars) WinPaintChars(s_chars, len, (uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintChars(chars=0x%08X [%s], len=%d, x=%d, y=%d)", chars, s_chars, len, x, y);
}
break;
case sysTrapWinDrawInvertedChars: {
  // void WinDrawInvertedChars(in Char *chars, Int16 len, Coord x, Coord y)
  chars = ARG32;
  s_chars = emupalmos_trap_in(chars, trap, 0);
  len = ARG16;
  x = ARG16;
  y = ARG16;
  if (s_chars) WinDrawInvertedChars(s_chars, len, (uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawInvertedChars(chars=0x%08X [%s], len=%d, x=%d, y=%d)", chars, s_chars, len, x, y);
}
break;
case sysTrapWinDrawTruncChars: {
  // void WinDrawTruncChars(in Char *chars, Int16 len, Coord x, Coord y, Coord maxWidth)
  chars = ARG32;
  s_chars = emupalmos_trap_in(chars, trap, 0);
  len = ARG16;
  x = ARG16;
  y = ARG16;
  maxWidth = ARG16;
  if (s_chars) WinDrawTruncChars(s_chars, len, (uint16_t)x, (uint16_t)y, maxWidth);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawTruncChars(chars=0x%08X [%s], len=%d, x=%d, y=%d, maxWidth=%d)", chars, s_chars, len, x, y, maxWidth);
}
break;
case sysTrapWinEraseChars: {
  // void WinEraseChars(in Char *chars, Int16 len, Coord x, Coord y)
  chars = ARG32;
  s_chars = emupalmos_trap_in(chars, trap, 0);
  len = ARG16;
  x = ARG16;
  y = ARG16;
  if (s_chars) WinEraseChars(s_chars, len, (uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinEraseChars(chars=0x%08X [%s], len=%d, x=%d, y=%d)", chars, s_chars, len, x, y);
}
break;
case sysTrapWinInvertChars: {
  // void WinInvertChars(in Char *chars, Int16 len, Coord x, Coord y)
  chars = ARG32;
  s_chars = emupalmos_trap_in(chars, trap, 0);
  len = ARG16;
  x = ARG16;
  y = ARG16;
  if (s_chars) WinInvertChars(s_chars, len, (uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinInvertChars(chars=0x%08X [%s], len=%d, x=%d, y=%d)", chars, s_chars, len, x, y);
}
break;
case sysTrapWinSetUnderlineMode: {
  // UnderlineModeType WinSetUnderlineMode(UnderlineModeType mode)
  mode = ARG8;
  resumt = WinSetUnderlineMode((uint8_t)mode);
  m68k_set_reg(M68K_REG_D0, resumt);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetUnderlineMode(mode=%d): %d", mode, resumt);
}
break;
case sysTrapWinPushDrawState: {
  // void WinPushDrawState(void)
  WinPushDrawState();
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPushDrawState()");
}
break;
case sysTrapWinPopDrawState: {
  // void WinPopDrawState(void)
  WinPopDrawState();
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPopDrawState()");
}
break;
case sysTrapWinSetDrawMode: {
  // WinDrawOperation WinSetDrawMode(WinDrawOperation newMode)
  newMode = ARG8;
  resdo = WinSetDrawMode(newMode);
  m68k_set_reg(M68K_REG_D0, resdo);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetDrawMode(newMode=%d): %d", newMode, resdo);
}
break;
case sysTrapWinSetForeColor: {
  // IndexedColorType WinSetForeColor(IndexedColorType foreColor)
  foreColor = ARG8;
  resic = WinSetForeColor(foreColor);
  m68k_set_reg(M68K_REG_D0, resic);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetForeColor(foreColor=%d): %d", foreColor, resic);
}
break;
case sysTrapWinSetBackColor: {
  // IndexedColorType WinSetBackColor(IndexedColorType backColor)
  backColor = ARG8;
  resic = WinSetBackColor(backColor);
  m68k_set_reg(M68K_REG_D0, resic);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetBackColor(backColor=%d): %d", backColor, resic);
}
break;
case sysTrapWinSetTextColor: {
  // IndexedColorType WinSetTextColor(IndexedColorType textColor)
  textColor = ARG8;
  resic = WinSetTextColor(textColor);
  m68k_set_reg(M68K_REG_D0, resic);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetTextColor(textColor=%d): %d", textColor, resic);
}
break;
case sysTrapWinSetForeColorRGB: {
  // void WinSetForeColorRGB(in RGBColorType *newRgbP, out RGBColorType *prevRgbP)
  newRgbP = ARG32;

  decode_rgb(newRgbP, &l_newRgbP);
  prevRgbP = ARG32;

  WinSetForeColorRGB(newRgbP ? &l_newRgbP : NULL, prevRgbP ? &l_prevRgbP : NULL);
  encode_rgb(prevRgbP, &l_prevRgbP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetForeColorRGB(newRgbP=0x%08X, prevRgbP=0x%08X)", newRgbP, prevRgbP);
}
break;
case sysTrapWinSetBackColorRGB: {
  // void WinSetBackColorRGB(in RGBColorType *newRgbP, out RGBColorType *prevRgbP)
  newRgbP = ARG32;

  decode_rgb(newRgbP, &l_newRgbP);
  prevRgbP = ARG32;

  WinSetBackColorRGB(newRgbP ? &l_newRgbP : NULL, prevRgbP ? &l_prevRgbP : NULL);
  encode_rgb(prevRgbP, &l_prevRgbP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetBackColorRGB(newRgbP=0x%08X, prevRgbP=0x%08X)", newRgbP, prevRgbP);
}
break;
case sysTrapWinSetTextColorRGB: {
  // void WinSetTextColorRGB(in RGBColorType *newRgbP, out RGBColorType *prevRgbP)
  newRgbP = ARG32;

  decode_rgb(newRgbP, &l_newRgbP);
  prevRgbP = ARG32;

  WinSetTextColorRGB(newRgbP ? &l_newRgbP : NULL, prevRgbP ? &l_prevRgbP : NULL);
  encode_rgb(prevRgbP, &l_prevRgbP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetTextColorRGB(newRgbP=0x%08X, prevRgbP=0x%08X)", newRgbP, prevRgbP);
}
break;
case sysTrapWinGetPattern: {
  // void WinGetPattern(out CustomPatternType *patternP)
  patternP = ARG32;
  s_patternP = emupalmos_trap_in(patternP, trap, 0);
  if (s_patternP) WinGetPattern(s_patternP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetPattern(patternP=0x%08X)", patternP);
}
break;
case sysTrapWinGetPatternType: {
  // PatternType WinGetPatternType(void)
  respt = WinGetPatternType();
  m68k_set_reg(M68K_REG_D0, respt);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetPatternType(): %d", respt);
}
break;
case sysTrapWinSetPattern: {
  // void WinSetPattern(in CustomPatternType *patternP)
  patternP = ARG32;
  s_patternP = emupalmos_trap_in(patternP, trap, 0);
  if (s_patternP) WinSetPattern(s_patternP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetPattern(patternP=0x%08X)", patternP);
}
break;
case sysTrapWinSetPatternType: {
  // void WinSetPatternType(PatternType newPattern)
  newPattern = ARG8;
  WinSetPatternType(newPattern);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetPatternType(newPattern=%d)", newPattern);
}
break;
case sysTrapWinRGBToIndex: {
  // IndexedColorType WinRGBToIndex(in RGBColorType *rgbP)
  rgbP = ARG32;
  decode_rgb(rgbP, &l_rgbP);
  resic = WinRGBToIndex(rgbP ? &l_rgbP : NULL);
  m68k_set_reg(M68K_REG_D0, resic);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinRGBToIndex(rgbP=0x%08X): %d", rgbP, resic);
}
break;
case sysTrapWinIndexToRGB: {
  // void WinIndexToRGB(IndexedColorType i, out RGBColorType *rgbP)
  i = ARG8;
  rgbP = ARG32;

  WinIndexToRGB((uint8_t)i, rgbP ? &l_rgbP : NULL);
  encode_rgb(rgbP, &l_rgbP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinIndexToRGB(i=%d, rgbP=0x%08X)", i, rgbP);
}
break;
case sysTrapWinSetColors: {
  // void WinSetColors(in RGBColorType *newForeColorP, out RGBColorType *oldForeColorP, in RGBColorType *newBackColorP, out RGBColorType *oldBackColorP)
  newForeColorP = ARG32;
  decode_rgb(newForeColorP, &l_newForeColorP);
  oldForeColorP = ARG32;
  newBackColorP = ARG32;
  decode_rgb(newBackColorP, &l_newBackColorP);
  oldBackColorP = ARG32;
  WinSetColors(newForeColorP ? &l_newForeColorP : NULL, oldForeColorP ? &l_oldForeColorP : NULL, newBackColorP ? &l_newBackColorP : NULL, oldBackColorP ? &l_oldBackColorP : NULL);
  encode_rgb(oldForeColorP, &l_oldForeColorP);
  encode_rgb(oldBackColorP, &l_oldBackColorP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetColors(newForeColorP=0x%08X, oldForeColorP=0x%08X, newBackColorP=0x%08X, oldBackColorP=0x%08X)", newForeColorP, oldForeColorP, newBackColorP, oldBackColorP);
}
break;
case sysTrapWinScreenInit: {
  // void WinScreenInit(void)
  WinScreenInit();
  debug(DEBUG_TRACE, "EmuPalmOS", "WinScreenInit()");
}
break;
case sysTrapWinScreenLock: {
  // UInt8 *WinScreenLock(WinLockInitType initMode)
  initMode = ARG8;
  res8 = WinScreenLock(initMode);
  r_res = emupalmos_trap_out(res8);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinScreenLock(initMode=%d): 0x%08X", initMode, r_res);
}
break;
case sysTrapWinScreenUnlock: {
  // void WinScreenUnlock(void)
  WinScreenUnlock();
  debug(DEBUG_TRACE, "EmuPalmOS", "WinScreenUnlock()");
}
break;
case sysTrapBmpCompress: {
  // Err BmpCompress(in bitmapP, BitmapCompressionType compType)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  compType = ARG8;
  res = BmpCompress(bitmapP ? l_bitmapP : NULL, compType);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpCompress(bitmapP=0x%08X, compType=%d): %d", bitmapP, compType, res);
}
break;
case sysTrapBmpGetBits: {
  // void *BmpGetBits(in bitmapP)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  resv = BmpGetBits(bitmapP ? l_bitmapP : NULL);
  r_res = emupalmos_trap_out(resv);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetBits(bitmapP=0x%08X): 0x%08X", bitmapP, r_res);
}
break;
case sysTrapBmpGetColortable: {
  // ColorTableType *BmpGetColortable(in bitmapP)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  resctt = BmpGetColortable(bitmapP ? l_bitmapP : NULL);
  r_res = emupalmos_trap_out(resctt);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetColortable(bitmapP=0x%08X): 0x%08X", bitmapP, r_res);
}
break;
case sysTrapBmpSize: {
  // UInt16 BmpSize(in bitmapP)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  res = (UInt32)BmpSize(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpSize(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpBitsSize: {
  // UInt16 BmpBitsSize(in bitmapP)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  res = (UInt32)BmpBitsSize(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpBitsSize(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpGetSizes: {
  // void BmpGetSizes(in bitmapP, out UInt32 *dataSizeP, out UInt32 *headerSizeP)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  dataSizeP = ARG32;
  headerSizeP = ARG32;
  BmpGetSizes(bitmapP ? l_bitmapP : NULL, dataSizeP ? &l_dataSizeP : NULL, headerSizeP ? &l_headerSizeP : NULL);
  if (dataSizeP) m68k_write_memory_32(dataSizeP, l_dataSizeP);
  if (headerSizeP) m68k_write_memory_32(headerSizeP, l_headerSizeP);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetSizes(bitmapP=0x%08X, dataSizeP=0x%08X [%d], headerSizeP=0x%08X [%d])", bitmapP, dataSizeP, l_dataSizeP, headerSizeP, l_headerSizeP);
}
break;
case sysTrapBmpColortableSize: {
  // UInt16 BmpColortableSize(in bitmapP)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  res = (UInt32)BmpColortableSize(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpColortableSize(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpGetDimensions: {
  // void BmpGetDimensions(in bitmapP, out Coord *widthP, out Coord *heightP, out UInt16 *rowBytesP)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  widthP = ARG32;
  l_widthP = 0;
  heightP = ARG32;
  l_heightP = 0;
  rowBytesP = ARG32;
  l_rowBytesP = 0;
  BmpGetDimensions(bitmapP ? l_bitmapP : NULL, widthP ? &l_widthP : NULL, heightP ? &l_heightP : NULL, rowBytesP ? &l_rowBytesP : NULL);
  if (widthP) m68k_write_memory_16(widthP, l_widthP);
  if (heightP) m68k_write_memory_16(heightP, l_heightP);
  if (rowBytesP) m68k_write_memory_16(rowBytesP, l_rowBytesP);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetDimensions(bitmapP=0x%08X, widthP=0x%08X [%d], heightP=0x%08X [%d], rowBytesP=0x%08X [%d])", bitmapP, widthP, l_widthP, heightP, l_heightP, rowBytesP, l_rowBytesP);
}
break;
case sysTrapBmpGetBitDepth: {
  // UInt8 BmpGetBitDepth(in bitmapP)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  res = BmpGetBitDepth(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, (UInt8)res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetBitDepth(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpGetNextBitmap: {
  // BitmapType *BmpGetNextBitmap(in bitmapP)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  resbm = BmpGetNextBitmap(bitmapP ? l_bitmapP : NULL);
  r_res = emupalmos_trap_out(resbm);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetNextBitmap(bitmapP=0x%08X): 0x%08X", bitmapP, r_res);
}
break;
case sysTrapFntGetFont: {
  // FontID FntGetFont(void)
  resfont = FntGetFont();
  m68k_set_reg(M68K_REG_D0, resfont);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntGetFont(): %d", resfont);
}
break;
case sysTrapFntSetFont: {
  // FontID FntSetFont(FontID font)
  font = ARG8;
  resfont = FntSetFont(font);
  m68k_set_reg(M68K_REG_D0, resfont);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntSetFont(font=%d): %d", font, resfont);
}
break;
case sysTrapFntGetFontPtr: {
  // FontType *FntGetFontPtr(void)
  fontp = FntGetFontPtr();
  r_res = emupalmos_trap_out(fontp);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntGetFontPtr(): 0x%08X", r_res);
}
break;
case sysTrapFntBaseLine: {
  // Int16 FntBaseLine(void)
  res = FntBaseLine();
  m68k_set_reg(M68K_REG_D0, (Int16)res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntBaseLine(): %d", res);
}
break;
case sysTrapFntCharHeight: {
  // Int16 FntCharHeight(void)
  res = FntCharHeight();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntCharHeight(): %d", res);
}
break;
case sysTrapFntLineHeight: {
  // Int16 FntLineHeight(void)
  res = FntLineHeight();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntLineHeight(): %d", res);
}
break;
case sysTrapFntAverageCharWidth: {
  // Int16 FntAverageCharWidth(void)
  res = FntAverageCharWidth();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntAverageCharWidth(): %d", res);
}
break;
case sysTrapFntCharWidth: {
  // Int16 FntCharWidth(Char ch)
  ch = ARG8;
  res = FntCharWidth(ch);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntCharWidth(ch=%d): %d", ch, res);
}
break;
case sysTrapFntWCharWidth: {
  // Int16 FntWCharWidth(WChar iChar)
  iChar = ARG16;
  res = FntWCharWidth(iChar);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntWCharWidth(iChar=%d): %d", iChar, res);
}
break;
case sysTrapFntCharsWidth: {
  // Int16 FntCharsWidth(in Char *chars, Int16 len)
  chars = ARG32;
  s_chars = emupalmos_trap_in(chars, trap, 0);
  len = ARG16;
  res = s_chars ? FntCharsWidth(s_chars, len) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntCharsWidth(chars=0x%08X [%.*s], len=%d): %d", chars, len, s_chars, len, res);
}
break;
case sysTrapFntWidthToOffset: {
  // Int16 FntWidthToOffset(in Char *pChars, UInt16 length, Int16 pixelWidth, out Boolean *leadingEdge, out Int16 *truncWidth)
  pChars = ARG32;
  s_pChars = emupalmos_trap_in(pChars, trap, 0);
  length = ARG16;
  pixelWidth = ARG16;
  leadingEdge = ARG32;
  truncWidth = ARG32;
  res = s_pChars ? FntWidthToOffset(s_pChars, length, pixelWidth, leadingEdge ? &l_leadingEdge : NULL, truncWidth ? &l_truncWidth : NULL) : 0;
  if (leadingEdge) m68k_write_memory_8(leadingEdge, l_leadingEdge);
  if (truncWidth) m68k_write_memory_16(truncWidth, l_truncWidth);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntWidthToOffset(pChars=0x%08X [%s], length=%d, pixelWidth=%d, leadingEdge=0x%08X, truncWidth=0x%08X [%d]): %d", pChars, s_pChars, length, pixelWidth, leadingEdge, truncWidth, l_truncWidth, res);
}
break;
case sysTrapFntCharsInWidth: {
  // void FntCharsInWidth(in string, inout Int16 *stringWidthP, inout Int16 *stringLengthP, out Boolean *fitWithinWidth)
  string32 = ARG32;
  s_string = emupalmos_trap_in(string32, trap, 0);
  stringWidthP = ARG32;
  if (stringWidthP) l_stringWidthP = m68k_read_memory_16(stringWidthP);
  stringLengthP = ARG32;
  if (stringLengthP) l_stringLengthP = m68k_read_memory_16(stringLengthP);
  fitWithinWidth = ARG32;
  if (s_string) FntCharsInWidth(s_string, stringWidthP ? &l_stringWidthP : NULL, stringLengthP ? &l_stringLengthP : NULL, fitWithinWidth ? &l_fitWithinWidth : NULL);
  if (stringWidthP) m68k_write_memory_16(stringWidthP, l_stringWidthP);
  if (stringLengthP) m68k_write_memory_16(stringLengthP, l_stringLengthP);
  if (fitWithinWidth) m68k_write_memory_8(fitWithinWidth, l_fitWithinWidth);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntCharsInWidth(string=0x%08X [%s], stringWidthP=0x%08X [%d], stringLengthP=0x%08X [%d], fitWithinWidth=0x%08X [%d])", string32, s_string, stringWidthP, l_stringWidthP, stringLengthP, l_stringLengthP, fitWithinWidth, l_fitWithinWidth);
}
break;
case sysTrapFntDescenderHeight: {
  // Int16 FntDescenderHeight(void)
  res = FntDescenderHeight();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntDescenderHeight(): %d", res);
}
break;
case sysTrapFntLineWidth: {
  // Int16 FntLineWidth(in Char *pChars, UInt16 length)
  pChars = ARG32;
  s_pChars = emupalmos_trap_in(pChars, trap, 0);
  length = ARG16;
  res = s_pChars ? FntLineWidth(s_pChars, length) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntLineWidth(pChars=0x%08X [%s], length=%d): %d", pChars, s_pChars, length, res);
}
break;
case sysTrapFntWordWrap: {
  // UInt16 FntWordWrap(in Char *chars, UInt16 maxWidth)
  chars = ARG32;
  s_chars = emupalmos_trap_in(chars, trap, 0);
  maxWidth = ARG16;
  res = s_chars ? FntWordWrap(s_chars, maxWidth) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntWordWrap(chars=0x%08X [%s], maxWidth=%d): %d", chars, s_chars, maxWidth, res);
}
break;
case sysTrapFntWordWrapReverseNLines: {
  // void FntWordWrapReverseNLines(in Char *chars, UInt16 maxWidth, inout UInt16 *linesToScrollP, inout UInt16 *scrollPosP)
  chars = ARG32;
  s_chars = emupalmos_trap_in(chars, trap, 0);
  maxWidth = ARG16;
  linesToScrollP = ARG32;
  if (linesToScrollP) l_linesToScrollP = m68k_read_memory_16(linesToScrollP);
  scrollPosP = ARG32;
  if (scrollPosP) l_scrollPosP = m68k_read_memory_16(scrollPosP);
  if (s_chars) FntWordWrapReverseNLines(s_chars, maxWidth, linesToScrollP ? &l_linesToScrollP : NULL, scrollPosP ? &l_scrollPosP : NULL);
  if (linesToScrollP) m68k_write_memory_16(linesToScrollP, l_linesToScrollP);
  if (scrollPosP) m68k_write_memory_16(scrollPosP, l_scrollPosP);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntWordWrapReverseNLines(chars=0x%08X [%s], maxWidth=%d, linesToScrollP=0x%08X [%d], scrollPosP=0x%08X [%d])", chars, s_chars, maxWidth, linesToScrollP, l_linesToScrollP, scrollPosP, l_scrollPosP);
}
break;
case sysTrapFntGetScrollValues: {
  // void FntGetScrollValues(in Char *chars, UInt16 width, UInt16 scrollPos, out UInt16 *linesP, out UInt16 *topLine)
  chars = ARG32;
  s_chars = emupalmos_trap_in(chars, trap, 0);
  width = ARG16;
  scrollPos = ARG16;
  linesP = ARG32;
  topLine = ARG32;
  if (s_chars) FntGetScrollValues(s_chars, width, scrollPos, linesP ? &l_linesP : NULL, topLine ? &l_topLine : NULL);
  if (linesP) m68k_write_memory_16(linesP, l_linesP);
  if (topLine) m68k_write_memory_16(topLine, l_topLine);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntGetScrollValues(chars=0x%08X [%s], width=%d, scrollPos=%d, linesP=0x%08X [%d], topLine=0x%08X [%d])", chars, s_chars, width, scrollPos, linesP, l_linesP, topLine, l_topLine);
}
break;
case sysTrapStrCopy: {
  // strCopy(out Char *dst, in src)
  dst32 = ARG32;
  s_dst = emupalmos_trap_in(dst32, trap, 0);
  src32 = ARG32;
  s_src = emupalmos_trap_in(src32, trap, 1);
  resc = NULL;
  if (s_dst && s_src) {
    if (emupalmos_check_address(dst32, (uint32_t)(sys_strlen(s_src)+1), 0)) {
      debug(DEBUG_TRACE, "EmuPalmOS", "StrCopy %d bytes", (int)sys_strlen(s_src)+1);
      resc = StrCopy(s_dst, s_src);
    }
  }
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrCopy(dst=0x%08X [%s], src=0x%08X [%s]): 0x%08X", dst32, s_dst, src32, s_src, r_res);
}
break;
case sysTrapStrNCopy: {
  // strNCopy(out Char *dst, in src, Int16 n)
  dst32 = ARG32;
  s_dst = emupalmos_trap_in(dst32, trap, 0);
  src32 = ARG32;
  s_src = emupalmos_trap_in(src32, trap, 1);
  n = ARG16;
  resc = s_dst && s_src ? StrNCopy(s_dst, s_src, (int16_t)n) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrNCopy(dst=0x%08X [%s], src=0x%08X [%s], n=%d): 0x%08X", dst32, s_dst, src32, s_src, n, r_res);
}
break;
case sysTrapStrCat: {
  // strCat(out Char *dst, in src)
  dst32 = ARG32;
  s_dst = emupalmos_trap_in(dst32, trap, 0);
  src32 = ARG32;
  s_src = emupalmos_trap_in(src32, trap, 1);
  resc = NULL;
  if (s_dst && s_src) {
    if (emupalmos_check_address((uint32_t)(dst32 + sys_strlen(s_dst)), (uint32_t)(sys_strlen(s_src)+1), 0)) {
      debug(DEBUG_TRACE, "EmuPalmOS", "StrCat %d bytes into %d bytes with %d total", (int)sys_strlen(s_src)+1, (int)sys_strlen(s_dst), (int)sys_strlen(s_dst) + (int)sys_strlen(s_src)+1);
      resc = StrCat(s_dst, s_src);
    }
  }
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrCat(dst=0x%08X [%s], src=0x%08X [%s]): 0x%08X", dst32, s_dst, src32, s_src, r_res);
}
break;
case sysTrapStrNCat: {
  // strNCat(out Char *dst, in src, Int16 n)
  dst32 = ARG32;
  s_dst = emupalmos_trap_in(dst32, trap, 0);
  src32 = ARG32;
  s_src = emupalmos_trap_in(src32, trap, 1);
  n = ARG16;
  resc = s_dst && s_src ? StrNCat(s_dst, s_src, (int16_t)n) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrNCat(dst=0x%08X [%s], src=0x%08X [%s], n=%d): 0x%08X", dst32, s_dst, src32, s_src, n, r_res);
}
break;
case sysTrapStrLen: {
  // UInt16 StrLen(in src)
  src32 = ARG32;
  s_src = emupalmos_trap_in(src32, trap, 0);
  res = s_src ? StrLen(s_src) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrLen(src=0x%08X [%s]): %d", src32, s_src, res);
}
break;
case sysTrapStrCompareAscii: {
  // Int16 StrCompareAscii(in s1, in s2)
  s1_32 = ARG32;
  s_s1 = emupalmos_trap_in(s1_32, trap, 0);
  s2_32 = ARG32;
  s_s2 = emupalmos_trap_in(s2_32, trap, 1);
  res = s_s1 && s_s2 ? StrCompareAscii(s_s1, s_s2) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrCompareAscii(s1=0x%08X [%s], s2=0x%08X [%s]): %d", s1_32, s_s1, s2_32, s_s2, res);
}
break;
case sysTrapStrCompare: {
  // Int16 StrCompare(in s1, in s2)
  s1_32 = ARG32;
  s_s1 = emupalmos_trap_in(s1_32, trap, 0);
  s2_32 = ARG32;
  s_s2 = emupalmos_trap_in(s2_32, trap, 1);
  res = s_s1 && s_s2 ? StrCompare(s_s1, s_s2) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrCompare(s1=0x%08X [%s], s2=0x%08X [%s]): %d", s1_32, s_s1, s2_32, s_s2, res);
}
break;
case sysTrapStrNCompareAscii: {
  // Int16 StrNCompareAscii(in s1, in s2, Int32 n)
  s1_32 = ARG32;
  s_s1 = emupalmos_trap_in(s1_32, trap, 0);
  s2_32 = ARG32;
  s_s2 = emupalmos_trap_in(s2_32, trap, 1);
  n = ARG32;
  res = s_s1 && s_s2 ? StrNCompareAscii(s_s1, s_s2, n) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrNCompareAscii(s1=0x%08X [%s], s2=0x%08X [%s], n=%d): %d", s1_32, s_s1, s2_32, s_s2, n, res);
}
break;
case sysTrapStrNCompare: {
  // Int16 StrNCompare(in s1, in s2, Int32 n)
  s1_32 = ARG32;
  s_s1 = emupalmos_trap_in(s1_32, trap, 0);
  s2_32 = ARG32;
  s_s2 = emupalmos_trap_in(s2_32, trap, 1);
  n = ARG32;
  res = s_s1 && s_s2 ? StrNCompare(s_s1, s_s2, n) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrNCompare(s1=0x%08X [%s], s2=0x%08X [%s], n=%d): %d", s1_32, s_s1, s2_32, s_s2, n, res);
}
break;
case sysTrapStrCaselessCompare: {
  // Int16 StrCaselessCompare(in s1, in s2)
  s1_32 = ARG32;
  s_s1 = emupalmos_trap_in(s1_32, trap, 0);
  s2_32 = ARG32;
  s_s2 = emupalmos_trap_in(s2_32, trap, 1);
  res = s_s1 && s_s2 ? StrCaselessCompare(s_s1, s_s2) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrCaselessCompare(s1=0x%08X [%s], s2=0x%08X [%s]): %d", s1_32, s_s1, s2_32, s_s2, res);
}
break;
case sysTrapStrNCaselessCompare: {
  // Int16 StrNCaselessCompare(in s1, in s2, Int32 n)
  s1_32 = ARG32;
  s_s1 = emupalmos_trap_in(s1_32, trap, 0);
  s2_32 = ARG32;
  s_s2 = emupalmos_trap_in(s2_32, trap, 1);
  n = ARG32;
  res = s_s1 && s_s2 ?StrNCaselessCompare(s_s1, s_s2, n) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrNCaselessCompare(s1=0x%08X [%s], s2=0x%08X [%s], n=%d): %d", s1_32, s_s1, s2_32, s_s2, n, res);
}
break;
case sysTrapStrToLower: {
  // strToLower(out Char *dst, in src)
  dst32 = ARG32;
  s_dst = emupalmos_trap_in(dst32, trap, 0);
  src32 = ARG32;
  s_src = emupalmos_trap_in(src32, trap, 1);
  resc = s_dst && s_src ? StrToLower(s_dst, s_src) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrToLower(dst=0x%08X [%s], src=0x%08X [%s]): 0x%08X", dst32, s_dst, src32, s_src, r_res);
}
break;
case sysTrapStrIToA: {
  // strIToA(out s, Int32 i)
  s32 = ARG32;
  s_s = emupalmos_trap_in(s32, trap, 0);
  i = ARG32;
  resc = s_s ? StrIToA(s_s, i) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrIToA(s=0x%08X [%s], i=%d): 0x%08X", s32, s_s, i, r_res);
}
break;
case sysTrapStrIToH: {
  // strIToH(out s, UInt32 i)
  s32 = ARG32;
  s_s = emupalmos_trap_in(s32, trap, 0);
  i = ARG32;
  resc = s_s ? StrIToH(s_s, i) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrIToH(s=0x%08X [%s], i=%d): 0x%08X", s32, s_s, i, r_res);
}
break;
case sysTrapStrLocalizeNumber: {
  // strLocalizeNumber(out s, Char thousandSeparator, Char decimalSeparator)
  s32 = ARG32;
  s_s = emupalmos_trap_in(s32, trap, 0);
  thousandSeparator = ARG8;
  decimalSeparator = ARG8;
  resc = s_s ? StrLocalizeNumber(s_s, thousandSeparator, decimalSeparator) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrLocalizeNumber(s=0x%08X [%s], thousandSeparator=%d, decimalSeparator=%d): 0x%08X", s32, s_s, thousandSeparator, decimalSeparator, r_res);
}
break;
case sysTrapStrDelocalizeNumber: {
  // strDelocalizeNumber(out s, Char thousandSeparator, Char decimalSeparator)
  s32 = ARG32;
  s_s = emupalmos_trap_in(s32, trap, 0);
  thousandSeparator = ARG8;
  decimalSeparator = ARG8;
  resc = s_s ? StrDelocalizeNumber(s_s, thousandSeparator, decimalSeparator) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrDelocalizeNumber(s=0x%08X [%s], thousandSeparator=%d, decimalSeparator=%d): 0x%08X", s32, s_s, thousandSeparator, decimalSeparator, r_res);
}
break;
case sysTrapStrChr: {
  // strChr(in str, WChar chr)
  str = ARG32;
  s_str = emupalmos_trap_in(str, trap, 0);
  chr = ARG16;
  resc = s_str ? StrChr(s_str, chr) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrChr(str=0x%08X [%s], chr=%d): 0x%08X", str, s_str, chr, r_res);
}
break;
case sysTrapStrStr: {
  // strStr(in str, in Char *token)
  str = ARG32;
  s_str = emupalmos_trap_in(str, trap, 0);
  token = ARG32;
  s_token = emupalmos_trap_in(token, trap, 1);
  resc = s_str && s_token ? StrStr(s_str, s_token) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrStr(str=0x%08X [%s], token=0x%08X [%s]): 0x%08X", str, s_str, token, s_token, r_res);
}
break;
case sysTrapStrAToI: {
  // Int32 StrAToI(in str)
  str = ARG32;
  s_str = emupalmos_trap_in(str, trap, 0);
  res = s_str ? StrAToI(s_str) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrAToI(str=0x%08X [%s]): %d", str, s_str, res);
}
break;
case sysTrapFldCopy: {
  // void FldCopy(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldCopy(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldCopy(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldCut: {
  // void FldCut(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldCut(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldCut(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldDrawField: {
  // void FldDrawField(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldDrawField(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldDrawField(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldEraseField: {
  // void FldEraseField(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldEraseField(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldEraseField(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldFreeMemory: {
  // void FldFreeMemory(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldFreeMemory(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldFreeMemory(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldGetBounds: {
  // void FldGetBounds(in fldP, out RectangleType *rect)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  rect32 = ARG32;
  FldGetBounds(fldP ? s_fldP : NULL, rect32 ? &l_rect : NULL);
  encode_rectangle(rect32, &l_rect);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetBounds(fldP=0x%08X, rect=0x%08X [%d,%d,%d,%d])", fldP, rect32, l_rect.topLeft.x, l_rect.topLeft.y, l_rect.extent.x, l_rect.extent.y);
}
break;
case sysTrapFldGetFont: {
  // FontID FldGetFont(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  resfont = FldGetFont(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, resfont);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetFont(fldP=0x%08X): %d", fldP, resfont);
}
break;
case sysTrapFldGetSelection: {
  // void FldGetSelection(in fldP, out UInt16 *startPosition, out UInt16 *endPosition)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  startPosition = ARG32;
  endPosition = ARG32;
  FldGetSelection(fldP ? s_fldP : NULL, startPosition ? &l_startPosition : NULL, endPosition ? &l_endPosition : NULL);
  if (startPosition) m68k_write_memory_16(startPosition, l_startPosition);
  if (endPosition) m68k_write_memory_16(endPosition, l_endPosition);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetSelection(fldP=0x%08X, startPosition=0x%08X [%d], endPosition=0x%08X [%d])", fldP, startPosition, l_startPosition, endPosition, l_endPosition);
}
break;
case sysTrapFldGetTextHandle: {
  // MemHandle FldGetTextHandle(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  resmem = FldGetTextHandle(fldP ? s_fldP : NULL);
  r_res = emupalmos_trap_out(resmem);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetTextHandle(fldP=0x%08X): 0x%08X", fldP, r_res);
}
break;
case sysTrapFldGetTextPtr: {
  // Char *FldGetTextPtr(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  resc = FldGetTextPtr(fldP ? s_fldP : NULL);
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetTextPtr(fldP=0x%08X): 0x%08X", fldP, r_res);
}
break;
case sysTrapFldHandleEvent: {
  // Boolean FldHandleEvent(in fldP, in EventType *eventP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  eventP = ARG32;
  decode_event(eventP, &l_eventP);
  res = (UInt32)FldHandleEvent(fldP ? s_fldP : NULL, eventP ? &l_eventP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldHandleEvent(fldP=0x%08X, eventP=0x%08X): %d", fldP, eventP, res);
}
break;
case sysTrapFldPaste: {
  // void FldPaste(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldPaste(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldPaste(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldRecalculateField: {
  // void FldRecalculateField(in fldP, Boolean redraw)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  redraw = ARG8;
  FldRecalculateField(fldP ? s_fldP : NULL, redraw);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldRecalculateField(fldP=0x%08X, redraw=%d)", fldP, redraw);
}
break;
case sysTrapFldSetBounds: {
  // void FldSetBounds(in fldP, in RectangleType *rP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  FldSetBounds(fldP ? s_fldP : NULL, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetBounds(fldP=0x%08X, rP=0x%08X [%d,%d,%d,%d])", fldP, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapFldSetFont: {
  // void FldSetFont(in fldP, FontID fontID)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  fontID = ARG8;
  FldSetFont(fldP ? s_fldP : NULL, fontID);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetFont(fldP=0x%08X, fontID=%d)", fldP, fontID);
}
break;
case sysTrapFldSetText: {
  // void FldSetText(in fldP, MemHandle textHandle, UInt16 offset, UInt16 size)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  textHandle = ARG32;
  l_textHandle = textHandle ? ram + textHandle : NULL;
  offset = ARG16;
  size = ARG16;
  FldSetText(fldP ? s_fldP : NULL, textHandle ? l_textHandle : 0, (uint16_t)offset, (uint16_t)size);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetText(fldP=0x%08X, textHandle=0x%08X, offset=%d, size=%d)", fldP, textHandle, offset, size);
}
break;
case sysTrapFldSetTextHandle: {
  // void FldSetTextHandle(in fldP, MemHandle textHandle)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  textHandle = ARG32;
  l_textHandle = textHandle ? ram + textHandle : NULL;
  FldSetTextHandle(fldP ? s_fldP : NULL, textHandle ? l_textHandle : 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetTextHandle(fldP=0x%08X, textHandle=0x%08X)", fldP, textHandle);
}
break;
case sysTrapFldSetTextPtr: {
  // void FldSetTextPtr(in fldP, in textP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  textP = ARG32;
  s_textP = textP ? (char *)(ram + textP) : NULL;
  FldSetTextPtr(fldP ? s_fldP : NULL, textP ? s_textP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetTextPtr(fldP=0x%08X, textP=0x%08X [%s])", fldP, textP, s_textP);
}
break;
case sysTrapFldSetUsable: {
  // void FldSetUsable(in fldP, Boolean usable)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  usable = ARG8;
  FldSetUsable(fldP ? s_fldP : NULL, usable);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetUsable(fldP=0x%08X, usable=%d)", fldP, usable);
}
break;
case sysTrapFldSetSelection: {
  // void FldSetSelection(in fldP, UInt16 startPosition, UInt16 endPosition)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  startPosition = ARG16;
  endPosition = ARG16;
  FldSetSelection(fldP ? s_fldP : NULL, (uint16_t)startPosition, (uint16_t)endPosition);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetSelection(fldP=0x%08X, startPosition=%d, endPosition=%d)", fldP, startPosition, endPosition);
}
break;
case sysTrapFldGrabFocus: {
  // void FldGrabFocus(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldGrabFocus(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGrabFocus(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldReleaseFocus: {
  // void FldReleaseFocus(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldReleaseFocus(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldReleaseFocus(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldGetInsPtPosition: {
  // UInt16 FldGetInsPtPosition(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  res = (UInt32)FldGetInsPtPosition(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetInsPtPosition(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldSetInsPtPosition: {
  // void FldSetInsPtPosition(in fldP, UInt16 pos)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  pos = ARG16;
  FldSetInsPtPosition(fldP ? s_fldP : NULL, pos);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetInsPtPosition(fldP=0x%08X, pos=%d)", fldP, pos);
}
break;
case sysTrapFldSetInsertionPoint: {
  // void FldSetInsertionPoint(in fldP, UInt16 pos)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  pos = ARG16;
  FldSetInsertionPoint(fldP ? s_fldP : NULL, pos);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetInsertionPoint(fldP=0x%08X, pos=%d)", fldP, pos);
}
break;
case sysTrapFldGetScrollPosition: {
  // UInt16 FldGetScrollPosition(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  res = (UInt32)FldGetScrollPosition(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetScrollPosition(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldSetScrollPosition: {
  // void FldSetScrollPosition(in fldP, UInt16 pos)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  pos = ARG16;
  FldSetScrollPosition(fldP ? s_fldP : NULL, pos);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetScrollPosition(fldP=0x%08X, pos=%d)", fldP, pos);
}
break;
case sysTrapFldGetScrollValues: {
  // void FldGetScrollValues(in fldP, out UInt16 *scrollPosP, out UInt16 *textHeightP, out UInt16 *fieldHeightP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  scrollPosP = ARG32;
  textHeightP = ARG32;
  fieldHeightP = ARG32;
  FldGetScrollValues(fldP ? s_fldP : NULL, scrollPosP ? &l_scrollPosP : NULL, textHeightP ? &l_textHeightP : NULL, fieldHeightP ? &l_fieldHeightP : NULL);
  if (scrollPosP) m68k_write_memory_16(scrollPosP, l_scrollPosP);
  if (textHeightP) m68k_write_memory_16(textHeightP, l_textHeightP);
  if (fieldHeightP) m68k_write_memory_16(fieldHeightP, l_fieldHeightP);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetScrollValues(fldP=0x%08X, scrollPosP=0x%08X [%d], textHeightP=0x%08X [%d], fieldHeightP=0x%08X [%d])", fldP, scrollPosP, l_scrollPosP, textHeightP, l_textHeightP, fieldHeightP, l_fieldHeightP);
}
break;
case sysTrapFldGetTextLength: {
  // UInt16 FldGetTextLength(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  res = (UInt32)FldGetTextLength(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetTextLength(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldScrollField: {
  // void FldScrollField(in fldP, UInt16 linesToScroll, WinDirectionType direction)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  linesToScroll = ARG16;
  direction = ARG8;
  FldScrollField(fldP ? s_fldP : NULL, linesToScroll, (uint8_t)direction);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldScrollField(fldP=0x%08X, linesToScroll=%d, direction=%d)", fldP, linesToScroll, direction);
}
break;
case sysTrapFldScrollable: {
  // Boolean FldScrollable(in fldP, WinDirectionType direction)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  direction = ARG8;
  res = (UInt32)FldScrollable(fldP ? s_fldP : NULL, (uint8_t)direction);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldScrollable(fldP=0x%08X, direction=%d): %d", fldP, direction, res);
}
break;
case sysTrapFldGetVisibleLines: {
  // UInt16 FldGetVisibleLines(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  res = (UInt32)FldGetVisibleLines(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetVisibleLines(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldGetTextHeight: {
  // UInt16 FldGetTextHeight(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  res = (UInt32)FldGetTextHeight(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetTextHeight(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldCalcFieldHeight: {
  // UInt16 FldCalcFieldHeight(in Char *chars, UInt16 maxWidth)
  chars = ARG32;
  s_chars = chars ? (char *)(ram + chars) : NULL;
  maxWidth = ARG16;
  res = (UInt32)FldCalcFieldHeight(chars ? s_chars : NULL, maxWidth);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldCalcFieldHeight(chars=0x%08X [%s], maxWidth=%d): %d", chars, s_chars, maxWidth, res);
}
break;
case sysTrapFldWordWrap: {
  // UInt16 FldWordWrap(in Char *chars, Int16 maxWidth)
  chars = ARG32;
  s_chars = chars ? (char *)(ram + chars) : NULL;
  maxWidth = ARG16;
  res = (UInt32)FldWordWrap(chars ? s_chars : NULL, maxWidth);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldWordWrap(chars=0x%08X [%s], maxWidth=%d): %d", chars, s_chars, maxWidth, res);
}
break;
case sysTrapFldCompactText: {
  // void FldCompactText(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldCompactText(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldCompactText(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldDirty: {
  // Boolean FldDirty(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  res = (UInt32)FldDirty(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldDirty(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldSetDirty: {
  // void FldSetDirty(in fldP, Boolean dirty)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  dirty = ARG8;
  FldSetDirty(fldP ? s_fldP : NULL, dirty);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetDirty(fldP=0x%08X, dirty=%d)", fldP, dirty);
}
break;
case sysTrapFldGetMaxChars: {
  // UInt16 FldGetMaxChars(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  res = (UInt32)FldGetMaxChars(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetMaxChars(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldSetMaxChars: {
  // void FldSetMaxChars(in fldP, UInt16 maxChars)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  maxChars = ARG16;
  FldSetMaxChars(fldP ? s_fldP : NULL, (uint16_t)maxChars);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetMaxChars(fldP=0x%08X, maxChars=%d)", fldP, maxChars);
}
break;
case sysTrapFldInsert: {
  // Boolean FldInsert(in fldP, in Char *insertChars, UInt16 insertLen)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  insertChars = ARG32;
  s_insertChars = insertChars ? (char *)(ram + insertChars) : NULL;
  insertLen = ARG16;
  res = (UInt32)FldInsert(fldP ? s_fldP : NULL, insertChars ? s_insertChars : NULL, insertLen);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldInsert(fldP=0x%08X, insertChars=0x%08X [%s], insertLen=%d): %d", fldP, insertChars, s_insertChars, insertLen, res);
}
break;
case sysTrapFldDelete: {
  // void FldDelete(in fldP, UInt16 start, UInt16 end)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  start = ARG16;
  end = ARG16;
  FldDelete(fldP ? s_fldP : NULL, start, end);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldDelete(fldP=0x%08X, start=%d, end=%d)", fldP, start, end);
}
break;
case sysTrapFldUndo: {
  // void FldUndo(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldUndo(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldUndo(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldGetTextAllocatedSize: {
  // UInt16 FldGetTextAllocatedSize(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  res = (UInt32)FldGetTextAllocatedSize(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetTextAllocatedSize(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldSetTextAllocatedSize: {
  // void FldSetTextAllocatedSize(in fldP, UInt16 allocatedSize)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  allocatedSize = ARG16;
  FldSetTextAllocatedSize(fldP ? s_fldP : NULL, allocatedSize);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetTextAllocatedSize(fldP=0x%08X, allocatedSize=%d)", fldP, allocatedSize);
}
break;
case sysTrapFldGetAttributes: {
  // void FldGetAttributes(in fldP, in FieldAttrType *attrP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  attrP = ARG32;
  FldGetAttributes(fldP ? s_fldP : NULL, attrP ? &attrFields : NULL);
  if (attrP) {
    attrBits = 0;
    if (attrFields.usable)       attrBits |= 0x8000;
    if (attrFields.visible)      attrBits |= 0x4000;
    if (attrFields.editable)     attrBits |= 0x2000;
    if (attrFields.singleLine)   attrBits |= 0x1000;
    if (attrFields.hasFocus)     attrBits |= 0x0800;
    if (attrFields.dynamicSize)  attrBits |= 0x0400;
    if (attrFields.insPtVisible) attrBits |= 0x0200;
    if (attrFields.dirty)        attrBits |= 0x0100;
    attrBits |= attrFields.underlined    << 6;
    attrBits |= attrFields.justification << 4;
    if (attrFields.autoShift)    attrBits |= 0x0008;
    if (attrFields.hasScrollBar) attrBits |= 0x0004;
    if (attrFields.numeric)      attrBits |= 0x0002;
    if (attrFields.reserved)     attrBits |= 0x0001;
    m68k_write_memory_16(attrP, attrBits);
  }
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetAttributes(fldP=0x%08X, attrP=0x%08X)", fldP, attrP);
}
break;
case sysTrapFldSetAttributes: {
  // void FldSetAttributes(in fldP, in FieldAttrType *attrP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  attrP = ARG32;
  attrBits = attrP ? m68k_read_memory_16(attrP) : 0;
  if (attrP) {
    attrFields.usable        = (attrBits & 0x8000) ? 1 : 0;
    attrFields.visible       = (attrBits & 0x4000) ? 1 : 0;
    attrFields.editable      = (attrBits & 0x2000) ? 1 : 0;
    attrFields.singleLine    = (attrBits & 0x1000) ? 1 : 0;
    attrFields.hasFocus      = (attrBits & 0x0800) ? 1 : 0;
    attrFields.dynamicSize   = (attrBits & 0x0400) ? 1 : 0;
    attrFields.insPtVisible  = (attrBits & 0x0200) ? 1 : 0;
    attrFields.dirty         = (attrBits & 0x0100) ? 1 : 0;
    attrFields.underlined    = (attrBits & 0x00c0) >> 6;
    attrFields.justification = (attrBits & 0x0030) >> 4;
    attrFields.autoShift     = (attrBits & 0x0008) ? 1 : 0;
    attrFields.hasScrollBar  = (attrBits & 0x0004) ? 1 : 0;
    attrFields.numeric       = (attrBits & 0x0002) ? 1 : 0;
    attrFields.reserved      = (attrBits & 0x0001) ? 1 : 0;
  }
  FldSetAttributes(fldP ? s_fldP : NULL, attrP ? &attrFields : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetAttributes(fldP=0x%08X, attrP=0x%08X)", fldP, attrP);
}
break;
case sysTrapFldSendChangeNotification: {
  // void FldSendChangeNotification(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldSendChangeNotification(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSendChangeNotification(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldSendHeightChangeNotification: {
  // void FldSendHeightChangeNotification(in fldP, UInt16 pos, Int16 numLines)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  pos = ARG16;
  numLines = ARG16;
  FldSendHeightChangeNotification(fldP ? s_fldP : NULL, pos, numLines);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSendHeightChangeNotification(fldP=0x%08X, pos=%d, numLines=%d)", fldP, pos, numLines);
}
break;
case sysTrapFldMakeFullyVisible: {
  // Boolean FldMakeFullyVisible(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  res = (UInt32)FldMakeFullyVisible(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldMakeFullyVisible(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldGetNumberOfBlankLines: {
  // UInt16 FldGetNumberOfBlankLines(in fldP)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  res = (UInt32)FldGetNumberOfBlankLines(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetNumberOfBlankLines(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldSetMaxVisibleLines: {
  // void FldSetMaxVisibleLines(in fldP, UInt8 maxLines)
  fldP = ARG32;
  s_fldP = emupalmos_trap_in(fldP, trap, 0);
  maxLines = ARG8;
  FldSetMaxVisibleLines(fldP ? s_fldP : NULL, maxLines);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetMaxVisibleLines(fldP=0x%08X, maxLines=%d)", fldP, maxLines);
}
break;
case sysTrapFldNewField: {
  // fldNewField(void **formPP, UInt16 id,
  //   Coord x, Coord y, Coord width, Coord height,
  //   FontID font, UInt32 maxChars, Boolean editable, Boolean underlined,
  //   Boolean singleLine, Boolean dynamicSize, JustificationType justification,
  //   Boolean autoShift, Boolean hasScrollBar, Boolean numeric)
  formPP = ARG32;
  id = ARG16;
  x = ARG16;
  y = ARG16;
  width = ARG16;
  height = ARG16;
  font = ARG8;
  maxChars = ARG32;
  editable = ARG8;
  underlined = ARG8;
  singleLine = ARG8;
  dynamicSize = ARG8;
  justification = ARG8;
  autoShift = ARG8;
  hasScrollBar = ARG8;
  numeric = ARG8;
  formP = formPP ? m68k_read_memory_32(formPP) : 0;
  form = emupalmos_trap_in(formP, trap, 0);
  fld = FldNewField((void **)&form, id, (uint16_t)x, (uint16_t)y, width, height,
    font, maxChars, editable, underlined,
    singleLine, dynamicSize, justification,
    autoShift, hasScrollBar, numeric);
  a = emupalmos_trap_out(fld);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldNewField(0x%08X, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d): 0x%08X",
    formPP, id, x, y, width, height, font, maxChars,
    editable, underlined, singleLine, dynamicSize, justification, autoShift, hasScrollBar, numeric, a);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapTblDrawTable: {
  // void TblDrawTable(in TableType *tableP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  TblDrawTable(tableP ? s_tableP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblDrawTable(tableP=0x%08X)", tableP);
}
break;
case sysTrapTblRedrawTable: {
  // void TblRedrawTable(in TableType *tableP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  TblRedrawTable(tableP ? s_tableP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblRedrawTable(tableP=0x%08X)", tableP);
}
break;
case sysTrapTblEraseTable: {
  // void TblEraseTable(in TableType *tableP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  TblEraseTable(tableP ? s_tableP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblEraseTable(tableP=0x%08X)", tableP);
}
break;
case sysTrapTblHandleEvent: {
  // Boolean TblHandleEvent(in TableType *tableP, in EventType *event)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  event = ARG32;
  decode_event(event, &l_event);
  res = (UInt32)TblHandleEvent(tableP ? s_tableP : NULL, event ? &l_event : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblHandleEvent(tableP=0x%08X, event=0x%08X): %d", tableP, event, res);
}
break;
case sysTrapTblGetItemBounds: {
  // void TblGetItemBounds(in TableType *tableP, Int16 row, Int16 column, out RectangleType *rP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  column = ARG16;
  rP = ARG32;

  TblGetItemBounds(tableP ? s_tableP : NULL, row, column, rP ? &l_rP : NULL);
  encode_rectangle(rP, &l_rP);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetItemBounds(tableP=0x%08X, row=%d, column=%d, rP=0x%08X [%d,%d,%d,%d])", tableP, row, column, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapTblSelectItem: {
  // void TblSelectItem(in TableType *tableP, Int16 row, Int16 column)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  column = ARG16;
  TblSelectItem(tableP ? s_tableP : NULL, row, column);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSelectItem(tableP=0x%08X, row=%d, column=%d)", tableP, row, column);
}
break;
case sysTrapTblGetItemInt: {
  // Int16 TblGetItemInt(in TableType *tableP, Int16 row, Int16 column)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  column = ARG16;
  res = TblGetItemInt(tableP ? s_tableP : NULL, row, column);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetItemInt(tableP=0x%08X, row=%d, column=%d): %d", tableP, row, column, res);
}
break;
case sysTrapTblSetItemInt: {
  // void TblSetItemInt(in TableType *tableP, Int16 row, Int16 column, Int16 value)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  column = ARG16;
  value = ARG16;
  TblSetItemInt(tableP ? s_tableP : NULL, row, column, value);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetItemInt(tableP=0x%08X, row=%d, column=%d, value=%d)", tableP, row, column, value);
}
break;
case sysTrapTblSetItemPtr: {
  // void TblSetItemPtr(in TableType *tableP, Int16 row, Int16 column, in void *value)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  column = ARG16;
  value = ARG32;
  s_value = value ? (void *)(ram + value) : NULL;
  TblSetItemPtr(tableP ? s_tableP : NULL, row, column, value ? s_value : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetItemPtr(tableP=0x%08X, row=%d, column=%d, value=0x%08X)", tableP, row, column, value);
}
break;
case sysTrapTblSetItemStyle: {
  // void TblSetItemStyle(in TableType *tableP, Int16 row, Int16 column, TableItemStyleType type)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  column = ARG16;
  type = ARG8;
  TblSetItemStyle(tableP ? s_tableP : NULL, row, column, (uint8_t)type);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetItemStyle(tableP=0x%08X, row=%d, column=%d, type=%d)", tableP, row, column, type);
}
break;
case sysTrapTblUnhighlightSelection: {
  // void TblUnhighlightSelection(in TableType *tableP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  TblUnhighlightSelection(tableP ? s_tableP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblUnhighlightSelection(tableP=0x%08X)", tableP);
}
break;
case sysTrapTblRowUsable: {
  // Boolean TblRowUsable(in TableType *tableP, Int16 row)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  res = (UInt32)TblRowUsable(tableP ? s_tableP : NULL, row);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblRowUsable(tableP=0x%08X, row=%d): %d", tableP, row, res);
}
break;
case sysTrapTblSetRowUsable: {
  // void TblSetRowUsable(in TableType *tableP, Int16 row, Boolean usable)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  usable = ARG8;
  TblSetRowUsable(tableP ? s_tableP : NULL, row, usable);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowUsable(tableP=0x%08X, row=%d, usable=%d)", tableP, row, usable);
}
break;
case sysTrapTblGetLastUsableRow: {
  // Int16 TblGetLastUsableRow(in TableType *tableP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  res = TblGetLastUsableRow(tableP ? s_tableP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetLastUsableRow(tableP=0x%08X): %d", tableP, res);
}
break;
case sysTrapTblSetColumnUsable: {
  // void TblSetColumnUsable(in TableType *tableP, Int16 column, Boolean usable)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  column = ARG16;
  usable = ARG8;
  TblSetColumnUsable(tableP ? s_tableP : NULL, column, usable);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetColumnUsable(tableP=0x%08X, column=%d, usable=%d)", tableP, column, usable);
}
break;
case sysTrapTblSetRowSelectable: {
  // void TblSetRowSelectable(in TableType *tableP, Int16 row, Boolean selectable)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  selectable = ARG8;
  TblSetRowSelectable(tableP ? s_tableP : NULL, row, selectable);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowSelectable(tableP=0x%08X, row=%d, selectable=%d)", tableP, row, selectable);
}
break;
case sysTrapTblRowSelectable: {
  // Boolean TblRowSelectable(in TableType *tableP, Int16 row)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  res = (UInt32)TblRowSelectable(tableP ? s_tableP : NULL, row);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblRowSelectable(tableP=0x%08X, row=%d): %d", tableP, row, res);
}
break;
case sysTrapTblGetNumberOfRows: {
  // Int16 TblGetNumberOfRows(in TableType *tableP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  res = TblGetNumberOfRows(tableP ? s_tableP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetNumberOfRows(tableP=0x%08X): %d", tableP, res);
}
break;
case sysTrapTblGetBounds: {
  // void TblGetBounds(in TableType *tableP, out RectangleType *rP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  rP = ARG32;

  TblGetBounds(tableP ? s_tableP : NULL, rP ? &l_rP : NULL);
  encode_rectangle(rP, &l_rP);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetBounds(tableP=0x%08X, rP=0x%08X [%d,%d,%d,%d])", tableP, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapTblSetBounds: {
  // void TblSetBounds(in TableType *tableP, in RectangleType *rP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  TblSetBounds(tableP ? s_tableP : NULL, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetBounds(tableP=0x%08X, rP=0x%08X [%d,%d,%d,%d])", tableP, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapTblGetRowHeight: {
  // Coord TblGetRowHeight(in TableType *tableP, Int16 row)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  resco = TblGetRowHeight(tableP ? s_tableP : NULL, row);
  m68k_set_reg(M68K_REG_D0, resco);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetRowHeight(tableP=0x%08X, row=%d): %d", tableP, row, resco);
}
break;
case sysTrapTblSetRowHeight: {
  // void TblSetRowHeight(in TableType *tableP, Int16 row, Coord height)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  height = ARG16;
  TblSetRowHeight(tableP ? s_tableP : NULL, row, height);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowHeight(tableP=0x%08X, row=%d, height=%d)", tableP, row, height);
}
break;
case sysTrapTblGetColumnWidth: {
  // Coord TblGetColumnWidth(in TableType *tableP, Int16 column)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  column = ARG16;
  resco = TblGetColumnWidth(tableP ? s_tableP : NULL, column);
  m68k_set_reg(M68K_REG_D0, resco);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetColumnWidth(tableP=0x%08X, column=%d): %d", tableP, column, resco);
}
break;
case sysTrapTblSetColumnWidth: {
  // void TblSetColumnWidth(in TableType *tableP, Int16 column, Coord width)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  column = ARG16;
  width = ARG16;
  TblSetColumnWidth(tableP ? s_tableP : NULL, column, width);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetColumnWidth(tableP=0x%08X, column=%d, width=%d)", tableP, column, width);
}
break;
case sysTrapTblGetColumnSpacing: {
  // Coord TblGetColumnSpacing(in TableType *tableP, Int16 column)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  column = ARG16;
  resco = TblGetColumnSpacing(tableP ? s_tableP : NULL, column);
  m68k_set_reg(M68K_REG_D0, resco);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetColumnSpacing(tableP=0x%08X, column=%d): %d", tableP, column, resco);
}
break;
case sysTrapTblSetColumnSpacing: {
  // void TblSetColumnSpacing(in TableType *tableP, Int16 column, Coord spacing)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  column = ARG16;
  spacing = ARG16;
  TblSetColumnSpacing(tableP ? s_tableP : NULL, column, spacing);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetColumnSpacing(tableP=0x%08X, column=%d, spacing=%d)", tableP, column, spacing);
}
break;
case sysTrapTblFindRowID: {
  // Boolean TblFindRowID(in TableType *tableP, UInt16 id, out Int16 *rowP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  id = ARG16;
  rowP = ARG32;

  res = (UInt32)TblFindRowID(tableP ? s_tableP : NULL, id, rowP ? &l_rowP : NULL);
  if (rowP) m68k_write_memory_16(rowP, l_rowP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblFindRowID(tableP=0x%08X, id=%d, rowP=0x%08X [%d]): %d", tableP, id, rowP, l_rowP, res);
}
break;
case sysTrapTblFindRowData: {
  // Boolean TblFindRowData(in TableType *tableP, UInt32 data, out Int16 *rowP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  data = ARG32;
  rowP = ARG32;

  res = (UInt32)TblFindRowData(tableP ? s_tableP : NULL, data, rowP ? &l_rowP : NULL);
  if (rowP) m68k_write_memory_16(rowP, l_rowP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblFindRowData(tableP=0x%08X, data=%d, rowP=0x%08X [%d]): %d", tableP, data, rowP, l_rowP, res);
}
break;
case sysTrapTblGetRowID: {
  // UInt16 TblGetRowID(in TableType *tableP, Int16 row)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  res = (UInt32)TblGetRowID(tableP ? s_tableP : NULL, row);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetRowID(tableP=0x%08X, row=%d): %d", tableP, row, res);
}
break;
case sysTrapTblSetRowID: {
  // void TblSetRowID(in TableType *tableP, Int16 row, UInt16 id)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  id = ARG16;
  TblSetRowID(tableP ? s_tableP : NULL, row, id);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowID(tableP=0x%08X, row=%d, id=%d)", tableP, row, id);
}
break;
case sysTrapTblGetRowData: {
  // UInt32 TblGetRowData(in TableType *tableP, Int16 row)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  resUP = TblGetRowData(tableP ? s_tableP : NULL, row);
  m68k_set_reg(M68K_REG_D0, (uint32_t)resUP);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetRowData(tableP=0x%08X, row=%d): 0x%08llX", tableP, row, resUP);
}
break;
case sysTrapTblSetRowData: {
  // void TblSetRowData(in TableType *tableP, Int16 row, UInt32 data)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  data = ARG32;
  TblSetRowData(tableP ? s_tableP : NULL, row, data);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowData(tableP=0x%08X, row=%d, data=0x%08X)", tableP, row, data);
}
break;
case sysTrapTblRowInvalid: {
  // Boolean TblRowInvalid(in TableType *tableP, Int16 row)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  res = (UInt32)TblRowInvalid(tableP ? s_tableP : NULL, row);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblRowInvalid(tableP=0x%08X, row=%d): %d", tableP, row, res);
}
break;
case sysTrapTblMarkRowInvalid: {
  // void TblMarkRowInvalid(in TableType *tableP, Int16 row)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  TblMarkRowInvalid(tableP ? s_tableP : NULL, row);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblMarkRowInvalid(tableP=0x%08X, row=%d)", tableP, row);
}
break;
case sysTrapTblMarkTableInvalid: {
  // void TblMarkTableInvalid(in TableType *tableP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  TblMarkTableInvalid(tableP ? s_tableP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblMarkTableInvalid(tableP=0x%08X)", tableP);
}
break;
case sysTrapTblGetSelection: {
  // Boolean TblGetSelection(in TableType *tableP, out Int16 *rowP, out Int16 *columnP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  rowP = ARG32;

  columnP = ARG32;
  res = (UInt32)TblGetSelection(tableP ? s_tableP : NULL, rowP ? &l_rowP : NULL, columnP ? &l_columnP : NULL);
  if (rowP) m68k_write_memory_16(rowP, l_rowP);
  if (columnP) m68k_write_memory_16(columnP, l_columnP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetSelection(tableP=0x%08X, rowP=0x%08X [%d], columnP=0x%08X [%d]): %d", tableP, rowP, l_rowP, columnP, l_columnP, res);
}
break;
case sysTrapTblInsertRow: {
  // void TblInsertRow(in TableType *tableP, Int16 row)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  TblInsertRow(tableP ? s_tableP : NULL, row);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblInsertRow(tableP=0x%08X, row=%d)", tableP, row);
}
break;
case sysTrapTblRemoveRow: {
  // void TblRemoveRow(in TableType *tableP, Int16 row)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  TblRemoveRow(tableP ? s_tableP : NULL, row);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblRemoveRow(tableP=0x%08X, row=%d)", tableP, row);
}
break;
case sysTrapTblReleaseFocus: {
  // void TblReleaseFocus(in TableType *tableP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  TblReleaseFocus(tableP ? s_tableP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblReleaseFocus(tableP=0x%08X)", tableP);
}
break;
case sysTrapTblEditing: {
  // Boolean TblEditing(in TableType *tableP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  res = (UInt32)TblEditing(tableP ? s_tableP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblEditing(tableP=0x%08X): %d", tableP, res);
}
break;
case sysTrapTblGetCurrentField: {
  // FieldType *TblGetCurrentField(in TableType *tableP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  resft = TblGetCurrentField(tableP ? s_tableP : NULL);
  r_res = emupalmos_trap_out(resft);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetCurrentField(tableP=0x%08X): 0x%08X", tableP, r_res);
}
break;
case sysTrapTblGrabFocus: {
  // void TblGrabFocus(in TableType *tableP, Int16 row, Int16 column)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  column = ARG16;
  TblGrabFocus(tableP ? s_tableP : NULL, row, column);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGrabFocus(tableP=0x%08X, row=%d, column=%d)", tableP, row, column);
}
break;
case sysTrapTblSetColumnEditIndicator: {
  // void TblSetColumnEditIndicator(in TableType *tableP, Int16 column, Boolean editIndicator)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  column = ARG16;
  editIndicator = ARG8;
  TblSetColumnEditIndicator(tableP ? s_tableP : NULL, column, editIndicator);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetColumnEditIndicator(tableP=0x%08X, column=%d, editIndicator=%d)", tableP, column, editIndicator);
}
break;
case sysTrapTblSetRowStaticHeight: {
  // void TblSetRowStaticHeight(in TableType *tableP, Int16 row, Boolean staticHeight)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  staticHeight = ARG8;
  TblSetRowStaticHeight(tableP ? s_tableP : NULL, row, staticHeight);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowStaticHeight(tableP=0x%08X, row=%d, staticHeight=%d)", tableP, row, staticHeight);
}
break;
case sysTrapTblHasScrollBar: {
  // void TblHasScrollBar(in TableType *tableP, Boolean hasScrollBar)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  hasScrollBar = ARG8;
  TblHasScrollBar(tableP ? s_tableP : NULL, hasScrollBar);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblHasScrollBar(tableP=0x%08X, hasScrollBar=%d)", tableP, hasScrollBar);
}
break;
case sysTrapTblGetItemFont: {
  // FontID TblGetItemFont(in TableType *tableP, Int16 row, Int16 column)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  column = ARG16;
  resfont = TblGetItemFont(tableP ? s_tableP : NULL, row, column);
  m68k_set_reg(M68K_REG_D0, resfont);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetItemFont(tableP=0x%08X, row=%d, column=%d): %d", tableP, row, column, resfont);
}
break;
case sysTrapTblSetItemFont: {
  // void TblSetItemFont(in TableType *tableP, Int16 row, Int16 column, FontID fontID)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  column = ARG16;
  fontID = ARG8;
  TblSetItemFont(tableP ? s_tableP : NULL, row, column, fontID);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetItemFont(tableP=0x%08X, row=%d, column=%d, fontID=%d)", tableP, row, column, fontID);
}
break;
case sysTrapTblGetItemPtr: {
  // void *TblGetItemPtr(in TableType *tableP, Int16 row, Int16 column)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  column = ARG16;
  resv = TblGetItemPtr(tableP ? s_tableP : NULL, row, column);
  r_res = emupalmos_trap_out(resv);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetItemPtr(tableP=0x%08X, row=%d, column=%d): 0x%08X", tableP, row, column, r_res);
}
break;
case sysTrapTblRowMasked: {
  // Boolean TblRowMasked(in TableType *tableP, Int16 row)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  res = (UInt32)TblRowMasked(tableP ? s_tableP : NULL, row);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblRowMasked(tableP=0x%08X, row=%d): %d", tableP, row, res);
}
break;
case sysTrapTblSetRowMasked: {
  // void TblSetRowMasked(in TableType *tableP, Int16 row, Boolean masked)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  masked = ARG8;
  TblSetRowMasked(tableP ? s_tableP : NULL, row, masked);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowMasked(tableP=0x%08X, row=%d, masked=%d)", tableP, row, masked);
}
break;
case sysTrapTblSetColumnMasked: {
  // void TblSetColumnMasked(in TableType *tableP, Int16 column, Boolean masked)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  column = ARG16;
  masked = ARG8;
  TblSetColumnMasked(tableP ? s_tableP : NULL, column, masked);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetColumnMasked(tableP=0x%08X, column=%d, masked=%d)", tableP, column, masked);
}
break;
case sysTrapTblGetNumberOfColumns: {
  // Int16 TblGetNumberOfColumns(in TableType *tableP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  res = TblGetNumberOfColumns(tableP ? s_tableP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetNumberOfColumns(tableP=0x%08X): %d", tableP, res);
}
break;
case sysTrapTblGetTopRow: {
  // Int16 TblGetTopRow(in TableType *tableP)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  res = TblGetTopRow(tableP ? s_tableP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetTopRow(tableP=0x%08X): %d", tableP, res);
}
break;
case sysTrapTblSetSelection: {
  // void TblSetSelection(in TableType *tableP, Int16 row, Int16 column)
  tableP = ARG32;
  s_tableP = emupalmos_trap_in(tableP, trap, 0);
  row = ARG16;
  column = ARG16;
  TblSetSelection(tableP ? s_tableP : NULL, row, column);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetSelection(tableP=0x%08X, row=%d, column=%d)", tableP, row, column);
}
break;
case sysTrapLstEraseList: {
  // void LstEraseList(in listP)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  LstEraseList(listP ? s_listP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstEraseList(listP=0x%08X)", listP);
}
break;
case sysTrapLstGetSelection: {
  // Int16 LstGetSelection(in listP)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  res = LstGetSelection(listP ? s_listP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstGetSelection(listP=0x%08X): %d", listP, res);
}
break;
case sysTrapLstGetSelectionText: {
  // Char *LstGetSelectionText(in listP, Int16 itemNum)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  itemNum = ARG16;
  resc = LstGetSelectionText(listP ? s_listP : NULL, itemNum);
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstGetSelectionText(listP=0x%08X, itemNum=%d): 0x%08X", listP, itemNum, r_res);
}
break;
case sysTrapLstHandleEvent: {
  // Boolean LstHandleEvent(in listP, in EventType *eventP)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  eventP = ARG32;

  decode_event(eventP, &l_eventP);
  res = (UInt32)LstHandleEvent(listP ? s_listP : NULL, eventP ? &l_eventP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstHandleEvent(listP=0x%08X, eventP=0x%08X): %d", listP, eventP, res);
}
break;
case sysTrapLstSetHeight: {
  // void LstSetHeight(in listP, Int16 visibleItems)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  visibleItems = ARG16;
  LstSetHeight(listP ? s_listP : NULL, visibleItems);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstSetHeight(listP=0x%08X, visibleItems=%d)", listP, visibleItems);
}
break;
case sysTrapLstSetPosition: {
  // void LstSetPosition(in listP, Coord x, Coord y)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  x = ARG16;
  y = ARG16;
  LstSetPosition(listP ? s_listP : NULL, (uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstSetPosition(listP=0x%08X, x=%d, y=%d)", listP, x, y);
}
break;
case sysTrapLstSetSelection: {
  // void LstSetSelection(in listP, Int16 itemNum)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  itemNum = ARG16;
  LstSetSelection(listP ? s_listP : NULL, itemNum);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstSetSelection(listP=0x%08X, itemNum=%d)", listP, itemNum);
}
break;
case sysTrapLstSetListChoices: {
  // void LstSetListChoices(in listP, in Char **itemsText, Int16 numItems)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  itemsText = ARG32;
  s_itemsText = itemsText ? (char **)(ram + itemsText) : NULL;
  numItems = ARG16;
  LstSetListChoices(listP ? s_listP : NULL, itemsText ? s_itemsText : NULL, numItems);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstSetListChoices(listP=0x%08X, itemsText=0x%08X, numItems=%d)", listP, itemsText, numItems);
}
break;
case sysTrapLstSetTopItem: {
  // void LstSetTopItem(in listP, Int16 itemNum)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  itemNum = ARG16;
  LstSetTopItem(listP ? s_listP : NULL, itemNum);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstSetTopItem(listP=0x%08X, itemNum=%d)", listP, itemNum);
}
break;
case sysTrapLstMakeItemVisible: {
  // void LstMakeItemVisible(in listP, Int16 itemNum)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  itemNum = ARG16;
  LstMakeItemVisible(listP ? s_listP : NULL, itemNum);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstMakeItemVisible(listP=0x%08X, itemNum=%d)", listP, itemNum);
}
break;
case sysTrapLstGetNumberOfItems: {
  // Int16 LstGetNumberOfItems(in listP)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  res = LstGetNumberOfItems(listP ? s_listP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstGetNumberOfItems(listP=0x%08X): %d", listP, res);
}
break;
case sysTrapLstPopupList: {
  // Int16 LstPopupList(in listP)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  res = LstPopupList(listP ? s_listP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstPopupList(listP=0x%08X): %d", listP, res);
}
break;
case sysTrapLstScrollList: {
  // Boolean LstScrollList(in listP, WinDirectionType direction, Int16 itemCount)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  direction = ARG8;
  itemCount = ARG16;
  res = (UInt32)LstScrollList(listP ? s_listP : NULL, (uint8_t)direction, itemCount);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstScrollList(listP=0x%08X, direction=%d, itemCount=%d): %d", listP, direction, itemCount, res);
}
break;
case sysTrapLstGetVisibleItems: {
  // Int16 LstGetVisibleItems(in listP)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  res = LstGetVisibleItems(listP ? s_listP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstGetVisibleItems(listP=0x%08X): %d", listP, res);
}
break;
case sysTrapLstGetTopItem: {
  // Int16 LstGetTopItem(in listP)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  res = LstGetTopItem(listP ? s_listP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstGetTopItem(listP=0x%08X): %d", listP, res);
}
break;
case sysTrapMenuInit: {
  // MenuBarType *MenuInit(UresourceId)
  resourceId = ARG16;
  resmbt = MenuInit(resourceId);
  r_res = emupalmos_trap_out(resmbt);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuInit(resourceId=%d): 0x%08X", resourceId, r_res);
}
break;
case sysTrapMenuGetActiveMenu: {
  // MenuBarType *MenuGetActiveMenu(void)
  resmbt = MenuGetActiveMenu();
  r_res = emupalmos_trap_out(resmbt);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuGetActiveMenu(): 0x%08X", r_res);
}
break;
case sysTrapMenuSetActiveMenu: {
  // MenuBarType *MenuSetActiveMenu(in MenuBarType *menuP)
  menuP = ARG32;
  s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
  resmbt = MenuSetActiveMenu(menuP ? s_menuP : NULL);
  r_res = emupalmos_trap_out(resmbt);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuSetActiveMenu(menuP=0x%08X): 0x%08X", menuP, r_res);
}
break;
case sysTrapMenuDispose: {
  // void MenuDispose(in MenuBarType *menuP)
  menuP = ARG32;
  s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
  MenuDispose(menuP ? s_menuP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuDispose(menuP=0x%08X)", menuP);
}
break;
case sysTrapMenuHandleEvent: {
  // Boolean MenuHandleEvent(in MenuBarType *menuP, in EventType *event, out UInt16 *error)
  menuP = ARG32;
  s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
  event = ARG32;

  decode_event(event, &l_event);
  error = ARG32;
  res = (UInt32)MenuHandleEvent(menuP ? s_menuP : NULL, event ? &l_event : NULL, error ? &l_error : NULL);
  if (error) m68k_write_memory_16(error, l_error);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuHandleEvent(menuP=0x%08X, event=0x%08X, error=0x%08X [%d]): %d", menuP, event, error, l_error, res);
}
break;
case sysTrapMenuDrawMenu: {
  // void MenuDrawMenu(in MenuBarType *menuP)
  menuP = ARG32;
  s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
  MenuDrawMenu(menuP ? s_menuP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuDrawMenu(menuP=0x%08X)", menuP);
}
break;
case sysTrapMenuEraseStatus: {
  // void MenuEraseStatus(in MenuBarType *menuP)
  menuP = ARG32;
  s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
  MenuEraseStatus(menuP ? s_menuP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuEraseStatus(menuP=0x%08X)", menuP);
}
break;
case sysTrapMenuSetActiveMenuRscID: {
  // void MenuSetActiveMenuRscID(UresourceId)
  resourceId = ARG16;
  MenuSetActiveMenuRscID(resourceId);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuSetActiveMenuRscID(resourceId=%d)", resourceId);
}
break;
case sysTrapMenuCmdBarAddButton: {
  // Err MenuCmdBarAddButton(UInt8 where, UInt16 bitmapId, MenuCmdBarResultType resultType, Uresult, in Char *nameP)
  where = ARG8;
  bitmapId = ARG16;
  resultType = ARG8;
  result = ARG32;
  nameP = ARG32;
  s_nameP = nameP ? (char *)(ram + nameP) : NULL;
  res = MenuCmdBarAddButton(where, bitmapId, resultType, result, nameP ? s_nameP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuCmdBarAddButton(where=%d, bitmapId=%d, resultType=%d, result=%d, nameP=0x%08X [%s]): %d", where, bitmapId, resultType, result, nameP, s_nameP, res);
}
break;
case sysTrapMenuCmdBarGetButtonData: {
  // Boolean MenuCmdBarGetButtonData(Int16 buttonIndex, out UInt16 *bitmapIdP, out MenuCmdBarResultType *resultTypeP, out UInt32 *resultP, out Char *nameP)
  buttonIndex = ARG16;
  bitmapIdP = ARG32;
  resultTypeP = ARG32;
  resultP = ARG32;
  nameP = ARG32;
  s_nameP = nameP ? (char *)(ram + nameP) : NULL;
  res = (UInt32)MenuCmdBarGetButtonData(buttonIndex, bitmapIdP ? &l_bitmapIdP : NULL, resultTypeP ? &l_resultTypeP : NULL, resultP ? &l_resultP : NULL, nameP ? s_nameP : NULL);
  if (bitmapIdP) m68k_write_memory_16(bitmapIdP, l_bitmapIdP);
  if (resultTypeP) m68k_write_memory_8(resultTypeP, l_resultTypeP);
  if (resultP) m68k_write_memory_32(resultP, l_resultP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuCmdBarGetButtonData(buttonIndex=%d, bitmapIdP=0x%08X [%d], resultTypeP=0x%08X, resultP=0x%08X [%d], nameP=0x%08X [%s]): %d", buttonIndex, bitmapIdP, l_bitmapIdP, resultTypeP, resultP, l_resultP, nameP, s_nameP, res);
}
break;
case sysTrapMenuCmdBarDisplay: {
  // void MenuCmdBarDisplay(void)
  MenuCmdBarDisplay();
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuCmdBarDisplay()");
}
break;
case sysTrapMenuShowItem: {
  // Boolean MenuShowItem(UInt16 id)
  id = ARG16;
  res = (UInt32)MenuShowItem(id);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuShowItem(id=%d): %d", id, res);
}
break;
case sysTrapMenuHideItem: {
  // Boolean MenuHideItem(UInt16 id)
  id = ARG16;
  res = (UInt32)MenuHideItem(id);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuHideItem(id=%d): %d", id, res);
}
break;
case sysTrapMenuAddItem: {
  // Err MenuAddItem(UInt16 positionId, UInt16 id, Char cmd, in textP)
  positionId = ARG16;
  id = ARG16;
  cmd = ARG8;
  textP = ARG32;
  s_textP = textP ? (char *)(ram + textP) : NULL;
  res = MenuAddItem(positionId, id, (uint8_t)cmd, textP ? s_textP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuAddItem(positionId=%d, id=%d, cmd=%d, textP=0x%08X [%s]): %d", positionId, id, (uint8_t)cmd, textP, s_textP, res);
}
break;
case sysTrapInsPtInitialize: {
  // void InsPtInitialize(void)
  InsPtInitialize();
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtInitialize()");
}
break;
case sysTrapInsPtSetLocation: {
  // void InsPtSetLocation(Int16 x, Int16 y)
  x = ARG16;
  y = ARG16;
  InsPtSetLocation((uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtSetLocation(x=%d, y=%d)", x, y);
}
break;
case sysTrapInsPtGetLocation: {
  // void InsPtGetLocation(out Int16 *x, out Int16 *y)
  x = ARG32;
  y = ARG32;
  InsPtGetLocation(x ? &l_x : NULL, y ? &l_y : NULL);
  if (x) m68k_write_memory_16(x, l_x);
  if (y) m68k_write_memory_16(y, l_y);
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtGetLocation(x=0x%08X [%d], y=0x%08X [%d])", x, l_x, y, l_y);
}
break;
case sysTrapInsPtEnable: {
  // void InsPtEnable(Boolean enableIt)
  enableIt = ARG8;
  InsPtEnable((Boolean)enableIt);
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtEnable(enableIt=%d)", enableIt);
}
break;
case sysTrapInsPtEnabled: {
  // Boolean InsPtEnabled(void)
  res = (UInt32)InsPtEnabled();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtEnabled(): %d", res);
}
break;
case sysTrapInsPtSetHeight: {
  // void InsPtSetHeight(Int16 height)
  height = ARG16;
  InsPtSetHeight(height);
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtSetHeight(height=%d)", height);
}
break;
case sysTrapInsPtGetHeight: {
  // Int16 InsPtGetHeight(void)
  res = InsPtGetHeight();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtGetHeight(): %d", res);
}
break;
case sysTrapInsPtCheckBlink: {
  // void InsPtCheckBlink(void)
  InsPtCheckBlink();
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtCheckBlink()");
}
break;
case sysTrapCtlDrawControl: {
  // void CtlDrawControl(in controlP)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  CtlDrawControl(controlP ? s_controlP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlDrawControl(controlP=0x%08X)", controlP);
}
break;
case sysTrapCtlEraseControl: {
  // void CtlEraseControl(in controlP)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  CtlEraseControl(controlP ? s_controlP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlEraseControl(controlP=0x%08X)", controlP);
}
break;
case sysTrapCtlHideControl: {
  // void CtlHideControl(in controlP)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  CtlHideControl(controlP ? s_controlP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlHideControl(controlP=0x%08X)", controlP);
}
break;
case sysTrapCtlShowControl: {
  // void CtlShowControl(in controlP)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  CtlShowControl(controlP ? s_controlP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlShowControl(controlP=0x%08X)", controlP);
}
break;
case sysTrapCtlEnabled: {
  // Boolean CtlEnabled(in controlP)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  res = (UInt32)CtlEnabled(controlP ? s_controlP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlEnabled(controlP=0x%08X): %d", controlP, res);
}
break;
case sysTrapCtlSetEnabled: {
  // void CtlSetEnabled(in controlP, Boolean usable)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  usable = ARG8;
  CtlSetEnabled(controlP ? s_controlP : NULL, usable);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetEnabled(controlP=0x%08X, usable=%d)", controlP, usable);
}
break;
case sysTrapCtlSetUsable: {
  // void CtlSetUsable(in controlP, Boolean usable)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  usable = ARG8;
  CtlSetUsable(controlP ? s_controlP : NULL, usable);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetUsable(controlP=0x%08X, usable=%d)", controlP, usable);
}
break;
case sysTrapCtlGetValue: {
  // Int16 CtlGetValue(in controlP)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  res = CtlGetValue(controlP ? s_controlP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlGetValue(controlP=0x%08X): %d", controlP, res);
}
break;
case sysTrapCtlSetValue: {
  // void CtlSetValue(in controlP, Int16 newValue)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  newValue = ARG16;
  CtlSetValue(controlP ? s_controlP : NULL, newValue);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetValue(controlP=0x%08X, newValue=%d)", controlP, newValue);
}
break;
case sysTrapCtlSetLabel: {
  // void CtlSetLabel(in controlP, in Char *newLabel)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  newLabel = ARG32;
  s_newLabel = emupalmos_trap_in(newLabel, trap, 1);
  CtlSetLabel(controlP ? s_controlP : NULL, newLabel ? s_newLabel : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetLabel(controlP=0x%08X, newLabel=0x%08X [%s])", controlP, newLabel, s_newLabel);
}
break;
case sysTrapCtlSetGraphics: {
  // void CtlSetGraphics(in ctlP, DmResID newBitmapID, DmResID newSelectedBitmapID)
  ctlP = ARG32;
  s_ctlP = ctlP ? (ControlType *)(ram + ctlP) : NULL;
  newBitmapID = ARG16;
  newSelectedBitmapID = ARG16;
  CtlSetGraphics(ctlP ? s_ctlP : NULL, newBitmapID, newSelectedBitmapID);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetGraphics(ctlP=0x%08X, newBitmapID=%d, newSelectedBitmapID=%d)", ctlP, newBitmapID, newSelectedBitmapID);
}
break;
case sysTrapCtlSetSliderValues: {
  // void CtlSetSliderValues(in ctlP, in UInt16 *minValueP, in UInt16 *maxValueP, in UInt16 *pageSizeP, in UInt16 *valueP)
  ctlP = ARG32;
  s_ctlP = ctlP ? (ControlType *)(ram + ctlP) : NULL;
  minValueP = ARG32;
  if (minValueP) l_minValueP = m68k_read_memory_16(minValueP);
  maxValueP = ARG32;
  if (maxValueP) l_maxValueP = m68k_read_memory_16(maxValueP);
  pageSizeP = ARG32;
  if (pageSizeP) l_pageSizeP = m68k_read_memory_16(pageSizeP);
  valueP = ARG32;
  if (valueP) l_valueP = m68k_read_memory_16(valueP);
  CtlSetSliderValues(ctlP ? s_ctlP : NULL, minValueP ? &l_minValueP : NULL, maxValueP ? &l_maxValueP : NULL, pageSizeP ? &l_pageSizeP : NULL, valueP ? &l_valueP : NULL);
  if (minValueP) m68k_write_memory_16(minValueP, l_minValueP);
  if (maxValueP) m68k_write_memory_16(maxValueP, l_maxValueP);
  if (pageSizeP) m68k_write_memory_16(pageSizeP, l_pageSizeP);
  if (valueP) m68k_write_memory_16(valueP, l_valueP);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetSliderValues(ctlP=0x%08X, minValueP=0x%08X [%d], maxValueP=0x%08X [%d], pageSizeP=0x%08X [%d], valueP=0x%08X [%d])", ctlP, minValueP, l_minValueP, maxValueP, l_maxValueP, pageSizeP, l_pageSizeP, valueP, l_valueP);
}
break;
case sysTrapCtlGetSliderValues: {
  // void CtlGetSliderValues(in ctlP, out UInt16 *minValueP, out UInt16 *maxValueP, out UInt16 *pageSizeP, out UInt16 *valueP)
  ctlP = ARG32;
  s_ctlP = ctlP ? (ControlType *)(ram + ctlP) : NULL;
  minValueP = ARG32;
  maxValueP = ARG32;
  pageSizeP = ARG32;
  valueP = ARG32;
  CtlGetSliderValues(ctlP ? s_ctlP : NULL, minValueP ? &l_minValueP : NULL, maxValueP ? &l_maxValueP : NULL, pageSizeP ? &l_pageSizeP : NULL, valueP ? &l_valueP : NULL);
  if (minValueP) m68k_write_memory_16(minValueP, l_minValueP);
  if (maxValueP) m68k_write_memory_16(maxValueP, l_maxValueP);
  if (pageSizeP) m68k_write_memory_16(pageSizeP, l_pageSizeP);
  if (valueP) m68k_write_memory_16(valueP, l_valueP);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlGetSliderValues(ctlP=0x%08X, minValueP=0x%08X [%d], maxValueP=0x%08X [%d], pageSizeP=0x%08X [%d], valueP=0x%08X [%d])", ctlP, minValueP, l_minValueP, maxValueP, l_maxValueP, pageSizeP, l_pageSizeP, valueP, l_valueP);
}
break;
case sysTrapCtlHitControl: {
  // void CtlHitControl(in controlP)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  CtlHitControl(controlP ? s_controlP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlHitControl(controlP=0x%08X)", controlP);
}
break;
case sysTrapCtlHandleEvent: {
  // Boolean CtlHandleEvent(in controlP, in EventType *pEvent)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  pEvent = ARG32;
  decode_event(pEvent, &l_pEvent);
  res = (UInt32)CtlHandleEvent(controlP ? s_controlP : NULL, pEvent ? &l_pEvent : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlHandleEvent(controlP=0x%08X, pEvent=0x%08X): %d", controlP, pEvent, res);
}
break;
case sysTrapCtlValidatePointer: {
  // Boolean CtlValidatePointer(in controlP)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  res = (UInt32)CtlValidatePointer(controlP ? s_controlP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlValidatePointer(controlP=0x%08X): %d", controlP, res);
}
break;
case sysTrapFileOpen: {
  // FileHand FileOpen(UInt16 cardNo, in Char *nameP, UInt32 type, UInt32 creator, UInt32 openMode, out Err *errP)
  cardNo = ARG16;
  nameP = ARG32;
  s_nameP = nameP ? (char *)(ram + nameP) : NULL;
  type = ARG32;
  creator = ARG32;
  openMode = ARG32;
  errP = ARG32;

  resfh = FileOpen(cardNo, nameP ? s_nameP : NULL, type, creator, openMode, errP ? &l_errP : NULL);
  if (errP) m68k_write_memory_16(errP, l_errP);
  r_res = emupalmos_trap_out(resfh);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileOpen(cardNo=%d, nameP=0x%08X [%s], type=%d, creator=%d, openMode=0x%08X, errP=0x%08X): 0x%08X", cardNo, nameP, s_nameP, type, creator, openMode, errP, r_res);
}
break;
case sysTrapFileClose: {
  // Err FileClose(FileHand stream)
  stream = ARG32;
  l_stream = stream ? (FileHand)(ram + stream) : NULL;
  res = FileClose(stream ? l_stream : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileClose(stream=0x%08X): %d", stream, res);
}
break;
case sysTrapFileDelete: {
  // Err FileDelete(UInt16 cardNo, in Char *nameP)
  cardNo = ARG16;
  nameP = ARG32;
  s_nameP = nameP ? (char *)(ram + nameP) : NULL;
  res = FileDelete(cardNo, nameP ? s_nameP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileDelete(cardNo=%d, nameP=0x%08X [%s]): %d", cardNo, nameP, s_nameP, res);
}
break;
case sysTrapFileReadLow: {
  // Int32 FileReadLow(FileHand stream, out void *baseP, Int32 offset, Boolean dataStoreBased, Int32 objSize, Int32 numObj, out Err *errP)
  stream = ARG32;
  l_stream = stream ? (FileHand)(ram + stream) : NULL;
  baseP = ARG32;
  s_baseP = baseP ? (void *)(ram + baseP) : NULL;
  offset = ARG32;
  dataStoreBased = ARG8;
  objSize = ARG32;
  numObj = ARG32;
  errP = ARG32;

  res = FileReadLow(stream ? l_stream : 0, baseP ? s_baseP : NULL, offset, dataStoreBased, objSize, numObj, errP ? &l_errP : NULL);
  if (errP) m68k_write_memory_16(errP, l_errP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileReadLow(stream=0x%08X, baseP=0x%08X, offset=%d, dataStoreBased=%d, objSize=%d, numObj=%d, errP=0x%08X): %d", stream, baseP, offset, dataStoreBased, objSize, numObj, errP, res);
}
break;
case sysTrapFileWrite: {
  // Int32 FileWrite(FileHand stream, in void *dataP, Int32 objSize, Int32 numObj, out Err *errP)
  stream = ARG32;
  l_stream = stream ? (FileHand)(ram + stream) : NULL;
  dataP = ARG32;
  s_dataP = dataP ? (void *)(ram + dataP) : NULL;
  objSize = ARG32;
  numObj = ARG32;
  errP = ARG32;
  res = FileWrite(stream ? l_stream : 0, dataP ? s_dataP : NULL, objSize, numObj, errP ? &l_errP : NULL);
  if (errP) m68k_write_memory_16(errP, l_errP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileWrite(stream=0x%08X, dataP=0x%08X, objSize=%d, numObj=%d, errP=0x%08X): %d", stream, dataP, objSize, numObj, errP, res);
}
break;
case sysTrapFileSeek: {
  // Err FileSeek(FileHand stream, Int32 offset, FileOriginEnum origin)
  stream = ARG32;
  l_stream = stream ? (FileHand)(ram + stream) : NULL;
  offset = ARG32;
  origin = ARG8;
  res = FileSeek(stream ? l_stream : 0, offset, origin);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileSeek(stream=0x%08X, offset=%d, origin=%d): %d", stream, offset, origin, res);
}
break;
case sysTrapFileTell: {
  // Int32 FileTell(FileHand stream, out Int32 *fileSizeP, out Err *errP)
  stream = ARG32;
  l_stream = stream ? (FileHand)(ram + stream) : NULL;
  fileSizeP = ARG32;
  errP = ARG32;
  res = FileTell(stream ? l_stream : 0, fileSizeP ? &l_fileSizeP : NULL, errP ? &l_errP : NULL);
  if (fileSizeP) m68k_write_memory_32(fileSizeP, l_fileSizeP);
  if (errP) m68k_write_memory_16(errP, l_errP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileTell(stream=0x%08X, fileSizeP=0x%08X [%d], errP=0x%08X): %d", stream, fileSizeP, l_fileSizeP, errP, res);
}
break;
case sysTrapFileTruncate: {
  // Err FileTruncate(FileHand stream, Int32 newSize)
  stream = ARG32;
  l_stream = stream ? (FileHand)(ram + stream) : NULL;
  newSize = ARG32;
  res = FileTruncate(stream ? l_stream : 0, newSize);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileTruncate(stream=0x%08X, newSize=%d): %d", stream, newSize, res);
}
break;
case sysTrapSysTicksPerSecond: {
  // UInt16 SysTicksPerSecond(void)
  res = (UInt32)SysTicksPerSecond();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysTicksPerSecond(): %d", res);
}
break;
case sysTrapSysHandleEvent: {
  // Boolean SysHandleEvent(in EventType *eventP)
  eventP = ARG32;

  decode_event(eventP, &l_eventP);
  res = (UInt32)SysHandleEvent(eventP ? &l_eventP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysHandleEvent(eventP=0x%08X): %d", eventP, res);
}
break;
case sysTrapSysUIBusy: {
  // UInt16 SysUIBusy(Boolean set, Boolean value)
  set = ARG8;
  value = ARG8;
  res = (UInt32)SysUIBusy(set, (uint8_t)value);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysUIBusy(%u, %u): %u", set, value, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapSysUIAppSwitch: {
  // Err SysUIAppSwitch(UInt16 cardNo, dbID, UInt16 cmd, MemPtr cmdPBP)
  cardNo = ARG16;
  dbID = ARG32;
  cmd = ARG16;
  cmdPBP = ARG32;
  r = 0;
  if (cmdPBP) {
    r = decode_launch(cmd, cmdPBP, &param);
  }
  res = sysErrParamErr;
  if (r == 0) {
    res = SysUIAppSwitch(cardNo, dbID, cmd, cmdPBP ? &param : NULL);
  } else {
    debug(DEBUG_TRACE, "EmuPalmOS", "SysUIAppSwitch invalid param type %d", cmd);
  }
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysUIAppSwitch(cardNo=%d, dbID=0x%08X, cmd=%d, cmdPBP=0x%08X)", cardNo, dbID, cmd, cmdPBP);
}
break;
case sysTrapSysCurAppDatabase: {
  // Err SysCurAppDatabase(out UInt16 *cardNoP, out LocalID *dbIDP)
  cardNoP = ARG32;
  dbIDP = ARG32;
  res = SysCurAppDatabase(cardNoP ? &l_cardNoP : NULL, dbIDP ? &l_dbIDP : NULL);
  if (cardNoP) m68k_write_memory_16(cardNoP, l_cardNoP);
  if (dbIDP) m68k_write_memory_32(dbIDP, l_dbIDP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysCurAppDatabase(cardNoP=0x%08X [%d], dbIDP=0x%08X): %d", cardNoP, l_cardNoP, dbIDP, res);
}
break;
case sysTrapSysAppLaunch: {
  // Err SysAppLaunch(UInt16 cardNo, dbID, UInt16 launchFlags, UInt16 cmd, MemPtr cmdPBP, UInt32 *resultP)
  cardNo = ARG16;
  dbID = ARG32;
  launchFlags = ARG16;
  cmd = ARG16;
  cmdPBP = ARG32;
  resultP = ARG32;
  r = 0;
  if (cmdPBP) {
    r = decode_launch(cmd, cmdPBP, &param);
  }
  res = sysErrParamErr;
  if (r == 0) {
    res = SysAppLaunch(cardNo, dbID, launchFlags, cmd, cmdPBP ? &param : NULL, resultP ? &l_resultP : NULL);
  } else {
    debug(DEBUG_TRACE, "EmuPalmOS", "SysAppLaunch invalid param type %d", cmd);
  }
  if (resultP) m68k_write_memory_32(resultP, l_resultP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysAppLaunch(cardNo=%d, dbID=0x%08X, launchFlags=%d, cmd=%d, cmdPBP=0x%08X, resultP=0x%08X [%d]): %d", cardNo, dbID, launchFlags, cmd, cmdPBP, resultP, l_resultP, res);
}
break;
case sysTrapSysLCDContrast: {
  // UInt8 SysLCDContrast(Boolean set, UInt8 newContrastLevel)
  set = ARG8;
  newContrastLevel = ARG8;
  res = SysLCDContrast(set, newContrastLevel);
  m68k_set_reg(M68K_REG_D0, (UInt8 )res);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysLCDContrast(set=%d, newContrastLevel=%d): %d", set, newContrastLevel, res);
}
break;
case sysTrapSysLCDBrightness: {
  // UInt8 SysLCDBrightness(Boolean set, UInt8 newBrightnessLevel)
  set = ARG8;
  newBrightnessLevel = ARG8;
  res = SysLCDBrightness(set, newBrightnessLevel);
  m68k_set_reg(M68K_REG_D0, (UInt8)res);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysLCDBrightness(set=%d, newBrightnessLevel=%d): %d", set, newBrightnessLevel, res);
}
break;
case sysTrapSysGetOSVersionString: {
  // sysGetOSVersionString()
  resc = SysGetOSVersionString();
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysGetOSVersionString(): 0x%08X", r_res);
}
break;
case sysTrapSysBatteryInfo: {
  // UInt16 SysBatteryInfo(Boolean set, out UInt16 *warnThresholdP, out UInt16 *criticalThresholdP, out Int16 *maxTicksP, out SysBatteryKind *kindP, out Boolean *pluggedIn, out UInt8 *percentP)
  set = ARG8;
  warnThresholdP = ARG32;
  criticalThresholdP = ARG32;
  maxTicksP = ARG32;
  kindP = ARG32;
  pluggedIn = ARG32;
  percentP = ARG32;
  res = (UInt32)SysBatteryInfo(set, warnThresholdP ? &l_warnThresholdP : NULL, criticalThresholdP ? &l_criticalThresholdP : NULL, maxTicksP ? &l_maxTicksP : NULL, kindP ? &l_kindP : NULL, pluggedIn ? &l_pluggedIn : NULL, percentP ? &l_percentP : NULL);
  if (warnThresholdP) m68k_write_memory_16(warnThresholdP, l_warnThresholdP);
  if (criticalThresholdP) m68k_write_memory_16(criticalThresholdP, l_criticalThresholdP);
  if (maxTicksP) m68k_write_memory_16(maxTicksP, l_maxTicksP);
  if (pluggedIn) m68k_write_memory_8(pluggedIn, l_pluggedIn);
  if (percentP) m68k_write_memory_8(percentP, l_percentP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysBatteryInfo(set=%d, warnThresholdP=0x%08X [%d], criticalThresholdP=0x%08X [%d], maxTicksP=0x%08X [%d], kindP=0x%08X, pluggedIn=0x%08X, percentP=0x%08X): %d", set, warnThresholdP, l_warnThresholdP, criticalThresholdP, l_criticalThresholdP, maxTicksP, l_maxTicksP, kindP, pluggedIn, percentP, res);
}
break;
case sysTrapSysBatteryInfoV20: {
  // UInt16 SysBatteryInfoV20(Boolean set, UInt16 *warnThresholdP, UInt16 *criticalThresholdP, UInt16 *maxTicksP, SysBatteryKind *kindP, Boolean *pluggedIn)
  set = ARG8;
  warnThresholdP = ARG32;
  criticalThresholdP = ARG32;
  maxTicksP = ARG32;
  kindP = ARG32;
  pluggedIn = ARG32;
  res = (UInt32)SysBatteryInfoV20(set, warnThresholdP ? &l_warnThresholdP : NULL, criticalThresholdP ? &l_criticalThresholdP : NULL, maxTicksP ? &l_maxTicksP : NULL, kindP ? &l_kindP : NULL, pluggedIn ? &l_pluggedIn : NULL);
  if (warnThresholdP) m68k_write_memory_16(warnThresholdP, l_warnThresholdP);
  if (criticalThresholdP) m68k_write_memory_16(criticalThresholdP, l_criticalThresholdP);
  if (maxTicksP) m68k_write_memory_16(maxTicksP, l_maxTicksP);
  if (pluggedIn) m68k_write_memory_8(pluggedIn, l_pluggedIn);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysBatteryInfoV20(set=%d, warnThresholdP=0x%08X [%d], criticalThresholdP=0x%08X [%d], maxTicksP=0x%08X [%d], kindP=0x%08X, pluggedIn=0x%08X): %d", set, warnThresholdP, l_warnThresholdP, criticalThresholdP, l_criticalThresholdP, maxTicksP, l_maxTicksP, kindP, pluggedIn, res);
}
break;
case sysTrapKeyCurrentState: {
  // UInt32 KeyCurrentState(void)
  res = (UInt32)KeyCurrentState();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "KeyCurrentState(): %d", res);
}
break;
case sysTrapKeyRates: {
  // Err KeyRates(Boolean set, inout UInt16 *initDelayP, inout UInt16 *periodP, inout UInt16 *doubleTapDelayP, inout Boolean *queueAheadP)
  set = ARG8;
  initDelayP = ARG32;
  if (initDelayP) l_initDelayP = m68k_read_memory_16(initDelayP);
  periodP = ARG32;
  if (periodP) l_periodP = m68k_read_memory_16(periodP);
  doubleTapDelayP = ARG32;
  if (doubleTapDelayP) l_doubleTapDelayP = m68k_read_memory_16(doubleTapDelayP);
  queueAheadP = ARG32;
  if (queueAheadP) l_queueAheadP = m68k_read_memory_8(queueAheadP);
  res = KeyRates(set, initDelayP ? &l_initDelayP : NULL, periodP ? &l_periodP : NULL, doubleTapDelayP ? &l_doubleTapDelayP : NULL, queueAheadP ? &l_queueAheadP : NULL);
  if (initDelayP) m68k_write_memory_16(initDelayP, l_initDelayP);
  if (periodP) m68k_write_memory_16(periodP, l_periodP);
  if (doubleTapDelayP) m68k_write_memory_16(doubleTapDelayP, l_doubleTapDelayP);
  if (queueAheadP) m68k_write_memory_8(queueAheadP, l_queueAheadP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "KeyRates(set=%d, initDelayP=0x%08X [%d], periodP=0x%08X [%d], doubleTapDelayP=0x%08X [%d], queueAheadP=0x%08X): %d", set, initDelayP, l_initDelayP, periodP, l_periodP, doubleTapDelayP, l_doubleTapDelayP, queueAheadP, res);
}
break;
case sysTrapKeySetMask: {
  // UInt32 KeySetMask(UInt32 keyMask)
  uint32_t keyMask = ARG32;
  res = (UInt32)KeySetMask(keyMask);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "KeySetMask(keyMask=%d): %d", keyMask, res);
}
break;
case sysTrapCategoryCreateListV10: {
  // void CategoryCreateListV10(DmOpenRef db, in ListType *lst, UInt16 currentCategory, Boolean showAll)
  db = ARG32;
  l_db = db ? (DmOpenRef)(ram + db) : NULL;
  lst = ARG32;
  s_lst = lst ? (ListType *)(ram + lst) : NULL;
  currentCategory = ARG16;
  showAll = ARG8;
  CategoryCreateListV10(db ? l_db : 0, lst ? s_lst : NULL, currentCategory, showAll);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryCreateListV10(db=0x%08X, lst=0x%08X, currentCategory=%d, showAll=%d)", db, lst, currentCategory, showAll);
}
break;
case sysTrapCategoryCreateList: {
  // void CategoryCreateList(DmOpenRef db, in listP, UInt16 currentCategory, Boolean showAll, Boolean showUneditables, UInt8 numUneditableCategories, UInt32 editingStrID, Boolean resizeList)
  db = ARG32;
  l_db = db ? (DmOpenRef)(ram + db) : NULL;
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  currentCategory = ARG16;
  showAll = ARG8;
  showUneditables = ARG8;
  numUneditableCategories = ARG8;
  editingStrID = ARG32;
  resizeList = ARG8;
  CategoryCreateList(db ? l_db : 0, listP ? s_listP : NULL, currentCategory, showAll, showUneditables, numUneditableCategories, editingStrID, resizeList);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryCreateList(db=0x%08X, listP=0x%08X, currentCategory=%d, showAll=%d, showUneditables=%d, numUneditableCategories=%d, editingStrID=%d, resizeList=%d)", db, listP, currentCategory, showAll, showUneditables, numUneditableCategories, editingStrID, resizeList);
}
break;
case sysTrapCategoryFreeListV10: {
  // void CategoryFreeListV10(DmOpenRef db, in ListType *lst)
  db = ARG32;
  l_db = db ? (DmOpenRef)(ram + db) : NULL;
  lst = ARG32;
  s_lst = lst ? (ListType *)(ram + lst) : NULL;
  CategoryFreeListV10(db ? l_db : 0, lst ? s_lst : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryFreeListV10(db=0x%08X, lst=0x%08X)", db, lst);
}
break;
case sysTrapCategoryFreeList: {
  // void CategoryFreeList(DmOpenRef db, in listP, Boolean showAll, UInt32 editingStrID)
  db = ARG32;
  l_db = db ? (DmOpenRef)(ram + db) : NULL;
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  showAll = ARG8;
  editingStrID = ARG32;
  CategoryFreeList(db ? l_db : 0, listP ? s_listP : NULL, showAll, editingStrID);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryFreeList(db=0x%08X, listP=0x%08X, showAll=%d, editingStrID=%d)", db, listP, showAll, editingStrID);
}
break;
case sysTrapCategoryFind: {
  // UInt16 CategoryFind(DmOpenRef db, in Char *name)
  db = ARG32;
  l_db = db ? (DmOpenRef)(ram + db) : NULL;
  name32 = ARG32;
  s_name = name32 ? (char *)(ram + name32) : NULL;
  res = (UInt32)CategoryFind(db ? l_db : 0, name32 ? s_name : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryFind(db=0x%08X, name=0x%08X [%s]): %d", db, name32, s_name, res);
}
break;
case sysTrapCategoryGetName: {
  // void CategoryGetName(DmOpenRef db, UInt16 index, out Char *name)
  db = ARG32;
  l_db = db ? (DmOpenRef)(ram + db) : NULL;
  index = ARG16;
  name32 = ARG32;
  s_name = name32 ? (char *)(ram + name32) : NULL;
  CategoryGetName(db ? l_db : 0, index, name32 ? s_name : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryGetName(db=0x%08X, index=%d, name=0x%08X [%s])", db, index, name32, s_name);
}
break;
case sysTrapCategoryEditV10: {
  // Boolean CategoryEditV10(DmOpenRef db, inout UInt16 *category)
  db = ARG32;
  l_db = db ? (DmOpenRef)(ram + db) : NULL;
  category = ARG32;
  if (category) l_category = m68k_read_memory_16(category);
  res = (UInt32)CategoryEditV10(db ? l_db : 0, category ? &l_category : NULL);
  if (category) m68k_write_memory_16(category, l_category);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryEditV10(db=0x%08X, category=0x%08X [%d]): %d", db, category, l_category, res);
}
break;
case sysTrapCategoryEditV20: {
  // Boolean CategoryEditV20(DmOpenRef db, inout UInt16 *category, UInt32 titleStrID)
  db = ARG32;
  l_db = db ? (DmOpenRef)(ram + db) : NULL;
  category = ARG32;
  if (category) l_category = m68k_read_memory_16(category);
  titleStrID = ARG32;
  res = (UInt32)CategoryEditV20(db ? l_db : 0, category ? &l_category : NULL, titleStrID);
  if (category) m68k_write_memory_16(category, l_category);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryEditV20(db=0x%08X, category=0x%08X [%d], titleStrID=%d): %d", db, category, l_category, titleStrID, res);
}
break;
case sysTrapCategoryEdit: {
  // Boolean CategoryEdit(DmOpenRef db, inout UInt16 *category, UInt32 titleStrID, UInt8 numUneditableCategories)
  db = ARG32;
  l_db = db ? (DmOpenRef)(ram + db) : NULL;
  category = ARG32;
  if (category) l_category = m68k_read_memory_16(category);
  titleStrID = ARG32;
  numUneditableCategories = ARG8;
  res = (UInt32)CategoryEdit(db ? l_db : 0, category ? &l_category : NULL, titleStrID, numUneditableCategories);
  if (category) m68k_write_memory_16(category, l_category);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryEdit(db=0x%08X, category=0x%08X [%d], titleStrID=%d, numUneditableCategories=%d): %d", db, category, l_category, titleStrID, numUneditableCategories, res);
}
break;
case sysTrapCategorySelectV10: {
  // Boolean CategorySelectV10(DmOpenRef db, in FormType *frm, UInt16 ctlID, UInt16 lstID, Boolean title, out UInt16 *categoryP, out Char *categoryName)
  db = ARG32;
  l_db = db ? (DmOpenRef)(ram + db) : NULL;
  frm = ARG32;
  s_frm = frm ? (FormType *)(ram + frm) : NULL;
  ctlID = ARG16;
  lstID = ARG16;
  title8 = ARG8;
  categoryP = ARG32;
  categoryName = ARG32;
  s_categoryName = categoryName ? (char *)(ram + categoryName) : NULL;
  res = (UInt32)CategorySelectV10(db ? l_db : 0, frm ? s_frm : NULL, ctlID, lstID, title8, categoryP ? &l_categoryP : NULL, categoryName ? s_categoryName : NULL);
  if (categoryP) m68k_write_memory_16(categoryP, l_categoryP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategorySelectV10(db=0x%08X, frm=0x%08X, ctlID=%d, lstID=%d, title=%d, categoryP=0x%08X [%d], categoryName=0x%08X [%s]): %d", db, frm, ctlID, lstID, title8, categoryP, l_categoryP, categoryName, s_categoryName, res);
}
break;
case sysTrapCategorySelect: {
  // Boolean CategorySelect(DmOpenRef db, in FormType *frm, UInt16 ctlID, UInt16 lstID, Boolean title, out UInt16 *categoryP, out Char *categoryName, UInt8 numUneditableCategories, UInt32 editingStrID)
  db = ARG32;
  l_db = db ? (DmOpenRef)(ram + db) : NULL;
  frm = ARG32;
  s_frm = frm ? (FormType *)(ram + frm) : NULL;
  ctlID = ARG16;
  lstID = ARG16;
  title8 = ARG8;
  categoryP = ARG32;
  categoryName = ARG32;
  s_categoryName = categoryName ? (char *)(ram + categoryName) : NULL;
  numUneditableCategories = ARG8;
  editingStrID = ARG32;
  res = (UInt32)CategorySelect(db ? l_db : 0, frm ? s_frm : NULL, ctlID, lstID, title8, categoryP ? &l_categoryP : NULL, categoryName ? s_categoryName : NULL, numUneditableCategories, editingStrID);
  if (categoryP) m68k_write_memory_16(categoryP, l_categoryP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategorySelect(db=0x%08X, frm=0x%08X, ctlID=%d, lstID=%d, title=%d, categoryP=0x%08X [%d], categoryName=0x%08X [%s], numUneditableCategories=%d, editingStrID=%d): %d", db, frm, ctlID, lstID, title8, categoryP, l_categoryP, categoryName, s_categoryName, numUneditableCategories, editingStrID, res);
}
break;
case sysTrapCategoryGetNext: {
  // UInt16 CategoryGetNext(DmOpenRef db, UInt16 index)
  db = ARG32;
  l_db = db ? (DmOpenRef)(ram + db) : NULL;
  index = ARG16;
  res = (UInt32)CategoryGetNext(db ? l_db : 0, index);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryGetNext(db=0x%08X, index=%d): %d", db, index, res);
}
break;
case sysTrapCategorySetTriggerLabel: {
  // void CategorySetTriggerLabel(in ctl, Char *name)
  ctl = ARG32;
  s_ctl = ctl ? (ControlType *)(ram + ctl) : NULL;
  name32 = ARG32;
  s_name = name32 ? (char *)(ram + name32) : NULL;
  CategorySetTriggerLabel(ctl ? s_ctl : NULL, name32 ? s_name : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategorySetTriggerLabel(ctl=0x%08X, name=0x%08X [%s])", ctl, name32, s_name);
}
break;
case sysTrapCategoryTruncateName: {
  // void CategoryTruncateName(inout Char *name, UInt16 maxWidth)
  name32 = ARG32;
  s_name = name32 ? (char *)(ram + name32) : NULL;
  maxWidth = ARG16;
  CategoryTruncateName(name32 ? s_name : NULL, maxWidth);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryTruncateName(name=0x%08X [%s], maxWidth=%d)", name32, s_name, maxWidth);
}
break;
case sysTrapCategoryInitialize: {
  // void CategoryInitialize(inout AppInfoType *appInfoP, UInt16 localizedAppInfoStrID)
  appInfoP = ARG32;
  if ((l_appInfoP = MemPtrNew(sizeof(AppInfoType))) != NULL) {
    decode_appinfo(appInfoP, l_appInfoP);
    localizedAppInfoStrID = ARG16;
    CategoryInitialize(appInfoP ? l_appInfoP : NULL, localizedAppInfoStrID);
    encode_appinfo(appInfoP, l_appInfoP);
    MemPtrFree(l_appInfoP);
    debug(DEBUG_TRACE, "EmuPalmOS", "CategoryInitialize(appInfoP=0x%08X, localizedAppInfoStrID=%d)", appInfoP, localizedAppInfoStrID);
  }
}
break;
case sysTrapCategorySetName: {
  // void CategorySetName(DmOpenRef db, UInt16 index, in Char *nameP)
  db = ARG32;
  l_db = db ? (DmOpenRef)(ram + db) : NULL;
  index = ARG16;
  nameP = ARG32;
  s_nameP = nameP ? (char *)(ram + nameP) : NULL;
  CategorySetName(db ? l_db : 0, index, nameP ? s_nameP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategorySetName(db=0x%08X, index=%d, nameP=0x%08X [%s])", db, index, nameP, s_nameP);
}
break;
case sysTrapPwdExists: {
  // Boolean PwdExists(void)
  res = (UInt32)PwdExists();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "PwdExists(): %d", res);
}
break;
