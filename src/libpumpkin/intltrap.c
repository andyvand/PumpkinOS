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

void palmos_intltrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];
  uint16_t inChar = 0;
  UInt16 res = 0;
  Boolean resb = 0;
  uint32_t inTextP = 0;
  uint32_t inOffset = 0;
  uint32_t outCharP = 0;
  char *inText = NULL;
  WChar outChar;
  uint32_t ioTextP = 0;
  char *ioText = NULL;
  uint32_t inTemplateP = 0;
  uint32_t param0P = 0;
  uint32_t param1P = 0;
  uint32_t param2P = 0;
  uint32_t param3P = 0;
  char *inTemplate = NULL;
  char *param0 = NULL;
  char *param1 = NULL;
  char *param2 = NULL;
  char *param3 = NULL;
  char *resc = NULL;
  uint32_t r = 0;
  uint32_t ioStrP = 0;
  uint16_t inMaxLen = 0;
  uint32_t inParamStrP = 0;
  uint16_t inParamNum = 0;
  char *ioStr = NULL;
  char *inParamStr = NULL;
  Int16 ress = 0;
  uint32_t s1P = 0;
  uint16_t s1Len = 0;
  uint32_t s1MatchLenP = 0;
  uint32_t s2P = 0;
  uint16_t s2Len = 0;
  uint32_t s2MatchLenP = 0;
  char *s1 = NULL;
  char *s2 = NULL;
  UInt16 s1MatchLen = 0, s2MatchLen = 0;
  uint32_t outStartP = 0;
  uint32_t outEndP = 0;
  UInt32 outStart = 0, outEnd = 0;
  WChar resw = 0;

  switch (sel) {
    case intlTxtCharAttr: {
      // UInt16 TxtCharAttr(WChar inChar)
      inChar = ARG16;
      res = TxtCharAttr(inChar);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtCharAttr(%d): %d", inChar, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;
    case intlTxtCharSize: {
      // UInt16 TxtCharSize(WChar inChar)
      inChar = ARG16;
      res = TxtCharSize(inChar);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtCharSize(%d): %d", inChar, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;
    case intlTxtGetChar: {
      // WChar TxtGetChar(const Char *inText, UInt32 inOffset)
      inTextP = ARG32;
      inOffset = ARG32;
      inText = emupalmos_trap_sel_in(inTextP, sysTrapIntlDispatch, sel, 0);
      res = TxtGetChar(inText, inOffset);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtGetChar(0x%08X \"%s\", %d): %d", inTextP, inText, inOffset, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;
    case intlTxtGetPreviousChar: {
      // UInt16 TxtGetPreviousChar(const Char *inText, UInt32 inOffset, WChar *outChar)
      inTextP = ARG32;
      inOffset = ARG32;
      outCharP = ARG32;
      inText = emupalmos_trap_sel_in(inTextP, sysTrapIntlDispatch, sel, 0);
      res = TxtGetPreviousChar(inText, inOffset, &outChar);
      if (outCharP) m68k_write_memory_16(outCharP, outChar);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtGetPreviousChar(0x%08X \"%s\", %d, 0x%08X): %d", inTextP, inText, inOffset, outCharP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;
    case intlTxtGetNextChar: {
      // UInt16 TxtGetNextChar(const Char *inText, UInt32 inOffset, WChar *outChar)
      inTextP = ARG32;
      inOffset = ARG32;
      outCharP = ARG32;
      inText = emupalmos_trap_sel_in(inTextP, sysTrapIntlDispatch, sel, 0);
      emupalmos_trap_sel_in(outCharP, sysTrapIntlDispatch, sel, 2);
      res = TxtGetNextChar(inText, inOffset, &outChar);
      if (outCharP) m68k_write_memory_16(outCharP, outChar);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtGetNextChar(0x%08X \"%s\", %u, 0x%08X): %d", inTextP, inText, inOffset, outCharP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;
    case intlTxtSetNextChar: {
      // UInt16 TxtSetNextChar(Char *ioText, UInt32 inOffset, WChar inChar)
      ioTextP = ARG32;
      inOffset = ARG32;
      inChar = ARG16;
      ioText = emupalmos_trap_sel_in(ioTextP, sysTrapIntlDispatch, sel, 0);
      res = TxtSetNextChar(ioText, inOffset, inChar);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtSetNextChar(0x%08X \"%s\", %u, %u): %d", ioTextP, ioText, inOffset, inChar, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;
    case intlTxtParamString: {
      // Char *TxtParamString(const Char *inTemplate, const Char *param0, const Char *param1, const Char *param2, const Char *param3)
      inTemplateP = ARG32;
      param0P = ARG32;
      param1P = ARG32;
      param2P = ARG32;
      param3P = ARG32;
      inTemplate = emupalmos_trap_sel_in(inTemplateP, sysTrapIntlDispatch, sel, 0);
      param0 = emupalmos_trap_sel_in(param0P, sysTrapIntlDispatch, sel, 1);
      param1 = emupalmos_trap_sel_in(param1P, sysTrapIntlDispatch, sel, 2);
      param2 = emupalmos_trap_sel_in(param2P, sysTrapIntlDispatch, sel, 3);
      param3 = emupalmos_trap_sel_in(param3P, sysTrapIntlDispatch, sel, 4);
      resc = TxtParamString(inTemplate, param0, param1, param2, param3);
      r = emupalmos_trap_out(resc);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtParamString(0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X): 0x%08X", inTemplateP, param0P, param1P, param2P, param3P, r);
      m68k_set_reg(M68K_REG_A0, r);
    }
      break;
    case intlTxtCharIsValid: {
      // Boolean TxtCharIsValid(WChar inChar)
      inChar = ARG16;
      resb = TxtCharIsValid(inChar);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtCharIsValid(%d): %d", inChar, resb);
      m68k_set_reg(M68K_REG_D0, resb);
      }
      break;
    case intlTxtReplaceStr: {
      // UInt16 TxtReplaceStr(Char *ioStr, UInt16 inMaxLen, const Char *inParamStr, UInt16 inParamNum)
      ioStrP = ARG32;
      inMaxLen = ARG16;
      inParamStrP = ARG32;
      inParamNum = ARG16;
      ioStr = emupalmos_trap_sel_in(ioStrP, sysTrapIntlDispatch, sel, 0);
      inParamStr = emupalmos_trap_sel_in(inParamStrP, sysTrapIntlDispatch, sel, 2);
      res = TxtReplaceStr(ioStr, inMaxLen, inParamStr, inParamNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtReplaceStr(0x%08X, %d, 0x%08X, %d): %d", ioStrP, inMaxLen, inParamStrP, inParamNum, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case intlTxtCompare: {
      // Int16 TxtCompare(const Char *s1, UInt16 s1Len, UInt16 *s1MatchLen, const Char *s2, UInt16 s2Len, UInt16 *s2MatchLen)
      s1P = ARG32;
      s1Len = ARG16;
      s1MatchLenP = ARG32;
      s2P = ARG32;
      s2Len = ARG16;
      s2MatchLenP = ARG32;
      s1 = emupalmos_trap_sel_in(s1P, sysTrapIntlDispatch, sel, 0);
      s2 = emupalmos_trap_sel_in(s2P, sysTrapIntlDispatch, sel, 3);
      ress = TxtCompare(s1, s1Len, &s1MatchLen, s2, s2Len, &s2MatchLen);
      if (s1MatchLenP) m68k_write_memory_16(s1MatchLenP, s1MatchLen);
      if (s2MatchLenP) m68k_write_memory_16(s2MatchLenP, s2MatchLen);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtCompare(0x%08X, %d, 0x%08X, 0x%08X, %d, 0x%08X): %d", s1P, s1Len, s1MatchLenP, s2P, s2Len, s2MatchLenP, ress);
      m68k_set_reg(M68K_REG_D0, ress);
      }
      break;
    case intlTxtCharBounds: {
      // WChar TxtCharBounds(const Char *inText, UInt32 inOffset, UInt32 *outStart, UInt32 *outEnd)
      inTextP = ARG32;
      inOffset = ARG32;
      outStartP = ARG32;
      outEndP = ARG32;
      inText = emupalmos_trap_sel_in(inTextP, sysTrapIntlDispatch, sel, 0);
      emupalmos_trap_sel_in(outStartP, sysTrapIntlDispatch, sel, 2);
      emupalmos_trap_sel_in(outEndP, sysTrapIntlDispatch, sel, 3);
      resw = TxtCharBounds(inTextP ? inText : NULL, inOffset, outStartP ? &outStart : NULL, outEndP ? &outEnd : NULL);
      if (outStartP) m68k_write_memory_32(outStartP, outStart);
      if (outEndP) m68k_write_memory_32(outEndP, outEnd);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtCharBounds(0x%08X \"%s\", %u, 0x%08X, 0x%08X): %d", inTextP, inText, inOffset, outStartP, outEndP, resw);
      m68k_set_reg(M68K_REG_D0, resw);
      }
      break;
    //case intlIntlInit:
    //case intlTxtByteAttr:
    //case intlTxtCharXAttr:
    //case intlTxtPrepFindString:
    //case intlTxtFindString:
    //case intlTxtWordBounds:
    //case intlTxtCharEncoding:
    //case intlTxtStrEncoding:
    //case intlTxtEncodingName:
    //case intlTxtMaxEncoding:
    //case intlTxtTransliterate:
    //case intlTxtCaselessCompare:
    //case intlTxtCharWidth:
    //case intlTxtGetTruncationOffset:
    //case intlIntlGetRoutineAddress:
    //case intlIntlHandleEvent:
    //case intlTxtConvertEncodingV35:
    //case intlTxtConvertEncoding:
    //case intlIntlSetRoutineAddress:
    //case intlTxtGetWordWrapOffset:
    //case intlTxtNameToEncoding:
    //case intlIntlStrictChecks:
    default:
      sys_snprintf(buf, sizeof(buf)-1, "IntlDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
