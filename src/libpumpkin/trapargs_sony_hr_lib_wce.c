// This file was generated with the command: gen_log.sh SONY_HR_LIB
// DO NOT MODIFY!

static trap_t trapArgs_SONY_HR_LIB[] = {
  { sysLibTrapOpen, -1, "HROpen", T_USIG, 0, 2, 1, { { T_USIG, 0, 2, 0, "refNum" } } },
  { sysLibTrapClose, -1, "HRClose", T_USIG, 0, 2, 1, { { T_USIG, 0, 2, 0, "refNum" } } },
  { sysLibTrapSleep, -1, "HRSleep", T_USIG, 0, 2, 1, { { T_USIG, 0, 2, 0, "refNum" } } },
  { sysLibTrapWake, -1, "HRWake", T_USIG, 0, 2, 1, { { T_USIG, 0, 2, 0, "refNum" } } },
  { HRTrapGetAPIVersion, -1, "HRGetAPIVersion", T_USIG, 0, 2, 2, { { T_USIG, 0, 2, 0, "refNum" }, { T_USIG, 1, 2, 0, "versionP" } } },
  { HRTrapWinClipRectangle, -1, "HRWinClipRectangle", T_VOID, 0, 0, 2, { { T_USIG, 0, 2, 0, "refNum" }, { T_RCT, 1, 4, 0, "rP" } } },
  { HRTrapWinCopyRectangle, -1, "HRWinCopyRectangle", T_VOID, 0, 0, 7, { { T_USIG, 0, 2, 0, "refNum" }, { T_VOID, 1, 4, 0, "srcWin" }, { T_VOID, 1, 4, 0, "dstWin" }, { T_RCT, 1, 4, 0, "srcRect" }, { T_SIG, 0, 2, 0, "destX" }, { T_SIG, 0, 2, 0, "destY" }, { T_USIG, 0, 1, 0, "mode" } } },
  { HRTrapWinCreateBitmapWindow, -1, "HRWinCreateBitmapWindow", T_VOID, 1, 4, 3, { { T_USIG, 0, 2, 0, "refNum" }, { T_BMP, 1, 4, 0, "bitmapP" }, { T_USIG, 1, 2, 0, "error" } } },
  { HRTrapWinDrawBitmap, -1, "HRWinDrawBitmap", T_VOID, 0, 0, 4, { { T_USIG, 0, 2, 0, "refNum" }, { T_BMP, 1, 4, 0, "bitmapP" }, { T_SIG, 0, 2, 0, "x" }, { T_SIG, 0, 2, 0, "y" } } },
  { HRTrapWinScreenMode, -1, "HRWinScreenMode", T_USIG, 0, 2, 6, { { T_USIG, 0, 2, 0, "refNum" }, { T_USIG, 0, 1, 0, "operation" }, { T_USIG, 1, 4, OMIT_IN, "widthP" }, { T_USIG, 1, 4, OMIT_IN, "heightP" }, { T_USIG, 1, 4, OMIT_IN, "depthP" }, { T_USIG, 1, 1, OMIT_IN, "enableColorP" } } },
  { HRTrapBmpCreate, -1, "HRBmpCreate", T_BMP, 1, 4, 6, { { T_USIG, 0, 2, 0, "refNum" }, { T_SIG, 0, 2, 0, "width" }, { T_SIG, 0, 2, 0, "height" }, { T_USIG, 0, 1, 0, "depth" }, { T_VOID, 1, 4, 0, "colortableP" }, { T_USIG, 1, 2, 0, "error" } } },
  { HRTrapBmpBitsSize, -1, "HRBmpBitsSize", T_USIG, 0, 2, 2, { { T_USIG, 0, 2, 0, "refNum" }, { T_BMP, 1, 4, 0, "bitmapP" } } },
  { 0 } 
};
