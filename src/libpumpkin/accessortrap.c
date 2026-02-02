#include <PalmOS.h>
#include <VFSMgr.h>
#include <FrmGlue.h>
#include <CtlGlue.h>
#include <FldGlue.h>
#include <LstGlue.h>
#include <TblGlue.h>
#include <BmpGlue.h>
#include <WinGlue.h>

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

void palmos_accessortrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];
  uint32_t ctlP = 0;
  ControlType *ctl = NULL;
  ControlStyleType res = 0;
  Boolean resb = 0;
  uint32_t fldP = 0;
  uint16_t lineNum = 0;
  uint32_t startP = 0;
  uint32_t lengthP = 0;
  FieldType *fld = NULL;
  UInt16 start = 0, length = 0;
  uint32_t formP = 0;
  uint16_t objIndex = 0;
  FormType *form = NULL;
  uint32_t bitmapP = 0;
  BitmapType *bitmap = NULL;
  BitmapCompressionType resbm;
  uint32_t transparentValueP = 0;
  UInt32 transparentValue = 0;
  FontID resf = 0;
  uint8_t fontID = 0;
  uint32_t bitmapIDP = 0;
  uint32_t selectedBitmapIDP = 0;
  DmResID bitmapID, selectedBitmapID;
  uint32_t formPP = 0;
  uint16_t id = 0;
  uint8_t style = 0;
  uint16_t thumbID = 0;
  uint16_t backgroundID = 0;
  int16_t x = 0;
  int16_t y = 0;
  int16_t width = 0;
  int16_t height = 0;
  uint16_t minValue = 0;
  uint16_t maxValue = 0;
  uint16_t pageSize = 0;
  uint16_t value = 0;
  SliderControlType *ctls = NULL;
  uint32_t a = 0;
  uint8_t leftAnchor = 0;
  uint16_t defaultButton = 0;
  uint16_t helpRscID = 0;
  uint16_t labelID = 0;
  UInt32 handler = 0;
  uint32_t listP = 0;
  ListType *list = NULL;
  FontID font = 0;
  ListType *lst = NULL;
  Char **itemsText = NULL;
  uint8_t incrementalSearch = 0;
  uint32_t tableP = 0;
  int16_t column = 0;
  TableType *tbl = NULL;
  uint32_t winH = 0;
  WinHandle wh;
  FrameType resfr;
  uint16_t frame = 0;
  uint32_t formObjP = 0;
  uint16_t objKind = 0;
  void *obj = NULL;
  void *resv = NULL;
  uint8_t frameStyle = 0;

  switch (sel) {
    case 0: {
      // ControlStyleType CtlGlueGetControlStyle(const ControlType *ctlP)
      ctlP = ARG32;
      ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      res = CtlGlueGetControlStyle(ctl);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueGetControlStyle(0x%08X): %d", ctlP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 1: {
      // Boolean FldGlueGetLineInfo(const FieldType *fldP, UInt16 lineNum, UInt16* startP, UInt16* lengthP)
      fldP = ARG32;
      lineNum = ARG16;
      startP = ARG32;
      lengthP = ARG32;
      fld = (FieldType *)emupalmos_trap_sel_in(fldP, sysTrapAccessorDispatch, sel, 0);
      emupalmos_trap_sel_in(startP, sysTrapAccessorDispatch, sel, 2);
      emupalmos_trap_sel_in(lengthP, sysTrapAccessorDispatch, sel, 3);
      resb = FldGlueGetLineInfo(fld, lineNum, &start, &length);
      if (startP) m68k_write_memory_32(startP, start);
      if (lengthP) m68k_write_memory_32(lengthP, length);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGlueGetLineInfo(0x%08X, %u, 0x%08X, 0x%08X): %d", fldP, lineNum, startP, lengthP, resb);
      m68k_set_reg(M68K_REG_D0, resb);
      }
      break;
    case 2: {
      // Boolean FrmGlueGetObjectUsable(const FormType *formP, UInt16 objIndex)
      formP = ARG32;
      objIndex = ARG16;
      form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      resb = FrmGlueGetObjectUsable(form, objIndex);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueGetObjectUsable(0x%08X, %d): %d", formP, objIndex, resb);
      m68k_set_reg(M68K_REG_D0, resb);
      }
      break;
    case 3: {
      // BitmapCompressionType BmpGlueGetCompressionType(const BitmapType *bitmapP)
      bitmapP = ARG32;
      bitmap = (BitmapType *)emupalmos_trap_sel_in(bitmapP, sysTrapAccessorDispatch, sel, 0);
      resbm = BmpGlueGetCompressionType(bitmapP ? bitmap : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGlueGetCompressionType(0x%08X): %d", bitmapP, resbm);
      m68k_set_reg(M68K_REG_D0, resbm);
      }
      break;
    case 4: {
      // Boolean BmpGlueGetTransparentValue(const BitmapType *bitmapP, UInt32 *transparentValueP)
      bitmapP = ARG32;
      transparentValueP = ARG32;
      bitmap = (BitmapType *)emupalmos_trap_sel_in(bitmapP, sysTrapAccessorDispatch, sel, 0);
      emupalmos_trap_sel_in(transparentValueP, sysTrapAccessorDispatch, sel, 1);
      resb = BmpGlueGetTransparentValue(bitmapP ? bitmap : NULL, &transparentValue);
      if (transparentValueP) m68k_write_memory_32(transparentValueP, transparentValue);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGlueGetTransparentValue(0x%08X, 0x%08X [0x%08X]): %d", bitmapP, transparentValueP, transparentValue, resb);
      m68k_set_reg(M68K_REG_D0, resb);
      }
      break;
    case 5: {
      // void BmpGlueSetTransparentValue(BitmapType *bitmapP, UInt32 transparentValue)
      bitmapP = ARG32;
      transparentValue = ARG32;
      bitmap = (BitmapType *)emupalmos_trap_sel_in(bitmapP, sysTrapAccessorDispatch, sel, 0);
      BmpGlueSetTransparentValue(bitmap, transparentValue);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGlueSetTransparentValue(0x%08X, 0x%08X)", bitmapP, transparentValue);
      }
      break;
    case 6: {
      // FontID CtlGlueGetFont(const ControlType *ctlP)
      ctlP = ARG32;
      ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      resf = CtlGlueGetFont(ctl);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueGetFont(0x%08X): %d", ctlP, resf);
      m68k_set_reg(M68K_REG_D0, resf);
      break;
      }
    case 7: {
      // void CtlGlueSetFont(ControlType *ctlP, FontID fontID)
      ctlP = ARG32;
      fontID = ARG8;
      ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      CtlGlueSetFont(ctl, fontID);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueSetFont(0x%08X, %d)", ctlP, fontID);
      }
      break;
    case 8: {
      // void CtlGlueGetGraphics(const ControlType *ctlP, DmResID *bitmapID, DmResID *selectedBitmapID)
      ctlP = ARG32;
      bitmapIDP = ARG32;
      selectedBitmapIDP = ARG32;
      ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      CtlGlueGetGraphics(ctl, &bitmapID, &selectedBitmapID);
      if (bitmapIDP) m68k_write_memory_32(bitmapIDP, bitmapID);
      if (selectedBitmapIDP) m68k_write_memory_32(selectedBitmapIDP, selectedBitmapID);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueGetGraphics(0x%08X, 0x%08X, 0x%08X)", ctlP, bitmapIDP, selectedBitmapIDP);
      }
      break;
    case 9: {
      // SliderControlType *CtlGlueNewSliderControl(void **formPP, UInt16 ID, ControlStyleType style, DmResID thumbID, DmResID backgroundID, Coord x, Coord y, Coord width, Coord height, UInt16 minValue, UInt16 maxValue, UInt16 pageSize, UInt16 value)
      formPP = ARG32;
      id = ARG16;
      style = ARG8;
      thumbID = ARG16;
      backgroundID = ARG16;
      x = ARG16;
      y = ARG16;
      width = ARG16;
      height = ARG16;
      minValue = ARG16;
      maxValue = ARG16;
      pageSize = ARG16;
      value = ARG16;
      formP = formPP ? m68k_read_memory_32(formPP) : 0;
      form = emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      ctls = CtlGlueNewSliderControl((void **)&form, id, style, thumbID, backgroundID, x, y, width, height, minValue, maxValue, pageSize, value);
      a = emupalmos_trap_out(ctls);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueNewSliderControl(0x%08X, %u, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d): 0x%08X", formPP, id, style, thumbID, backgroundID, x, y, width, height, minValue, maxValue, pageSize, value, a);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case 10: {
      // void CtlGlueSetLeftAnchor(ControlType *ctlP, Boolean leftAnchor)
      ctlP = ARG32;
      leftAnchor = ARG8;
      ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      CtlGlueSetLeftAnchor(ctl, leftAnchor);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueSetLeftAnchor(0x%08X, %d)", ctlP, leftAnchor);
      }
      break;
    case 11: {
      // UInt16 FrmGlueGetDefaultButtonID(const FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
       res = FrmGlueGetDefaultButtonID(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueGetDefaultButtonID(0x%08X): %u", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 12: {
      // void FrmGlueSetDefaultButtonID(FormType *formP, UInt16 defaultButton)
      formP = ARG32;
      defaultButton = ARG16;
      form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      FrmGlueSetDefaultButtonID(form, defaultButton);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueSetDefaultButtonID(0x%08X, %u)", formP, defaultButton);
      }
      break;
    case 13: {
      // UInt16 FrmGlueGetHelpID(const FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      res = FrmGlueGetHelpID(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueGetHelpID(0x%08X): %u", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 14: {
      // void FrmGlueSetHelpID(FormType *formP, UInt16 helpRscID)
      formP = ARG32;
      helpRscID = ARG16;
      form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      FrmGlueSetHelpID(form, helpRscID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueSetHelpID(0x%08X, %u)", formP, helpRscID);
      }
      break;
    case 15: {
      // UInt16 FrmGlueGetMenuBarID(const FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      res = FrmGlueGetMenuBarID(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueGetMenuBarID(0x%08X): %u", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 16: {
      // FontID FrmGlueGetLabelFont(const FormType *formP, UInt16 labelID)
      formP = ARG32;
      labelID = ARG16;
      form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      res = FrmGlueGetLabelFont(form, labelID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueGetLabelFont(0x%08X, %u): %u", formP, labelID, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 17: {
      // void FrmGlueSetLabelFont(FormType *formP, UInt16 labelID, FontID fontID)
      formP = ARG32;
      labelID = ARG16;
      fontID = ARG8;
      form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      FrmGlueSetLabelFont(form, labelID, fontID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueSetLabelFont(0x%08X, %u, %u)", formP, labelID, fontID);
      }
      break;
    case 18: {
      // FormEventHandlerType *FrmGlueGetEventHandler(const FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      handler = form->m68k_handler;
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueGetEventHandler(0x%08X): 0x%08X", formP, handler);
      m68k_set_reg(M68K_REG_A0, handler);
      }
      break;
    case 19: {
      // FontID LstGlueGetFont(const ListType *listP)
      listP = ARG32;
      list = (ListType *)emupalmos_trap_sel_in(listP, sysTrapAccessorDispatch, sel, 0);
      font = LstGlueGetFont(list);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGlueGetFont(0x%08X): %d", listP, font);
      m68k_set_reg(M68K_REG_D0, font);
      }
      break;
    case 20: {
      // void LstGlueSetFont(ListType *listP, FontID fontID)
      listP = ARG32;
      fontID = ARG8;
      lst = (ListType *)emupalmos_trap_sel_in(listP, sysTrapAccessorDispatch, sel, 0);
      LstGlueSetFont(lst, fontID);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGlueSetFont(0x%08X, %u)", listP, fontID);
      }
      break;
    case 21: {
      // Char **LstGlueGetItemsText(const ListType *listP)
      listP = ARG32;
      lst = (ListType *)emupalmos_trap_sel_in(listP, sysTrapAccessorDispatch, sel, 0);
      itemsText = LstGlueGetItemsText(lst);
      a = emupalmos_trap_out(itemsText);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGlueGetItemsText(0x%08X): 0x%08X", listP, a);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case 22: {
      // void LstGlueSetIncrementalSearch(ListType *listP, Boolean incrementalSearch)
      listP = ARG32;
      incrementalSearch = ARG8;
      lst = (ListType *)emupalmos_trap_sel_in(listP, sysTrapAccessorDispatch, sel, 0);
      LstGlueSetIncrementalSearch(lst, incrementalSearch);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGlueSetIncrementalSearch(0x%08X, %u)", listP, incrementalSearch);
      }
      break;
    case 23: {
      // Boolean TblGlueGetColumnMasked(const TableType *tableP, Int16 column)
      tableP = ARG32;
      column = ARG8;
      tbl = (TableType *)emupalmos_trap_sel_in(tableP, sysTrapAccessorDispatch, sel, 0);
      resb = TblGlueGetColumnMasked(tbl, column);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGlueGetColumnMasked(0x%08X, %d): %u", tableP, column, resb);
      m68k_set_reg(M68K_REG_D0, resb);
      }
      break;
    case 24: {
      // FrameType WinGlueGetFrameType(const WinHandle winH)
      winH = ARG32;
      wh = (WinHandle)emupalmos_trap_sel_in(winH, sysTrapAccessorDispatch, sel, 0);
      resfr = WinGlueGetFrameType(wh);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinGlueGetFrameType(0x%08X): %u", winH, resfr);
      m68k_set_reg(M68K_REG_D0, resfr);
      }
      break;
    case 25: {
      // void WinGlueSetFrameType(WinHandle winH, FrameType frame)
      winH = ARG32;
      frame = ARG16;
      wh = (WinHandle)emupalmos_trap_sel_in(winH, sysTrapAccessorDispatch, sel, 0);
      WinGlueSetFrameType(wh, frame);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinGlueSetFrameType(0x%08X, %u)", winH, frame);
      }
      break;
    case 26: {
      // UInt16 FrmGlueGetObjIDFromObjPtr(void *formObjP, FormObjectKind objKind)
      formObjP = ARG32;
      objKind = ARG16;
      obj = emupalmos_trap_sel_in(formObjP, sysTrapAccessorDispatch, sel, 0);
      res = FrmGlueGetObjIDFromObjPtr(obj, objKind);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueGetObjIDFromObjPtr(0x%08X, %u): %u", formObjP, objKind, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 27: {
      // void *LstGlueGetDrawFunction(ListType* listP)
      listP = ARG32;
      lst = (ListType *)emupalmos_trap_sel_in(listP, sysTrapAccessorDispatch, sel, 0);
      resv = LstGlueGetDrawFunction(lst);
      a = emupalmos_trap_out(resv);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGlueGetDrawFunction(0x%08X): 0x%08X", listP, a);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case 28: {
      // Boolean CtlGlueIsGraphical(ControlType* controlP)
      ctlP = ARG32;
      ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      resb = CtlGlueIsGraphical(ctl);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueIsGraphical(0x%08X): %u", ctlP, resb);
      m68k_set_reg(M68K_REG_D0, resb);
      }
      break;
    case 29: {
      // void CtlGlueSetFrameStyle(ControlType* controlP, ButtonFrameType frameStyle)
      ctlP = ARG32;
      frameStyle = ARG8;
      ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      CtlGlueSetFrameStyle(ctl, frameStyle);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueSetFrameStyle(0x%08X, %u)", ctlP, frameStyle);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "AccessorDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
