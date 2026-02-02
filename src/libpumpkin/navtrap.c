#include <PalmOS.h>
#include <VFSMgr.h>
#include <HsNavCommon.h>
#include <HsExt.h>
#include <HsNav.h>
    
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
    
void palmos_navtrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];
  Err err;
  uint32_t formP = 0;
  uint16_t objID = 0;
  FormType *form = NULL;
  uint16_t objectID = 0;
  uint16_t extraInfo = 0;
  uint32_t boundsInsideRingP = 0;
  uint16_t ringStyle = 0;
  uint8_t forceRestore = 0;
  uint32_t objectIDP = 0;
  uint32_t extraInfoP = 0;
  uint32_t ringStyleP = 0;

  switch (sel) {
    case NavSelectorFrmNavObjectTakeFocus: {
      // void FrmNavObjectTakeFocus(const FormType* formP, UInt16 objID)
      formP = ARG32;
      objID = ARG16;
      form = emupalmos_trap_sel_in(formP, sysTrapNavSelector, sel, 0);
      FrmNavObjectTakeFocus(form, objID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmNavObjectTakeFocus(0x%08X, %d)", formP, objID);
      }
      break;
    case NavSelectorFrmNavDrawFocusRing: {
      // Err FrmNavDrawFocusRing(FormType* formP, UInt16 objectID, Int16 extraInfo,
      //    RectangleType* boundsInsideRingP,
      //    FrmNavFocusRingStyleEnum ringStyle, Boolean forceRestore)
      formP = ARG32;
      objectID = ARG16;
      extraInfo = ARG16;
      boundsInsideRingP = ARG32;
      ringStyle = ARG16;
      forceRestore = ARG8;
      emupalmos_trap_sel_in(formP, sysTrapNavSelector, sel, 0);
      emupalmos_trap_sel_in(boundsInsideRingP, sysTrapNavSelector, sel, 3);
      err = errNone;
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmNavDrawFocusRing(0x%08X, %u, %d, 0x%08X, %u, %u): %d",
        formP, objectID, extraInfo, boundsInsideRingP, ringStyle, forceRestore, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case NavSelectorFrmNavGetFocusRingInfo: {
      // Err FrmNavGetFocusRingInfo(const FormType *formP, UInt16 *objectIDP,
      //   Int16 *extraInfoP, RectangleType *boundsInsideRingP,
      //   FrmNavFocusRingStyleEnum *ringStyleP)
      formP = ARG32;
      objectIDP = ARG32;
      extraInfoP = ARG32;
      boundsInsideRingP = ARG32;
      ringStyleP = ARG32;
      emupalmos_trap_sel_in(formP, sysTrapNavSelector, sel, 0);
      emupalmos_trap_sel_in(objectIDP, sysTrapNavSelector, sel, 1);
      emupalmos_trap_sel_in(extraInfoP, sysTrapNavSelector, sel, 2);
      emupalmos_trap_sel_in(boundsInsideRingP, sysTrapNavSelector, sel, 3);
      emupalmos_trap_sel_in(ringStyleP, sysTrapNavSelector, sel, 4);
      if (objectIDP) m68k_write_memory_16(objectIDP, frmInvalidObjectId);
      if (extraInfoP) m68k_write_memory_16(extraInfoP, frmNavFocusRingNoExtraInfo);
      if (boundsInsideRingP) {
        m68k_write_memory_16(boundsInsideRingP, 0);
        m68k_write_memory_16(boundsInsideRingP + 2, 0);
        m68k_write_memory_16(boundsInsideRingP + 4, 0);
        m68k_write_memory_16(boundsInsideRingP + 6, 0);
      }
      if (ringStyleP) m68k_write_memory_16(ringStyleP, frmNavFocusRingStyleInvalid);
      err = uilibErrObjectNotFound;
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmNavGetFocusRingInfo(0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X): %d",
        formP, objectIDP, extraInfoP, boundsInsideRingP, ringStyleP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "NavSelector selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
