#include <PalmOS.h>
#include <VFSMgr.h>
    
#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "pumpkin.h"
#include "vfsimport.h"
#include "logtrap.h"
#include "m68k.h"
#include "m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"
    
typedef struct {
  FileRef ref;
} FileRefProxy;

void palmos_filesystemtrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];
  uint16_t volRefNum = 0;
  uint32_t pathNameP = 0;
  char *s_pathNameP = NULL;
  uint16_t openMode = 0;
  uint32_t fileRefP = 0;
  FileRef l_fileRefP = 0;
  Err res = 0;
  FileRefProxy *proxy = NULL;
  FileRefProxy *ll_fileRef = NULL;
  FileRef l_fileRef = NULL;
  uint32_t numBytes = 0;
  uint32_t bufBaseP = 0;
  void *s_bufBaseP = NULL;
  uint32_t offset = 0;
  uint32_t numBytesReadP = 0;
  UInt32 l_numBytesReadP = 0;
  uint32_t bufP = 0;
  void *s_bufP = NULL;
  uint32_t dataP = 0;
  void *s_dataP = NULL;
  uint32_t numBytesWrittenP = 0;
  UInt32 l_numBytesWrittenP = 0;
  uint32_t newNameP = 0;
  char *s_newNameP = NULL;
  uint16_t origin = 0;
  uint32_t filePosP = 0;
  UInt32 l_filePosP = 0;
  uint32_t fileSizeP = 0;
  UInt32 l_fileSizeP = 0;
  uint32_t newSize = 0;
  uint32_t attributesP = 0;
  UInt32 l_attributesP = 0;
  uint32_t attributes = 0;
  uint16_t whichDate = 0;
  uint32_t dateP = 0;
  UInt32 l_dateP = 0;
  uint32_t date = 0;
  uint32_t dirNameP = 0;
  char *s_dirNameP = NULL;
  uint32_t dirRef = 0;
  FileRefProxy *ll_dirRef = NULL;
  FileRef l_dirRef = NULL;
  uint32_t dirEntryIteratorP = 0;
  UInt32 l_dirEntryIteratorP = 0;
  uint32_t infoP = 0;
  FileInfoType l_infoP;
  uint32_t fileTypeStr = 0;
  char *s_fileTypeStr = NULL;
  uint32_t pathStr = 0;
  char *s_pathStr = NULL;
  uint32_t bufLenP = 0;
  UInt16 l_bufLenP = 0;
  uint32_t mediaType = 0;
  uint32_t volRefNumP = 0;
  UInt16 l_volRefNumP = 0;
  uint32_t volIteratorP = 0;
  UInt32 l_volIteratorP = 0;
  uint32_t volInfoP = 0;
  VolumeInfoType l_volInfoP;
  uint32_t labelP = 0;
  char *s_labelP = NULL;
  uint16_t bufLen = 0;
  uint32_t volumeUsedP = 0;
  UInt32 l_volumeUsedP = 0;
  uint32_t volumeTotalP = 0;
  UInt32 l_volumeTotalP = 0;
  uint32_t creator = 0;
  uint32_t fsLibRefNumP = 0;
  UInt16 l_fsLibRefNumP = 0;
  uint16_t fsLibRefNum = 0;
  uint32_t refP = 0;
  uint32_t type = 0;
  uint16_t resID = 0;
  uint32_t resHP = 0;
  FileRefProxy *ref = NULL;
  FileRef fileRef =  NULL;
  MemHandle resH;
  uint32_t nameP = 0;
  uint32_t versionP = 0;
  uint32_t crDateP = 0;
  uint32_t modDateP = 0;
  uint32_t bckUpDateP = 0;
  uint32_t modNumP = 0;
  uint32_t appInfoHP = 0;
  uint32_t sortInfoHP = 0;
  uint32_t typeP = 0;
  uint32_t creatorP = 0;
  uint32_t numRecordsP = 0;
  char *name = NULL;
  UInt16 version, numRecords;
  UInt32 crDate, modDate, bckUpDate, modNum;
  MemHandle appInfoH, sortInfoH;
  uint32_t cardNoP = 0;
  uint32_t dbIDP = 0;
  char *pathName = NULL;
  UInt16 cardNo = 0;
  LocalID dbID = 0;
  uint32_t importProcP = 0;
  uint32_t userDataP = 0;
  uint32_t exportProcP = 0;

  switch (sel) {
    case vfsTrapFileOpen: {
      // Err VFSFileOpen(UInt16 volRefNum, in Char *pathNameP, UInt16 openMode, FileRef *fileRefP);
      volRefNum = ARG16;
      pathNameP = ARG32;
      s_pathNameP = emupalmos_trap_sel_in(pathNameP, sysTrapFileSystemDispatch, sel, 0);
      openMode = ARG16;
      fileRefP = ARG32;
      l_fileRefP = 0;
      res = VFSFileOpen(volRefNum, pathNameP ? s_pathNameP : NULL, openMode, fileRefP ? &l_fileRefP : NULL);
      if (fileRefP) {
        proxy = pumpkin_heap_alloc(sizeof(FileRefProxy), "FileProxy");
        if (proxy) {
          proxy->ref = l_fileRefP;
          m68k_write_memory_32(fileRefP, emupalmos_trap_out(proxy));
        }
      }
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileOpen(volRefNum=%d, pathNameP=0x%08X [%s], openMode=0x%04X, fileRefP=0x%08X): %d",
        volRefNum, pathNameP, s_pathNameP ? s_pathNameP : "", openMode, fileRefP, res);
      }
      break;
    case vfsTrapFileClose: {
      // Err VFSFileClose(FileRef fileRef);
      fileRefP = ARG32;
      ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRefP, sysTrapFileSystemDispatch, sel, 0);
      l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      res = VFSFileClose(fileRef ? l_fileRef : 0);
      if (ll_fileRef) pumpkin_heap_free(ll_fileRef, "FileProxy");
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileClose(fileRef=%d): %d", fileRef, res);
      }
      break;
    case vfsTrapInit: {
      // Err VFSInit(void);
      res = VFSInit();
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSInit(): %d", res);
    }
    break;
    case vfsTrapFileCreate: {
      // Err VFSFileCreate(UInt16 volRefNum, in Char *pathNameP);
      volRefNum = ARG16;
      pathNameP = ARG32;
      s_pathNameP = emupalmos_trap_sel_in(pathNameP, sysTrapFileSystemDispatch, sel, 1);
      res = VFSFileCreate(volRefNum, pathNameP ? s_pathNameP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileCreate(volRefNum=%d, pathNameP=0x%08X [%s]): %d", volRefNum, pathNameP, s_pathNameP, res);
    }
    break;
    case vfsTrapFileReadData: {
      // Err VFSFileReadData(FileRef fileRef, UInt32 numBytes, out void *bufBaseP, UInt32 offset, out UInt32 *numBytesReadP);
      fileRefP = ARG32;
      ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRefP, sysTrapFileSystemDispatch, sel, 0);
      l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      numBytes = ARG32;
      bufBaseP = ARG32;
      s_bufBaseP = emupalmos_trap_sel_in(bufBaseP, sysTrapFileSystemDispatch, sel, 0);
      offset = ARG32;
      numBytesReadP = ARG32;
      l_numBytesReadP = 0;
      res = VFSFileReadData(fileRef ? l_fileRef : 0, numBytes, bufBaseP ? s_bufBaseP : NULL, offset, numBytesReadP ? &l_numBytesReadP : NULL);
      if (numBytesReadP) m68k_write_memory_32(numBytesReadP, l_numBytesReadP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileReadData(fileRef=%d, numBytes=%d, bufBaseP=0x%08X, offset=%d, numBytesReadP=0x%08X [%d]): %d", fileRef, numBytes, bufBaseP, offset, numBytesReadP, l_numBytesReadP, res);
    }
    break;
    case vfsTrapFileRead: {
      // Err VFSFileRead(FileRef fileRef, UInt32 numBytes, out void *bufP, out UInt32 *numBytesReadP);
      fileRefP = ARG32;
      ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRefP, sysTrapFileSystemDispatch, sel, 0);
      l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      numBytes = ARG32;
      bufP = ARG32;
      s_bufP = emupalmos_trap_sel_in(bufP, sysTrapFileSystemDispatch, sel, 2);
      numBytesReadP = ARG32;
      l_numBytesReadP = 0;
      res = VFSFileRead(fileRef ? l_fileRef : 0, numBytes, bufP ? s_bufP : NULL, numBytesReadP ? &l_numBytesReadP : NULL);
      if (numBytesReadP) m68k_write_memory_32(numBytesReadP, l_numBytesReadP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileRead(fileRef=%d, numBytes=%d, bufP=0x%08X, numBytesReadP=0x%08X [%d]): %d", fileRef, numBytes, bufP, numBytesReadP, l_numBytesReadP, res);
    }
    break;
    case vfsTrapFileWrite: {
      // Err VFSFileWrite(FileRef fileRef, UInt32 numBytes, in void *dataP, out UInt32 *numBytesWrittenP);
      fileRefP = ARG32;
      ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRefP, sysTrapFileSystemDispatch, sel, 0);
      l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      numBytes = ARG32;
      dataP = ARG32;
      s_dataP = emupalmos_trap_sel_in(dataP, sysTrapFileSystemDispatch, sel, 2);
      numBytesWrittenP = ARG32;
      l_numBytesWrittenP = 0;
      res = VFSFileWrite(fileRef ? l_fileRef : 0, numBytes, dataP ? s_dataP : NULL, numBytesWrittenP ? &l_numBytesWrittenP : NULL);
      if (numBytesWrittenP) m68k_write_memory_32(numBytesWrittenP, l_numBytesWrittenP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileWrite(fileRef=%d, numBytes=%d, dataP=0x%08X, numBytesWrittenP=0x%08X [%d]): %d", fileRef, numBytes, dataP, numBytesWrittenP, l_numBytesWrittenP, res);
    }
    break;
    case vfsTrapFileDelete: {
      // Err VFSFileDelete(UInt16 volRefNum, in Char *pathNameP);
      volRefNum = ARG16;
      pathNameP = ARG32;
      s_pathNameP = emupalmos_trap_sel_in(pathNameP, sysTrapFileSystemDispatch, sel, 1);
      res = VFSFileDelete(volRefNum, pathNameP ? s_pathNameP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileDelete(volRefNum=%d, pathNameP=0x%08X [%s]): %d", volRefNum, pathNameP, s_pathNameP, res);
    }
    break;
    case vfsTrapFileRename: {
      // Err VFSFileRename(UInt16 volRefNum, in Char *pathNameP, in Char *newNameP);
      volRefNum = ARG16;
      pathNameP = ARG32;
      s_pathNameP = emupalmos_trap_sel_in(pathNameP, sysTrapFileSystemDispatch, sel, 1);
      newNameP = ARG32;
      s_newNameP = emupalmos_trap_sel_in(newNameP, sysTrapFileSystemDispatch, sel, 2);
      res = VFSFileRename(volRefNum, pathNameP ? s_pathNameP : NULL, newNameP ? s_newNameP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileRename(volRefNum=%d, pathNameP=0x%08X [%s], newNameP=0x%08X [%s]): %d", volRefNum, pathNameP, s_pathNameP, newNameP, s_newNameP, res);
    }
    break;
    case vfsTrapFileSeek: {
      // Err VFSFileSeek(FileRef fileRef, FileOrigin origin, Int32 offset);
      fileRefP = ARG32;
      ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRefP, sysTrapFileSystemDispatch, sel, 0);
      l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      origin = ARG16;
      offset = ARG32;
      res = VFSFileSeek(fileRef ? l_fileRef : 0, origin, (int32_t)offset);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileSeek(fileRef=%d, origin=%d, offset=%d): %d", fileRef, origin, offset, res);
    }
    break;
    case vfsTrapFileEOF: {
      // Err VFSFileEOF(FileRef fileRef);
      fileRefP = ARG32;
      ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRefP, sysTrapFileSystemDispatch, sel, 0);
      l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      res = VFSFileEOF(fileRef ? l_fileRef : 0);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileEOF(fileRef=%d): %d", fileRef, res);
    }
    break;
    case vfsTrapFileTell: {
      // Err VFSFileTell(FileRef fileRef, out UInt32 *filePosP);
      fileRefP = ARG32;
      ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRefP, sysTrapFileSystemDispatch, sel, 0);
      l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      filePosP = ARG32;
      l_filePosP = 0;
      res = VFSFileTell(fileRef ? l_fileRef : 0, filePosP ? &l_filePosP : NULL);
      if (filePosP) m68k_write_memory_32(filePosP, l_filePosP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileTell(fileRef=%d, filePosP=0x%08X [%d]): %d", fileRef, filePosP, l_filePosP, res);
    }
    break;
    case vfsTrapFileSize: {
      // Err VFSFileSize(FileRef fileRef, out UInt32 *fileSizeP);
      fileRefP = ARG32;
      ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRefP, sysTrapFileSystemDispatch, sel, 0);
      l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      fileSizeP = ARG32;
      l_fileSizeP = 0;
      res = VFSFileSize(fileRef ? l_fileRef : 0, fileSizeP ? &l_fileSizeP : NULL);
      if (fileSizeP) m68k_write_memory_32(fileSizeP, l_fileSizeP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileSize(fileRef=%d, fileSizeP=0x%08X [%d]): %d", fileRef, fileSizeP, l_fileSizeP, res);
    }
    break;
    case vfsTrapFileResize: {
      // Err VFSFileResize(FileRef fileRef, UInt32 newSize);
      fileRefP = ARG32;
      ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRefP, sysTrapFileSystemDispatch, sel, 0);
      l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      newSize = ARG32;
      res = VFSFileResize(fileRef ? l_fileRef : 0, newSize);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileResize(fileRef=%d, newSize=%d): %d", fileRef, newSize, res);
    }
    break;
    case vfsTrapFileGetAttributes: {
      // Err VFSFileGetAttributes(FileRef fileRef, out UInt32 *attributesP);
      fileRefP = ARG32;
      ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRefP, sysTrapFileSystemDispatch, sel, 0);
      l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      attributesP = ARG32;
      l_attributesP = 0;
      res = VFSFileGetAttributes(fileRef ? l_fileRef : 0, attributesP ? &l_attributesP : NULL);
      if (attributesP) m68k_write_memory_32(attributesP, l_attributesP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileGetAttributes(fileRef=%d, attributesP=0x%08X [%d]): %d", fileRef, attributesP, l_attributesP, res);
    }
    break;
    case vfsTrapFileSetAttributes: {
      // Err VFSFileSetAttributes(FileRef fileRef, UInt32 attributes);
      fileRefP = ARG32;
      ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRefP, sysTrapFileSystemDispatch, sel, 0);
      l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      attributes = ARG32;
      res = VFSFileSetAttributes(fileRef ? l_fileRef : 0, attributes);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileSetAttributes(fileRef=%d, attributes=%d): %d", fileRef, attributes, res);
    }
    break;
    case vfsTrapFileGetDate: {
      // Err VFSFileGetDate(FileRef fileRef, UInt16 whichDate, out UInt32 *dateP);
      fileRefP = ARG32;
      ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRefP, sysTrapFileSystemDispatch, sel, 0);
      l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      whichDate = ARG16;
      dateP = ARG32;
      l_dateP = 0;
      res = VFSFileGetDate(fileRef ? l_fileRef : 0, whichDate, dateP ? &l_dateP : NULL);
      if (dateP) m68k_write_memory_32(dateP, l_dateP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileGetDate(fileRef=%d, whichDate=%d, dateP=0x%08X [%d]): %d", fileRef, whichDate, dateP, l_dateP, res);
    }
    break;
    case vfsTrapFileSetDate: {
      // Err VFSFileSetDate(FileRef fileRef, UInt16 whichDate, UInt32 date);
      fileRefP = ARG32;
      ll_fileRef = (FileRefProxy *)emupalmos_trap_sel_in(fileRefP, sysTrapFileSystemDispatch, sel, 0);
      l_fileRef = ll_fileRef ? ll_fileRef->ref : NULL;
      whichDate = ARG16;
      date = ARG32;
      res = VFSFileSetDate(fileRef ? l_fileRef : 0, whichDate, date);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileSetDate(fileRef=%d, whichDate=%d, date=%d): %d", fileRef, whichDate, date, res);
    }
    break;
    case vfsTrapDirCreate: {
      // Err VFSDirCreate(UInt16 volRefNum, in Char *dirNameP);
      volRefNum = ARG16;
      dirNameP = ARG32;
      s_dirNameP = emupalmos_trap_sel_in(dirNameP, sysTrapFileSystemDispatch, sel, 1);
      res = VFSDirCreate(volRefNum, dirNameP ? s_dirNameP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSDirCreate(volRefNum=%d, dirNameP=0x%08X [%s]): %d", volRefNum, dirNameP, s_dirNameP, res);
    }
    break;
    case vfsTrapDirEntryEnumerate: {
      // Err VFSDirEntryEnumerate(FileRef dirRef, inout UInt32 *dirEntryIteratorP, inout FileInfoType *infoP);
      dirRef = ARG32;
      ll_dirRef = (FileRefProxy *)emupalmos_trap_sel_in(dirRef, sysTrapFileSystemDispatch, sel, 0);
      l_dirRef = ll_dirRef ? ll_dirRef->ref : NULL;
      dirEntryIteratorP = ARG32;
      if (dirEntryIteratorP) l_dirEntryIteratorP = m68k_read_memory_32(dirEntryIteratorP);
      infoP = ARG32;
      decode_FileInfoType(infoP, &l_infoP);
      res = VFSDirEntryEnumerate(dirRef ? l_dirRef : 0, dirEntryIteratorP ? &l_dirEntryIteratorP : NULL, infoP ? &l_infoP : NULL);
      if (dirEntryIteratorP) m68k_write_memory_32(dirEntryIteratorP, l_dirEntryIteratorP);
      encode_FileInfoType(infoP, &l_infoP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSDirEntryEnumerate(dirRef=%d, dirEntryIteratorP=0x%08X [%d], infoP=0x%08X): %d", dirRef, dirEntryIteratorP, l_dirEntryIteratorP, infoP, res);
    }
    break;
    case vfsTrapGetDefaultDirectory: {
      // Err VFSGetDefaultDirectory(UInt16 volRefNum, in Char *fileTypeStr, out Char *pathStr, inout UInt16 *bufLenP);
      volRefNum = ARG16;
      fileTypeStr = ARG32;
      s_fileTypeStr = emupalmos_trap_sel_in(fileTypeStr, sysTrapFileSystemDispatch, sel, 1);
      pathStr = ARG32;
      s_pathStr = emupalmos_trap_sel_in(pathStr, sysTrapFileSystemDispatch, sel, 2);
      bufLenP = ARG32;
      if (bufLenP) l_bufLenP = m68k_read_memory_16(bufLenP);
      res = VFSGetDefaultDirectory(volRefNum, fileTypeStr ? s_fileTypeStr : NULL, pathStr ? s_pathStr : NULL, bufLenP ? &l_bufLenP : NULL);
      if (bufLenP) m68k_write_memory_16(bufLenP, l_bufLenP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSGetDefaultDirectory(volRefNum=%d, fileTypeStr=0x%08X [%s], pathStr=0x%08X [%s], bufLenP=0x%08X [%d]): %d", volRefNum, fileTypeStr, s_fileTypeStr, pathStr, s_pathStr, bufLenP, l_bufLenP, res);
    }
    break;
    case vfsTrapRegisterDefaultDirectory: {
      // Err VFSRegisterDefaultDirectory(in Char *fileTypeStr, UInt32 mediaType, in Char *pathStr);
      fileTypeStr = ARG32;
      s_fileTypeStr = emupalmos_trap_sel_in(fileTypeStr, sysTrapFileSystemDispatch, sel, 0);
      mediaType = ARG32;
      pathStr = ARG32;
      s_pathStr = emupalmos_trap_sel_in(pathStr, sysTrapFileSystemDispatch, sel, 2);
      res = VFSRegisterDefaultDirectory(fileTypeStr ? s_fileTypeStr : NULL, mediaType, pathStr ? s_pathStr : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSRegisterDefaultDirectory(fileTypeStr=0x%08X [%s], mediaType=%d, pathStr=0x%08X [%s]): %d", fileTypeStr, s_fileTypeStr, mediaType, pathStr, s_pathStr, res);
    }
    break;
    case vfsTrapUnregisterDefaultDirectory: {
      // Err VFSUnregisterDefaultDirectory(in Char *fileTypeStr, UInt32 mediaType);
      fileTypeStr = ARG32;
      s_fileTypeStr = emupalmos_trap_sel_in(fileTypeStr, sysTrapFileSystemDispatch, sel, 0);
      mediaType = ARG32;
      res = VFSUnregisterDefaultDirectory(fileTypeStr ? s_fileTypeStr : NULL, mediaType);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSUnregisterDefaultDirectory(fileTypeStr=0x%08X [%s], mediaType=%d): %d", fileTypeStr, s_fileTypeStr, mediaType, res);
    }
    break;
    case vfsTrapVolumeUnmount: {
      // Err VFSVolumeUnmount(UInt16 volRefNum);
      volRefNum = ARG16;
      res = VFSVolumeUnmount(volRefNum);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSVolumeUnmount(volRefNum=%d): %d", volRefNum, res);
    }
    break;
    case vfsTrapVolumeEnumerate: {
      // Err VFSVolumeEnumerate(out UInt16 *volRefNumP, inout UInt32 *volIteratorP);
      volRefNumP = ARG32;
      l_volRefNumP = 0;
      volIteratorP = ARG32;
      l_volIteratorP = 0;
      if (volIteratorP) l_volIteratorP = m68k_read_memory_32(volIteratorP);
      res = VFSVolumeEnumerate(volRefNumP ? &l_volRefNumP : NULL, volIteratorP ? &l_volIteratorP : NULL);
      if (volRefNumP) m68k_write_memory_16(volRefNumP, l_volRefNumP);
      if (volIteratorP) m68k_write_memory_32(volIteratorP, l_volIteratorP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSVolumeEnumerate(volRefNumP=0x%08X [%d], volIteratorP=0x%08X [%d]): %d", volRefNumP, l_volRefNumP, volIteratorP, l_volIteratorP, res);
    }
    break;
    case vfsTrapVolumeInfo: {
      // Err VFSVolumeInfo(UInt16 volRefNum, out VolumeInfoType *volInfoP);
      volRefNum = ARG16;
      volInfoP = ARG32;
      res = VFSVolumeInfo(volRefNum, volInfoP ? &l_volInfoP : NULL);
      encode_VolumeInfoType(volInfoP, &l_volInfoP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSVolumeInfo(volRefNum=%d, volInfoP=0x%08X): %d", volRefNum, volInfoP, res);
    }
    break;
    case vfsTrapVolumeGetLabel: {
      // Err VFSVolumeGetLabel(UInt16 volRefNum, out Char *labelP, UInt16 bufLen);
      volRefNum = ARG16;
      labelP = ARG32;
      s_labelP = emupalmos_trap_sel_in(labelP, sysTrapFileSystemDispatch, sel, 1);
      bufLen = ARG16;
      res = VFSVolumeGetLabel(volRefNum, labelP ? s_labelP : NULL, bufLen);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSVolumeGetLabel(volRefNum=%d, labelP=0x%08X [%s], bufLen=%d): %d", volRefNum, labelP, s_labelP, bufLen, res);
    }
    break;
    case vfsTrapVolumeSetLabel: {
      // Err VFSVolumeSetLabel(UInt16 volRefNum, in Char *labelP);
      volRefNum = ARG16;
      labelP = ARG32;
      s_labelP = emupalmos_trap_sel_in(labelP, sysTrapFileSystemDispatch, sel, 1);
      res = VFSVolumeSetLabel(volRefNum, labelP ? s_labelP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSVolumeSetLabel(volRefNum=%d, labelP=0x%08X [%s]): %d", volRefNum, labelP, s_labelP, res);
    }
    break;
    case vfsTrapVolumeSize: {
      // Err VFSVolumeSize(UInt16 volRefNum, out UInt32 *volumeUsedP, out UInt32 *volumeTotalP);
      volRefNum = ARG16;
      volumeUsedP = ARG32;
      l_volumeUsedP = 0;
      volumeTotalP = ARG32;
      l_volumeTotalP = 0;
      res = VFSVolumeSize(volRefNum, volumeUsedP ? &l_volumeUsedP : NULL, volumeTotalP ? &l_volumeTotalP : NULL);
      if (volumeUsedP) m68k_write_memory_32(volumeUsedP, l_volumeUsedP);
      if (volumeTotalP) m68k_write_memory_32(volumeTotalP, l_volumeTotalP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSVolumeSize(volRefNum=%d, volumeUsedP=0x%08X [%d], volumeTotalP=0x%08X [%d]): %d", volRefNum, volumeUsedP, l_volumeUsedP, volumeTotalP, l_volumeTotalP, res);
    }
    break;
    case vfsTrapInstallFSLib: {
      // Err VFSInstallFSLib(UInt32 creator, out UInt16 *fsLibRefNumP);
      creator = ARG32;
      fsLibRefNumP = ARG32;
      l_fsLibRefNumP = 0;
      res = VFSInstallFSLib(creator, fsLibRefNumP ? &l_fsLibRefNumP : NULL);
      if (fsLibRefNumP) m68k_write_memory_16(fsLibRefNumP, l_fsLibRefNumP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSInstallFSLib(creator=%d, fsLibRefNumP=0x%08X [%d]): %d", creator, fsLibRefNumP, l_fsLibRefNumP, res);
    }
    break;
    case vfsTrapRemoveFSLib: {
      // Err VFSRemoveFSLib(UInt16 fsLibRefNum);
      fsLibRefNum = ARG16;
      res = VFSRemoveFSLib(fsLibRefNum);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSRemoveFSLib(fsLibRefNum=%d): %d", fsLibRefNum, res);
    }
    break;
    case vfsTrapFileDBGetResource: {
      // Err VFSFileDBGetResource(FileRef ref, DmResType type, DmResID resID, MemHandle *resHP)
      refP = ARG32;
      type = ARG32;
      resID = ARG16;
      resHP = ARG32;
      ref = (FileRefProxy *)emupalmos_trap_sel_in(refP, sysTrapFileSystemDispatch, sel, 0);
      fileRef = ref ? ref->ref : NULL;
      emupalmos_trap_sel_in(resHP, sysTrapFileSystemDispatch, sel, 3);
      res = VFSFileDBGetResource(fileRef, type, resID, resHP ? &resH : NULL);
      if (resHP) m68k_write_memory_32(resHP, emupalmos_trap_out(resH));
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileDBGetResource(fileRef=%d): %d", refP, res);
    }
    break;
    case vfsTrapFileDBInfo: {
      // Err VFSFileDBInfo(FileRef ref, Char *nameP,
      //    UInt16 *attributesP, UInt16 *versionP, UInt32 *crDateP,
      //    UInt32 *modDateP, UInt32 *bckUpDateP,
      //    UInt32 *modNumP, MemHandle *appInfoHP,
      //    MemHandle *sortInfoHP, UInt32 *typeP,
      //    UInt32 *creatorP, UInt16 *numRecordsP)
      refP = ARG32;
      nameP = ARG32;
      attributesP = ARG32;
      versionP = ARG32;
      crDateP = ARG32;
      modDateP = ARG32;
      bckUpDateP = ARG32;
      modNumP = ARG32;
      appInfoHP = ARG32;
      sortInfoHP = ARG32;
      typeP = ARG32;
      creatorP = ARG32;
      numRecordsP = ARG32;
      ref = (FileRefProxy *)emupalmos_trap_sel_in(refP, sysTrapFileSystemDispatch, sel, 0);
      fileRef = ref ? ref->ref : NULL;
      name = emupalmos_trap_sel_in(nameP, sysTrapFileSystemDispatch, sel, 1);
      res = VFSFileDBInfo(fileRef, name, (UInt16 *)&attributes, (UInt16 *)&version, &crDate, &modDate, &bckUpDate, &modNum, &appInfoH, &sortInfoH, &type, &creator, &numRecords);
      if (attributesP) m68k_write_memory_16(attributesP, attributes);
      if (versionP) m68k_write_memory_16(versionP, version);
      if (crDateP) m68k_write_memory_32(crDateP, crDate);
      if (modDateP) m68k_write_memory_32(modDateP, modDate);
      if (bckUpDateP) m68k_write_memory_32(bckUpDateP, bckUpDate);
      if (modNumP) m68k_write_memory_32(modNumP, modNum);
      if (appInfoHP) m68k_write_memory_32(appInfoHP, 0); // XXX
      if (sortInfoHP) m68k_write_memory_32(sortInfoHP, 0); // XXX
      if (typeP) m68k_write_memory_32(typeP, type);
      if (creatorP) m68k_write_memory_32(creatorP, creator);
      if (numRecordsP) m68k_write_memory_32(numRecordsP, numRecords);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSFileDBInfo(fileRef=%d): %d", refP, res);
    }
    break;
    case vfsTrapImportDatabaseFromFile: {
      // Err VFSImportDatabaseFromFile(UInt16 volRefNum, const Char *pathNameP, UInt16 *cardNoP, LocalID *dbIDP)
      volRefNum = ARG16;
      pathNameP = ARG32;
      cardNoP = ARG32;
      dbIDP = ARG32;
      pathName = emupalmos_trap_sel_in(pathNameP, sysTrapFileSystemDispatch, sel, 1);
      res = VFSImportDatabaseFromFileEx(volRefNum, pathName, cardNoP ? &cardNo : NULL, dbIDP ? &dbID : NULL, NULL, NULL, 0, 0);
      if (cardNoP) m68k_write_memory_16(cardNoP, cardNo);
      if (dbIDP) m68k_write_memory_32(dbIDP, dbID);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSImportDatabaseFromFile(volRefNum=%d, pathName=\"%s\", dbID=0x%08X): %d", volRefNum, pathName, dbID, res);
    }
    break;
    case vfsTrapImportDatabaseFromFileCustom: {
      // Err VFSImportDatabaseFromFileCustom(UInt16 volRefNum, const Char *pathNameP, UInt16 *cardNoP, LocalID *dbIDP, VFSImportProcPtr importProcP, void *userDataP)
      volRefNum = ARG16;
      pathNameP = ARG32;
      cardNoP = ARG32;
      dbIDP = ARG32;
      importProcP = ARG32;
      userDataP = ARG32;
      pathName = emupalmos_trap_sel_in(pathNameP, sysTrapFileSystemDispatch, sel, 1);
      res = VFSImportDatabaseFromFileEx(volRefNum, pathName, cardNoP ? &cardNo : NULL, dbIDP ? &dbID : NULL, NULL, NULL, importProcP, userDataP);
      if (cardNoP) m68k_write_memory_16(cardNoP, cardNo);
      if (dbIDP) m68k_write_memory_32(dbIDP, dbID);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSImportDatabaseFromFileCustom(volRefNum=%d, pathName=\"%s\", dbID=0x%08X, importProc=0x%08X, userData=0x%08X): %d",
        volRefNum, pathName, dbID, importProcP, userDataP, res);
    }
    break;
    case vfsTrapExportDatabaseToFile: {
      // Err VFSExportDatabaseToFile(UInt16 volRefNum, const Char *pathNameP, UInt16 cardNo, LocalID dbID)
      volRefNum = ARG16;
      pathNameP = ARG32;
      cardNo = ARG16;
      dbID = ARG32;
      pathName = emupalmos_trap_sel_in(pathNameP, sysTrapFileSystemDispatch, sel, 1);
      res = VFSExportDatabaseToFileEx(volRefNum, pathName, cardNo, dbID, NULL, NULL, 0, 0);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSExportDatabaseToFile(volRefNum=%d, pathName=\"%s\", dbID=0x%08X): %d", volRefNum, pathName, dbID, res);
    }
    break;
    case vfsTrapExportDatabaseToFileCustom: {
      // Err VFSExportDatabaseToFileCustom(UInt16 volRefNum, const Char *pathNameP, UInt16 cardNo, LocalID dbID, VFSExportProcPtr exportProcP, void *userDataP)
      volRefNum = ARG16;
      pathNameP = ARG32;
      cardNo = ARG16;
      dbID = ARG32;
      exportProcP = ARG32;
      userDataP = ARG32;
      pathName = emupalmos_trap_sel_in(pathNameP, sysTrapFileSystemDispatch, sel, 1);
      res = VFSExportDatabaseToFileEx(volRefNum, pathName, cardNo, dbID, NULL, NULL, exportProcP, userDataP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "VFSExportDatabaseToFileCustom(volRefNum=%d, pathName=\"%s\", dbID=0x%08X, exportProc=0x%08X, userData=0x%08X): %d",
        volRefNum, pathName, dbID, exportProcP, userDataP, res);
    }
    break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "FileSystemDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
