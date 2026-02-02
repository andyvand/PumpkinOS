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

void palmos_pinstrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];
  Err err = 0;
  uint16_t selector = 0;
  uint32_t dataP = 0;
  UInt32 data = 0;
  UInt16 orientation = 0;
  uint16_t triggerState = 0;
  uint32_t w = 0;
  uint16_t minH = 0;
  uint16_t prefH = 0;
  uint16_t maxH = 0;
  uint16_t minW = 0;
  uint16_t prefW = 0;
  uint16_t maxW = 0;
  WinHandle wh;
  uint32_t formP = 0;
  uint16_t diaPolicy = 0;
  FormType *form = NULL;
  uint16_t state = 0;

  switch (sel) {
    case pinPINSetInputAreaState: {
      // Err PINSetInputAreaState(UInt16 state)
      state = ARG16;
      err = PINSetInputAreaState(state);
      debug(DEBUG_TRACE, "EmuPalmOS", "PINSetInputAreaState(%d): %d", state, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case pinPINGetInputAreaState: {
      // UInt16 PINGetInputAreaState(void)
      state = PINGetInputAreaState();
      debug(DEBUG_TRACE, "EmuPalmOS", "PINGetInputAreaState(): %d", state);
      m68k_set_reg(M68K_REG_D0, state);
    }
    break;
    case pinPINSetInputTriggerState: {
      // Err PINSetInputTriggerState(UInt16 state)
      state = ARG16;
      err = PINSetInputTriggerState(state);
      debug(DEBUG_TRACE, "EmuPalmOS", "PINSetInputTriggerState(%d): %d", state, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    //case pinPINGetInputTriggerState:
    //case pinPINAltInputSystemEnabled:
    //case pinPINGetCurrentPinletName:
    //case pinPINSwitchToPinlet:
    //case pinPINCountPinlets:
    //case pinPINGetPinletInfo:
    //case pinPINSetInputMode:
    //case pinPINGetInputMode:
    //case pinPINClearPinletState:
    //case pinPINShowReferenceDialog:
    case pinWinSetConstraintsSize: {
	  w = ARG32;
	  minH = ARG16;
	  prefH = ARG16;
	  maxH = ARG16;
	  minW = ARG16;
	  prefW = ARG16;
	  maxW = ARG16;
	  wh = emupalmos_trap_sel_in(w, sysTrapPinsDispatch, sel, 0);
	  // Err WinSetConstraintsSize(WinHandle winH, Coord minH, Coord prefH, Coord maxH, Coord minW, Coord prefW, Coord maxW)
      err = WinSetConstraintsSize(wh, minH, prefH, maxH, minW, prefW, maxW);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinSetConstraintsSize(0x%08X, %d, %d, %d, %d, %d, %d): %d", w, minH, prefH, maxH, minW, prefW, maxW, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case pinFrmSetDIAPolicyAttr: {
	  formP = ARG32;
	  diaPolicy = ARG16;
	  form = emupalmos_trap_sel_in(formP, sysTrapPinsDispatch, sel, 0);
	  // Err FrmSetDIAPolicyAttr(FormPtr formP, UInt16 diaPolicy)
      err = FrmSetDIAPolicyAttr(form, diaPolicy);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetDIAPolicyAttr(0x%08X, %d): %d", formP, diaPolicy, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    //case pinFrmGetDIAPolicyAttr:
    case pinStatHide: {
      // Err StatHide(void)
      err = StatHide();
      debug(DEBUG_TRACE, "EmuPalmOS", "StatHide(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case pinStatShow: {
      // Err StatShow(void)
      err = StatShow();
      debug(DEBUG_TRACE, "EmuPalmOS", "StatShow(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case pinStatGetAttribute: {
	  selector = ARG16;
	  dataP = ARG32;
	  // Err StatGetAttribute(UInt16 selector, UInt32 *dataP)
      err = StatGetAttribute(selector, dataP ? &data : NULL);
      if (dataP) m68k_write_memory_32(dataP, data);
      debug(DEBUG_TRACE, "EmuPalmOS", "StatGetAttribute(%d, 0x%08X): %d", selector, dataP, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case pinSysGetOrientation: {
      //  UInt16 SysGetOrientation(void)
      orientation = SysGetOrientation();
      debug(DEBUG_TRACE, "EmuPalmOS", "SysGetOrientation(): %d", orientation);
      m68k_set_reg(M68K_REG_D0, orientation);
    }
    break;
    case pinSysSetOrientation: {
      // Err SysSetOrientation(UInt16 orientation)
      orientation = ARG16;
      err = SysSetOrientation(orientation);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysSetOrientation(%d): %d", orientation, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    //case pinSysGetOrientationTriggerState
    case pinSysSetOrientationTriggerState: {
      // Err SysSetOrientationTriggerState(UInt16 triggerState)
      triggerState = ARG16;
      err = SysSetOrientationTriggerState(triggerState);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysSetOrientationTriggerState(%d): %d", triggerState, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "PinsDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
