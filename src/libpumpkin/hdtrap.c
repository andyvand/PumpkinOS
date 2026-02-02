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

void palmos_highdensitytrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];
  Err err;
  BitmapType *bitmap = NULL;
  BitmapType *next = NULL;
  uint32_t a = 0;
  uint32_t bitmapP = 0;
  UInt8 version = 0;
  BitmapCompressionType type;
  UInt16 density = 0;
  uint32_t densityP = 0;
  UInt32 transparentValue = 0;
  uint32_t bitsP = 0;
  uint32_t colorTableP = 0;
  Boolean r = 0;
  void *bits = NULL;
  ColorTableType *colorTable = NULL;
  BitmapTypeV3 *bmpV3 = NULL;
  uint16_t coordSys = 0;
  UInt16 old = 0;
  uint32_t pointP = 0;
  uint8_t ceiling = 0;
  uint32_t rectP = 0;
  RectangleType rect;
  PointType point;
  Coord res = 0;
  int16_t width = 0;
  int16_t cornerRadius = 0;
  int16_t shadowWidth = 0;
  uint8_t selector = 0;
  uint32_t attrP = 0;
  UInt32 attr = 0;
  uint32_t w = 0;
  uint32_t xP = 0;
  uint32_t yP = 0;
  uint32_t downP = 0;
  WinHandle wh;
  Int16 x = 0, y = 0;
  Boolean down = false;
  uint32_t transparentValueP = 0;
  int16_t coord = 0;

  switch (sel) {
    case HDSelectorBmpGetNextBitmapAnyDensity: {
      // BitmapType *BmpGetNextBitmapAnyDensity(BitmapType *bitmapP)
      bitmapP = ARG32;
      bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      next = BmpGetNextBitmapAnyDensity(bitmap);
      a = emupalmos_trap_out(next);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetNextBitmapAnyDensity(0x%08X): 0x%08X", bitmapP, a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;

    case HDSelectorBmpGetVersion: {
      // UInt8 BmpGetVersion(const BitmapType *bitmapP)
      bitmapP = ARG32;
      bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      version = BmpGetVersion(bitmap);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetVersion(0x%08X): %d", bitmapP, version);
      m68k_set_reg(M68K_REG_D0, version);
    }
      break;

    case HDSelectorBmpGetCompressionType: {
      //BitmapCompressionType BmpGetCompressionType(const BitmapType *bitmapP)
      bitmapP = ARG32;
      bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      type = BmpGetCompressionType(bitmap);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetCompressionType(0x%08X): %d", bitmapP, type);
      m68k_set_reg(M68K_REG_D0, type);
    }
    break;

    case HDSelectorBmpGetDensity: {
      // UInt16 BmpGetDensity(const BitmapType *bitmapP)
      bitmapP = ARG32;
      bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      density = BmpGetDensity(bitmap);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetDensity(0x%08X): %d", bitmapP, density);
      m68k_set_reg(M68K_REG_D0, density);
    }
    break;

    case HDSelectorBmpSetDensity: {
      // Err BmpSetDensity(BitmapType *bitmapP, UInt16 density)
      bitmapP = ARG32;
      density = ARG16;
      bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      err = BmpSetDensity(bitmap, density);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpSetDensity(0x%08X, %d): %d", bitmapP, density, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;

    case HDSelectorBmpGetTransparentValue: {
      // Boolean BmpGetTransparentValue(const BitmapType *bitmapP, UInt32 *transparentValueP)
      bitmapP = ARG32;
      transparentValueP = ARG32;
      bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      r = BmpGetTransparentValue(bitmap, &transparentValue);
      if (transparentValueP) m68k_write_memory_32(transparentValueP, transparentValue);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetTransparentValue(0x%08X, 0x%08X): %d", bitmapP, transparentValueP, r);
      m68k_set_reg(M68K_REG_D0, r);
    }
    break;

    case HDSelectorBmpSetTransparentValue: {
      // void BmpSetTransparentValue(BitmapType *bitmapP, UInt32 transparentValue)
      bitmapP = ARG32;
      transparentValue = ARG32;
      bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      BmpSetTransparentValue(bitmap, transparentValue);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpSetTransparentValue(0x%08X, %d)", bitmapP, transparentValue);
    }
      break;

    case HDSelectorBmpCreateBitmapV3: {
      // BitmapTypeV3 *BmpCreateBitmapV3(const BitmapType *bitmapP, UInt16 density, const void *bitsP, const ColorTableType *colorTableP)
      bitmapP = ARG32;
      density = ARG16;
      bitsP = ARG32;
      colorTableP = ARG32;
      bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      bits = emupalmos_trap_sel_in(bitsP, sysTrapPinsDispatch, sel, 2);
      colorTable = emupalmos_trap_sel_in(colorTableP, sysTrapPinsDispatch, sel, 3);
      bmpV3 = BmpCreateBitmapV3(bitmap, density, bits, colorTable);
      a = emupalmos_trap_out(bmpV3);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpCreateBitmapV3(0x%08X, %d, 0x%08X, 0x%08X): 0x%08X", bitmapP, density, bitsP, colorTableP, a);
      m68k_set_reg(M68K_REG_A0, a);
    }
      break;

    case HDSelectorWinSetCoordinateSystem: {
      // UInt16 WinSetCoordinateSystem(UInt16 coordSys)
      coordSys = ARG16;
      old = WinSetCoordinateSystem(coordSys);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinSetCoordinateSystem(%d): %d", coordSys, old);
      m68k_set_reg(M68K_REG_D0, old);
     }
      break;

    case HDSelectorWinGetCoordinateSystem: {
      // UInt16 WinGetCoordinateSystem(void)
      coordSys = WinGetCoordinateSystem();
      debug(DEBUG_TRACE, "EmuPalmOS", "WinGetCoordinateSystem(): %d", coordSys);
      m68k_set_reg(M68K_REG_D0, coordSys);
     }
      break;

    case HDSelectorWinScalePoint: {
      // void WinScalePoint(PointType *pointP, Boolean ceiling)
      pointP = ARG32;
      ceiling = ARG8;
      decode_point(pointP, &point);
      WinScalePoint(pointP ? &point : NULL, ceiling);
      encode_point(pointP, &point);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinScalePoint(0x%08X [%d,%d], %d)", pointP, point.x, point.y, ceiling);
    }
    break;

    case HDSelectorWinUnscalePoint: {
      // void WinUnscalePoint(PointType *pointP, Boolean ceiling)
      pointP = ARG32;
      ceiling = ARG8;
      decode_point(pointP, &point);
      WinUnscalePoint(pointP ? &point : NULL, ceiling);
      encode_point(pointP, &point);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinUnscalePoint(0x%08X [%d,%d], %d)", pointP, point.x, point.y, ceiling);
    }
      break;

    case HDSelectorWinScaleRectangle: {
      // void WinScaleRectangle(RectangleType *rectP)
      rectP = ARG32;
      decode_rectangle(rectP, &rect);
      WinScaleRectangle(rectP ? &rect : NULL);
      encode_rectangle(rectP, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinScaleRectangle(0x%08X [%d,%d,%d,%d])", rectP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y);
    }
      break;

    case HDSelectorWinUnscaleRectangle: {
      // void WinUnscaleRectangle(RectangleType *rectP)
      rectP = ARG32;
      decode_rectangle(rectP, &rect);
      WinUnscaleRectangle(rectP ? &rect : NULL);
      encode_rectangle(rectP, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinUnscaleRectangle(0x%08X [%d,%d,%d,%d])", rectP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y);
    }
      break;

    case HDSelectorWinScreenGetAttribute: {
      // Err WinScreenGetAttribute(WinScreenAttrType selector, UInt32 *attrP)
      selector = ARG8;
      attrP = ARG32;
      attr = 0;
      err = WinScreenGetAttribute(selector, attrP ? &attr : NULL);
      if (attrP) m68k_write_memory_32(attrP, attr);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinScreenGetAttribute(%d, 0x%08X [%d]): %d", selector, attrP, attr, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
      break;

    //case HDSelectorWinPaintTiledBitmap:

    case HDSelectorWinGetSupportedDensity: {
      // Err WinGetSupportedDensity(UInt16 *densityP)
      densityP = ARG32;
      density = densityP ? m68k_read_memory_16(densityP) : 0;
      err = WinGetSupportedDensity(&density);
      if (densityP) m68k_write_memory_16(densityP, density);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinGetSupportedDensity(0x%08X): %d", densityP, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;

    case HDSelectorEvtGetPenNative: {
      // void EvtGetPenNative(WinHandle winH, Int16* pScreenX, Int16* pScreenY, Boolean* pPenDown)
      w = ARG32;
      xP = ARG32;
      yP = ARG32;
      downP = ARG32;
      wh = emupalmos_trap_sel_in(w, sysTrapPinsDispatch, sel, 0);
	  x = 0;
	  y = 0;
      down = false;
      EvtGetPenNative(wh, xP ? &x : NULL, yP ? &y : NULL, downP ? &down : NULL);
      if (xP) m68k_write_memory_16(xP, x);
      if (yP) m68k_write_memory_16(yP, y);
      if (downP) m68k_write_memory_8(downP, down);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtGetPenNative(0x%08X, 0x%08X [%d], 0x%08X [%d], 0x%08X [%d])", w, xP, x, yP, y, downP, down);
    }
      break;
    case HDSelectorWinScaleCoord: {
      // Coord WinScaleCoord(Coord coord, Boolean ceiling)
      coord = ARG16;
      ceiling = ARG8;
      res = WinScaleCoord(coord, ceiling);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinScaleCoord(%d, %d): %d", coord, ceiling, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;
    case HDSelectorWinUnscaleCoord: {
      // Coord WinUnscaleCoord(Coord coord, Boolean ceiling)
      coord = ARG16;
      ceiling = ARG8;
      res = WinUnscaleCoord(coord, ceiling);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinUnscaleCoord(%d, %d): %d", coord, ceiling, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;

    case HDSelectorWinPaintRoundedRectangleFrame: {
	  width = ARG16;
	  cornerRadius = ARG16;
	  shadowWidth = ARG16;
	  // void WinPaintRoundedRectangleFrame(const RectangleType *rP, Coord width, Coord cornerRadius, Coord shadowWidth)
      rectP = ARG32;
      decode_rectangle(rectP, &rect);
      WinPaintRoundedRectangleFrame(&rect, width, cornerRadius, shadowWidth);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintRoundedRectangleFrame(0x%08X [%d,%d,%d,%d], %d, %d, %d)", rectP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y, width, cornerRadius, shadowWidth);
    }
    break;

    //case HDSelectorWinSetScalingMode:
    //case HDSelectorWinGetScalingMode:

    default:
      sys_snprintf(buf, sizeof(buf)-1, "HighDensityDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
