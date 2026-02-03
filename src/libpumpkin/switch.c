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
  p = ARG32;
  l_p = emupalmos_trap_in(p, trap, 0);
  resmem = MemPtrRecoverHandle(p ? l_p : 0);
  r_res = emupalmos_trap_out(resmem);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrRecoverHandle(p=0x%08X): %p", p, resmem);
}
break;
case sysTrapMemPtrFlags: {
  // UInt16 MemPtrFlags(MemPtr p)
  p = ARG32;
  l_p = emupalmos_trap_in(p, trap, 0);
  res = (UInt32)MemPtrFlags(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrFlags(p=0x%08X): %d", p, res);
}
break;
case sysTrapMemPtrSize: {
  // UInt32 MemPtrSize(MemPtr p)
  p = ARG32;
  l_p = emupalmos_trap_in(p, trap, 0);
  res = (UInt32)MemPtrSize(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrSize(p=0x%08X): %d", p, res);
}
break;
case sysTrapMemPtrOwner: {
  // UInt16 MemPtrOwner(MemPtr p)
  p = ARG32;
  l_p = emupalmos_trap_in(p, trap, 0);
  res = (UInt32)MemPtrOwner(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrOwner(p=0x%08X): %d", p, res);
}
break;
case sysTrapMemPtrHeapID: {
  // UInt16 MemPtrHeapID(MemPtr p)
  p = ARG32;
  l_p = emupalmos_trap_in(p, trap, 0);
  res = (UInt32)MemPtrHeapID(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrHeapID(p=0x%08X): %d", p, res);
}
break;
case sysTrapMemPtrDataStorage: {
  // Boolean MemPtrDataStorage(MemPtr p)
  p = ARG32;
  l_p = emupalmos_trap_in(p, trap, 0);
  res = (UInt32)MemPtrDataStorage(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrDataStorage(p=0x%08X): %d", p, res);
}
break;
case sysTrapMemPtrCardNo: {
  // UInt16 MemPtrCardNo(MemPtr p)
  p = ARG32;
  l_p = emupalmos_trap_in(p, trap, 0);
  res = (UInt32)MemPtrCardNo(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrCardNo(p=0x%08X): %d", p, res);
}
break;
case sysTrapMemPtrToLocalID: {
  // LocalID MemPtrToLocalID(MemPtr p)
  p = ARG32;
  l_p = emupalmos_trap_in(p, trap, 0);
  resid = MemPtrToLocalID(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, resid);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrToLocalID(p=0x%08X): 0x%08X", p, resid);
}
break;
case sysTrapMemPtrSetOwner: {
  // Err MemPtrSetOwner(MemPtr p, UInt16 owner)
  p = ARG32;
  l_p = emupalmos_trap_in(p, trap, 0);
  owner = ARG16;
  res = MemPtrSetOwner(p ? l_p : 0, owner);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrSetOwner(p=0x%08X, owner=%d): %d", p, owner, res);
}
break;
case sysTrapMemPtrResize: {
  // Err MemPtrResize(MemPtr p, UInt32 newSize)
  p = ARG32;
  l_p = emupalmos_trap_in(p, trap, 0);
  newSize = ARG32;
  res = MemPtrResize(p ? l_p : 0, newSize);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrResize(p=0x%08X, newSize=%d): %d", p, newSize, res);
}
break;
case sysTrapMemPtrResetLock: {
  // Err MemPtrResetLock(MemPtr p)
  p = ARG32;
  l_p = emupalmos_trap_in(p, trap, 0);
  res = MemPtrResetLock(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrResetLock(p=0x%08X): %d", p, res);
}
break;
case sysTrapMemPtrUnlock: {
  // Err MemPtrUnlock(MemPtr p)
  p = ARG32;
  l_p = emupalmos_trap_in(p, trap, 0);
  res = MemPtrUnlock(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrUnlock(p=0x%08X): %d", p, res);
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
  s1 = ARG32;
  s_s1 = emupalmos_trap_in(s1, trap, 0);
  s2 = ARG32;
  s_s2 = emupalmos_trap_in(s2, trap, 1);
  numBytes = ARG32;
  res = MemCmp(s1 ? s_s1 : NULL, s2 ? s_s2 : NULL, numBytes);
  m68k_set_reg(M68K_REG_D0, (Int16)res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemCmp(s1=0x%08X, s2=0x%08X, numBytes=%d): %d", s1, s2, numBytes, res);
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
  // Err DmStrCopy(out void *recordP, UInt32 offset, in Char *srcP)
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
  source = ARG32;
  decode_rectangle(source, &l_source);
  error = ARG32;
  reswh = WinSaveBits(source ? &l_source : NULL, error ? &l_error : NULL);
  if (error) m68k_write_memory_16(error, l_error);
  r_res = emupalmos_trap_out(reswh);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSaveBits(source=0x%08X [%d,%d,%d,%d], error=0x%08X [%d]): 0x%08X", source, l_source.topLeft.x, l_source.topLeft.y, l_source.extent.x, l_source.extent.y, error, l_error, r_res);
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
  uint16_t frame = ARG16;
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  WinPaintRectangleFrame(frame, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintRectangleFrame(frame=%d, rP=0x%08X [%d,%d,%d,%d])", frame, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinDrawRectangleFrame: {
  // void WinDrawRectangleFrame(FrameType frame, in RectangleType *rP)
  uint16_t frame = ARG16;
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  WinDrawRectangleFrame(frame, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawRectangleFrame(frame=%d, rP=0x%08X [%d,%d,%d,%d])", frame, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinDrawGrayRectangleFrame: {
  // void WinDrawGrayRectangleFrame(FrameType frame, in RectangleType *rP)
  uint16_t frame = ARG16;
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  WinDrawGrayRectangleFrame(frame, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawGrayRectangleFrame(frame=%d, rP=0x%08X [%d,%d,%d,%d])", frame, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinEraseRectangleFrame: {
  // void WinEraseRectangleFrame(FrameType frame, in RectangleType *rP)
  uint16_t frame = ARG16;
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  WinEraseRectangleFrame(frame, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinEraseRectangleFrame(frame=%d, rP=0x%08X [%d,%d,%d,%d])", frame, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinInvertRectangleFrame: {
  // void WinInvertRectangleFrame(FrameType frame, in RectangleType *rP)
  uint16_t frame = ARG16;
  rP = ARG32;

  decode_rectangle(rP, &l_rP);
  WinInvertRectangleFrame(frame, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinInvertRectangleFrame(frame=%d, rP=0x%08X [%d,%d,%d,%d])", frame, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinDrawBitmap: {
  // void WinDrawBitmap(in BitmapType *bitmapP, Coord x, Coord y)
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
  // void WinPaintBitmap(in BitmapType *bitmapP, Coord x, Coord y)
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
  // Err BmpCompress(in BitmapType *bitmapP, BitmapCompressionType compType)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  compType = ARG8;
  res = BmpCompress(bitmapP ? l_bitmapP : NULL, compType);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpCompress(bitmapP=0x%08X, compType=%d): %d", bitmapP, compType, res);
}
break;
case sysTrapBmpGetBits: {
  // void *BmpGetBits(in BitmapType *bitmapP)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  resv = BmpGetBits(bitmapP ? l_bitmapP : NULL);
  r_res = emupalmos_trap_out(resv);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetBits(bitmapP=0x%08X): 0x%08X", bitmapP, r_res);
}
break;
case sysTrapBmpGetColortable: {
  // ColorTableType *BmpGetColortable(in BitmapType *bitmapP)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  resctt = BmpGetColortable(bitmapP ? l_bitmapP : NULL);
  r_res = emupalmos_trap_out(resctt);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetColortable(bitmapP=0x%08X): 0x%08X", bitmapP, r_res);
}
break;
case sysTrapBmpSize: {
  // UInt16 BmpSize(in BitmapType *bitmapP)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  res = (UInt32)BmpSize(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpSize(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpBitsSize: {
  // UInt16 BmpBitsSize(in BitmapType *bitmapP)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  res = (UInt32)BmpBitsSize(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpBitsSize(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpGetSizes: {
  // void BmpGetSizes(in BitmapType *bitmapP, out UInt32 *dataSizeP, out UInt32 *headerSizeP)
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
  // UInt16 BmpColortableSize(in BitmapType *bitmapP)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  res = (UInt32)BmpColortableSize(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpColortableSize(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpGetDimensions: {
  // void BmpGetDimensions(in BitmapType *bitmapP, out Coord *widthP, out Coord *heightP, out UInt16 *rowBytesP)
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
  // UInt8 BmpGetBitDepth(in BitmapType *bitmapP)
  bitmapP = ARG32;
  l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  res = BmpGetBitDepth(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, (UInt8)res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetBitDepth(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpGetNextBitmap: {
  // BitmapType *BmpGetNextBitmap(in BitmapType *bitmapP)
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
  // void FntCharsInWidth(in Char *string, inout Int16 *stringWidthP, inout Int16 *stringLengthP, out Boolean *fitWithinWidth)
  string = ARG32;
  s_string = emupalmos_trap_in(string, trap, 0);
  stringWidthP = ARG32;
  if (stringWidthP) l_stringWidthP = m68k_read_memory_16(stringWidthP);
  stringLengthP = ARG32;
  if (stringLengthP) l_stringLengthP = m68k_read_memory_16(stringLengthP);
  fitWithinWidth = ARG32;
  if (s_string) FntCharsInWidth(s_string, stringWidthP ? &l_stringWidthP : NULL, stringLengthP ? &l_stringLengthP : NULL, fitWithinWidth ? &l_fitWithinWidth : NULL);
  if (stringWidthP) m68k_write_memory_16(stringWidthP, l_stringWidthP);
  if (stringLengthP) m68k_write_memory_16(stringLengthP, l_stringLengthP);
  if (fitWithinWidth) m68k_write_memory_8(fitWithinWidth, l_fitWithinWidth);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntCharsInWidth(string=0x%08X [%s], stringWidthP=0x%08X [%d], stringLengthP=0x%08X [%d], fitWithinWidth=0x%08X [%d])", string, s_string, stringWidthP, l_stringWidthP, stringLengthP, l_stringLengthP, fitWithinWidth, l_fitWithinWidth);
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
  // Char *StrCopy(out Char *dst, in Char *src)
  dst = ARG32;
  s_dst = emupalmos_trap_in(dst, trap, 0);
  src = ARG32;
  s_src = emupalmos_trap_in(src, trap, 1);
  resc = NULL;
  if (s_dst && s_src) {
    if (emupalmos_check_address(dst, (uint32_t)(sys_strlen(s_src)+1), 0)) {
      debug(DEBUG_TRACE, "EmuPalmOS", "StrCopy %d bytes", (int)sys_strlen(s_src)+1);
      resc = StrCopy(s_dst, s_src);
    }
  }
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrCopy(dst=0x%08X [%s], src=0x%08X [%s]): 0x%08X", dst, s_dst, src, s_src, r_res);
}
break;
case sysTrapStrNCopy: {
  // Char *StrNCopy(out Char *dst, in Char *src, Int16 n)
  dst = ARG32;
  s_dst = emupalmos_trap_in(dst, trap, 0);
  src = ARG32;
  s_src = emupalmos_trap_in(src, trap, 1);
  n = ARG16;
  resc = s_dst && s_src ? StrNCopy(s_dst, s_src, (int16_t)n) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrNCopy(dst=0x%08X [%s], src=0x%08X [%s], n=%d): 0x%08X", dst, s_dst, src, s_src, n, r_res);
}
break;
case sysTrapStrCat: {
  // Char *StrCat(out Char *dst, in Char *src)
  dst = ARG32;
  s_dst = emupalmos_trap_in(dst, trap, 0);
  src = ARG32;
  s_src = emupalmos_trap_in(src, trap, 1);
  resc = NULL;
  if (s_dst && s_src) {
    if (emupalmos_check_address((uint32_t)(dst + sys_strlen(s_dst)), (uint32_t)(sys_strlen(s_src)+1), 0)) {
      debug(DEBUG_TRACE, "EmuPalmOS", "StrCat %d bytes into %d bytes with %d total", (int)sys_strlen(s_src)+1, (int)sys_strlen(s_dst), (int)sys_strlen(s_dst) + (int)sys_strlen(s_src)+1);
      resc = StrCat(s_dst, s_src);
    }
  }
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrCat(dst=0x%08X [%s], src=0x%08X [%s]): 0x%08X", dst, s_dst, src, s_src, r_res);
}
break;
case sysTrapStrNCat: {
  // Char *StrNCat(out Char *dst, in Char *src, Int16 n)
  dst = ARG32;
  s_dst = emupalmos_trap_in(dst, trap, 0);
  src = ARG32;
  s_src = emupalmos_trap_in(src, trap, 1);
  n = ARG16;
  resc = s_dst && s_src ? StrNCat(s_dst, s_src, (int16_t)n) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrNCat(dst=0x%08X [%s], src=0x%08X [%s], n=%d): 0x%08X", dst, s_dst, src, s_src, n, r_res);
}
break;
case sysTrapStrLen: {
  // UInt16 StrLen(in Char *src)
  src = ARG32;
  s_src = emupalmos_trap_in(src, trap, 0);
  res = s_src ? StrLen(s_src) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrLen(src=0x%08X [%s]): %d", src, s_src, res);
}
break;
case sysTrapStrCompareAscii: {
  // Int16 StrCompareAscii(in Char *s1, in Char *s2)
  s1 = ARG32;
  s_s1 = emupalmos_trap_in(s1, trap, 0);
  s2 = ARG32;
  s_s2 = emupalmos_trap_in(s2, trap, 1);
  res = s_s1 && s_s2 ? StrCompareAscii(s_s1, s_s2) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrCompareAscii(s1=0x%08X [%s], s2=0x%08X [%s]): %d", s1, s_s1, s2, s_s2, res);
}
break;
case sysTrapStrCompare: {
  // Int16 StrCompare(in Char *s1, in Char *s2)
  s1 = ARG32;
  s_s1 = emupalmos_trap_in(s1, trap, 0);
  s2 = ARG32;
  s_s2 = emupalmos_trap_in(s2, trap, 1);
  res = s_s1 && s_s2 ? StrCompare(s_s1, s_s2) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrCompare(s1=0x%08X [%s], s2=0x%08X [%s]): %d", s1, s_s1, s2, s_s2, res);
}
break;
case sysTrapStrNCompareAscii: {
  // Int16 StrNCompareAscii(in Char *s1, in Char *s2, Int32 n)
  s1 = ARG32;
  s_s1 = emupalmos_trap_in(s1, trap, 0);
  s2 = ARG32;
  s_s2 = emupalmos_trap_in(s2, trap, 1);
  n = ARG32;
  res = s_s1 && s_s2 ? StrNCompareAscii(s_s1, s_s2, n) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrNCompareAscii(s1=0x%08X [%s], s2=0x%08X [%s], n=%d): %d", s1, s_s1, s2, s_s2, n, res);
}
break;
case sysTrapStrNCompare: {
  // Int16 StrNCompare(in Char *s1, in Char *s2, Int32 n)
  s1 = ARG32;
  s_s1 = emupalmos_trap_in(s1, trap, 0);
  s2 = ARG32;
  s_s2 = emupalmos_trap_in(s2, trap, 1);
  n = ARG32;
  res = s_s1 && s_s2 ? StrNCompare(s_s1, s_s2, n) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrNCompare(s1=0x%08X [%s], s2=0x%08X [%s], n=%d): %d", s1, s_s1, s2, s_s2, n, res);
}
break;
case sysTrapStrCaselessCompare: {
  // Int16 StrCaselessCompare(in Char *s1, in Char *s2)
  s1 = ARG32;
  s_s1 = emupalmos_trap_in(s1, trap, 0);
  s2 = ARG32;
  s_s2 = emupalmos_trap_in(s2, trap, 1);
  res = s_s1 && s_s2 ? StrCaselessCompare(s_s1, s_s2) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrCaselessCompare(s1=0x%08X [%s], s2=0x%08X [%s]): %d", s1, s_s1, s2, s_s2, res);
}
break;
case sysTrapStrNCaselessCompare: {
  // Int16 StrNCaselessCompare(in Char *s1, in Char *s2, Int32 n)
  s1 = ARG32;
  s_s1 = emupalmos_trap_in(s1, trap, 0);
  s2 = ARG32;
  s_s2 = emupalmos_trap_in(s2, trap, 1);
  n = ARG32;
  res = s_s1 && s_s2 ?StrNCaselessCompare(s_s1, s_s2, n) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrNCaselessCompare(s1=0x%08X [%s], s2=0x%08X [%s], n=%d): %d", s1, s_s1, s2, s_s2, n, res);
}
break;
case sysTrapStrToLower: {
  // Char *StrToLower(out Char *dst, in Char *src)
  dst = ARG32;
  s_dst = emupalmos_trap_in(dst, trap, 0);
  src = ARG32;
  s_src = emupalmos_trap_in(src, trap, 1);
  resc = s_dst && s_src ? StrToLower(s_dst, s_src) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrToLower(dst=0x%08X [%s], src=0x%08X [%s]): 0x%08X", dst, s_dst, src, s_src, r_res);
}
break;
case sysTrapStrIToA: {
  // Char *StrIToA(out Char *s, Int32 i)
  s = ARG32;
  s_s = emupalmos_trap_in(s, trap, 0);
  i = ARG32;
  resc = s_s ? StrIToA(s_s, i) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrIToA(s=0x%08X [%s], i=%d): 0x%08X", s, s_s, i, r_res);
}
break;
case sysTrapStrIToH: {
  // Char *StrIToH(out Char *s, UInt32 i)
  s = ARG32;
  s_s = emupalmos_trap_in(s, trap, 0);
  i = ARG32;
  resc = s_s ? StrIToH(s_s, i) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrIToH(s=0x%08X [%s], i=%d): 0x%08X", s, s_s, i, r_res);
}
break;
case sysTrapStrLocalizeNumber: {
  // Char *StrLocalizeNumber(out Char *s, Char thousandSeparator, Char decimalSeparator)
  s = ARG32;
  s_s = emupalmos_trap_in(s, trap, 0);
  thousandSeparator = ARG8;
  decimalSeparator = ARG8;
  resc = s_s ? StrLocalizeNumber(s_s, thousandSeparator, decimalSeparator) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrLocalizeNumber(s=0x%08X [%s], thousandSeparator=%d, decimalSeparator=%d): 0x%08X", s, s_s, thousandSeparator, decimalSeparator, r_res);
}
break;
case sysTrapStrDelocalizeNumber: {
  // Char *StrDelocalizeNumber(out Char *s, Char thousandSeparator, Char decimalSeparator)
  s = ARG32;
  s_s = emupalmos_trap_in(s, trap, 0);
  thousandSeparator = ARG8;
  decimalSeparator = ARG8;
  resc = s_s ? StrDelocalizeNumber(s_s, thousandSeparator, decimalSeparator) : NULL;
  r_res = emupalmos_trap_out(resc);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrDelocalizeNumber(s=0x%08X [%s], thousandSeparator=%d, decimalSeparator=%d): 0x%08X", s, s_s, thousandSeparator, decimalSeparator, r_res);
}
break;
case sysTrapStrChr: {
  // Char *StrChr(in Char *str, WChar chr)
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
  // Char *StrStr(in Char *str, in Char *token)
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
  // Int32 StrAToI(in Char *str)
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
  rect = ARG32;
  FldGetBounds(fldP ? s_fldP : NULL, rect ? &l_rect : NULL);
  encode_rectangle(rect, &l_rect);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetBounds(fldP=0x%08X, rect=0x%08X [%d,%d,%d,%d])", fldP, rect, l_rect.topLeft.x, l_rect.topLeft.y, l_rect.extent.x, l_rect.extent.y);
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
  // void FldSetTextPtr(in fldP, in Char *textP)
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
  fld = FldNewField(&form, id, (uint16_t)x, (uint16_t)y, width, height,
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
  // void LstEraseList(in ListType *listP)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  LstEraseList(listP ? s_listP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstEraseList(listP=0x%08X)", listP);
}
break;
case sysTrapLstGetSelection: {
  // Int16 LstGetSelection(in ListType *listP)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  res = LstGetSelection(listP ? s_listP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstGetSelection(listP=0x%08X): %d", listP, res);
}
break;
case sysTrapLstGetSelectionText: {
  // Char *LstGetSelectionText(in ListType *listP, Int16 itemNum)
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
  // Boolean LstHandleEvent(in ListType *listP, in EventType *eventP)
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
  // void LstSetHeight(in ListType *listP, Int16 visibleItems)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  visibleItems = ARG16;
  LstSetHeight(listP ? s_listP : NULL, visibleItems);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstSetHeight(listP=0x%08X, visibleItems=%d)", listP, visibleItems);
}
break;
case sysTrapLstSetPosition: {
  // void LstSetPosition(in ListType *listP, Coord x, Coord y)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  x = ARG16;
  y = ARG16;
  LstSetPosition(listP ? s_listP : NULL, (uint16_t)x, (uint16_t)y);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstSetPosition(listP=0x%08X, x=%d, y=%d)", listP, x, y);
}
break;
case sysTrapLstSetSelection: {
  // void LstSetSelection(in ListType *listP, Int16 itemNum)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  itemNum = ARG16;
  LstSetSelection(listP ? s_listP : NULL, itemNum);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstSetSelection(listP=0x%08X, itemNum=%d)", listP, itemNum);
}
break;
case sysTrapLstSetListChoices: {
  // void LstSetListChoices(in ListType *listP, in Char **itemsText, Int16 numItems)
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
  // void LstSetTopItem(in ListType *listP, Int16 itemNum)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  itemNum = ARG16;
  LstSetTopItem(listP ? s_listP : NULL, itemNum);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstSetTopItem(listP=0x%08X, itemNum=%d)", listP, itemNum);
}
break;
case sysTrapLstMakeItemVisible: {
  // void LstMakeItemVisible(in ListType *listP, Int16 itemNum)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  itemNum = ARG16;
  LstMakeItemVisible(listP ? s_listP : NULL, itemNum);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstMakeItemVisible(listP=0x%08X, itemNum=%d)", listP, itemNum);
}
break;
case sysTrapLstGetNumberOfItems: {
  // Int16 LstGetNumberOfItems(in ListType *listP)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  res = LstGetNumberOfItems(listP ? s_listP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstGetNumberOfItems(listP=0x%08X): %d", listP, res);
}
break;
case sysTrapLstPopupList: {
  // Int16 LstPopupList(in ListType *listP)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  res = LstPopupList(listP ? s_listP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstPopupList(listP=0x%08X): %d", listP, res);
}
break;
case sysTrapLstScrollList: {
  // Boolean LstScrollList(in ListType *listP, WinDirectionType direction, Int16 itemCount)
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
  // Int16 LstGetVisibleItems(in ListType *listP)
  listP = ARG32;
  s_listP = emupalmos_trap_in(listP, trap, 0);
  res = LstGetVisibleItems(listP ? s_listP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstGetVisibleItems(listP=0x%08X): %d", listP, res);
}
break;
case sysTrapLstGetTopItem: {
  // Int16 LstGetTopItem(in ListType *listP)
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
  // Err MenuAddItem(UInt16 positionId, UInt16 id, Char cmd, in Char *textP)
  positionId = ARG16;
  id = ARG16;
  cmd = ARG8;
  textP = ARG32;
  s_textP = textP ? (char *)(ram + textP) : NULL;
  res = MenuAddItem(positionId, id, (uint8_t)cmd, textP ? s_textP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuAddItem(positionId=%d, id=%d, cmd=%d, textP=0x%08X [%s]): %d", positionId, id, cmd, textP, s_textP, res);
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
  uint8_t enableIt = ARG8;
  InsPtEnable(enableIt);
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
  // void CtlDrawControl(in ControlType *controlP)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  CtlDrawControl(controlP ? s_controlP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlDrawControl(controlP=0x%08X)", controlP);
}
break;
case sysTrapCtlEraseControl: {
  // void CtlEraseControl(in ControlType *controlP)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  CtlEraseControl(controlP ? s_controlP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlEraseControl(controlP=0x%08X)", controlP);
}
break;
case sysTrapCtlHideControl: {
  // void CtlHideControl(in ControlType *controlP)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  CtlHideControl(controlP ? s_controlP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlHideControl(controlP=0x%08X)", controlP);
}
break;
case sysTrapCtlShowControl: {
  // void CtlShowControl(in ControlType *controlP)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  CtlShowControl(controlP ? s_controlP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlShowControl(controlP=0x%08X)", controlP);
}
break;
case sysTrapCtlEnabled: {
  // Boolean CtlEnabled(in ControlType *controlP)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  res = (UInt32)CtlEnabled(controlP ? s_controlP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlEnabled(controlP=0x%08X): %d", controlP, res);
}
break;
case sysTrapCtlSetEnabled: {
  // void CtlSetEnabled(in ControlType *controlP, Boolean usable)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  usable = ARG8;
  CtlSetEnabled(controlP ? s_controlP : NULL, usable);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetEnabled(controlP=0x%08X, usable=%d)", controlP, usable);
}
break;
case sysTrapCtlSetUsable: {
  // void CtlSetUsable(in ControlType *controlP, Boolean usable)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  usable = ARG8;
  CtlSetUsable(controlP ? s_controlP : NULL, usable);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetUsable(controlP=0x%08X, usable=%d)", controlP, usable);
}
break;
case sysTrapCtlGetValue: {
  // Int16 CtlGetValue(in ControlType *controlP)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  res = CtlGetValue(controlP ? s_controlP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlGetValue(controlP=0x%08X): %d", controlP, res);
}
break;
case sysTrapCtlSetValue: {
  // void CtlSetValue(in ControlType *controlP, Int16 newValue)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  newValue = ARG16;
  CtlSetValue(controlP ? s_controlP : NULL, newValue);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetValue(controlP=0x%08X, newValue=%d)", controlP, newValue);
}
break;
case sysTrapCtlSetLabel: {
  // void CtlSetLabel(in ControlType *controlP, in Char *newLabel)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  newLabel = ARG32;
  s_newLabel = emupalmos_trap_in(newLabel, trap, 1);
  CtlSetLabel(controlP ? s_controlP : NULL, newLabel ? s_newLabel : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetLabel(controlP=0x%08X, newLabel=0x%08X [%s])", controlP, newLabel, s_newLabel);
}
break;
case sysTrapCtlSetGraphics: {
  // void CtlSetGraphics(in ControlType *ctlP, DmResID newBitmapID, DmResID newSelectedBitmapID)
  ctlP = ARG32;
  s_ctlP = ctlP ? (ControlType *)(ram + ctlP) : NULL;
  newBitmapID = ARG16;
  newSelectedBitmapID = ARG16;
  CtlSetGraphics(ctlP ? s_ctlP : NULL, newBitmapID, newSelectedBitmapID);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetGraphics(ctlP=0x%08X, newBitmapID=%d, newSelectedBitmapID=%d)", ctlP, newBitmapID, newSelectedBitmapID);
}
break;
case sysTrapCtlSetSliderValues: {
  // void CtlSetSliderValues(in ControlType *ctlP, in UInt16 *minValueP, in UInt16 *maxValueP, in UInt16 *pageSizeP, in UInt16 *valueP)
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
  // void CtlGetSliderValues(in ControlType *ctlP, out UInt16 *minValueP, out UInt16 *maxValueP, out UInt16 *pageSizeP, out UInt16 *valueP)
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
  // void CtlHitControl(in ControlType *controlP)
  controlP = ARG32;
  s_controlP = emupalmos_trap_in(controlP, trap, 0);
  CtlHitControl(controlP ? s_controlP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlHitControl(controlP=0x%08X)", controlP);
}
break;
case sysTrapCtlHandleEvent: {
  // Boolean CtlHandleEvent(in ControlType *controlP, in EventType *pEvent)
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
  // Boolean CtlValidatePointer(in ControlType *controlP)
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
  // Char *SysGetOSVersionString()
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
  // void CategoryCreateList(DmOpenRef db, in ListType *listP, UInt16 currentCategory, Boolean showAll, Boolean showUneditables, UInt8 numUneditableCategories, UInt32 editingStrID, Boolean resizeList)
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
  // void CategoryFreeList(DmOpenRef db, in ListType *listP, Boolean showAll, UInt32 editingStrID)
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
  name = ARG32;
  s_name = name ? (char *)(ram + name) : NULL;
  res = (UInt32)CategoryFind(db ? l_db : 0, name ? s_name : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryFind(db=0x%08X, name=0x%08X [%s]): %d", db, name, s_name, res);
}
break;
case sysTrapCategoryGetName: {
  // void CategoryGetName(DmOpenRef db, UInt16 index, out Char *name)
  db = ARG32;
  l_db = db ? (DmOpenRef)(ram + db) : NULL;
  index = ARG16;
  name = ARG32;
  s_name = name ? (char *)(ram + name) : NULL;
  CategoryGetName(db ? l_db : 0, index, name ? s_name : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryGetName(db=0x%08X, index=%d, name=0x%08X [%s])", db, index, name, s_name);
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
  title = ARG8;
  categoryP = ARG32;
  categoryName = ARG32;
  s_categoryName = categoryName ? (char *)(ram + categoryName) : NULL;
  res = (UInt32)CategorySelectV10(db ? l_db : 0, frm ? s_frm : NULL, ctlID, lstID, title, categoryP ? &l_categoryP : NULL, categoryName ? s_categoryName : NULL);
  if (categoryP) m68k_write_memory_16(categoryP, l_categoryP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategorySelectV10(db=0x%08X, frm=0x%08X, ctlID=%d, lstID=%d, title=%d, categoryP=0x%08X [%d], categoryName=0x%08X [%s]): %d", db, frm, ctlID, lstID, title, categoryP, l_categoryP, categoryName, s_categoryName, res);
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
  title = ARG8;
  categoryP = ARG32;
  categoryName = ARG32;
  s_categoryName = categoryName ? (char *)(ram + categoryName) : NULL;
  numUneditableCategories = ARG8;
  editingStrID = ARG32;
  res = (UInt32)CategorySelect(db ? l_db : 0, frm ? s_frm : NULL, ctlID, lstID, title, categoryP ? &l_categoryP : NULL, categoryName ? s_categoryName : NULL, numUneditableCategories, editingStrID);
  if (categoryP) m68k_write_memory_16(categoryP, l_categoryP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategorySelect(db=0x%08X, frm=0x%08X, ctlID=%d, lstID=%d, title=%d, categoryP=0x%08X [%d], categoryName=0x%08X [%s], numUneditableCategories=%d, editingStrID=%d): %d", db, frm, ctlID, lstID, title, categoryP, l_categoryP, categoryName, s_categoryName, numUneditableCategories, editingStrID, res);
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
  // void CategorySetTriggerLabel(in ControlType *ctl, Char *name)
  ctl = ARG32;
  s_ctl = ctl ? (ControlType *)(ram + ctl) : NULL;
  name = ARG32;
  s_name = name ? (char *)(ram + name) : NULL;
  CategorySetTriggerLabel(ctl ? s_ctl : NULL, name ? s_name : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategorySetTriggerLabel(ctl=0x%08X, name=0x%08X [%s])", ctl, name, s_name);
}
break;
case sysTrapCategoryTruncateName: {
  // void CategoryTruncateName(inout Char *name, UInt16 maxWidth)
  name = ARG32;
  s_name = name ? (char *)(ram + name) : NULL;
  maxWidth = ARG16;
  CategoryTruncateName(name ? s_name : NULL, maxWidth);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryTruncateName(name=0x%08X [%s], maxWidth=%d)", name, s_name, maxWidth);
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
