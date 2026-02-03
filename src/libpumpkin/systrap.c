#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <CharAttr.h>
#include <HsNavCommon.h>

#include "sys.h"
#include "mutex.h"
#include "storage.h"
#include "pumpkin.h"
#include "bytes.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "logtrap.h"
#include "emupalmosinc.h"
#include "m68k.h"
#include "m68kcpu.h"
#include "emupalmos.h"
#include "launch_serde.h"
#include "emu_launch_serde.h"
#include "debug.h"

// not mapped:
// FldNewField

static void palmos_libtrap(uint16_t refNum, uint16_t trap) {
  char buf[256];

  switch (refNum) {
    case NetLibRefNum:
      palmos_netlibtrap(trap);
      break;
    case GPDLibRefNum:
      palmos_gpdlibtrap(trap);
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "trap 0x%04X refNum %d not mapped", trap, refNum);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}

static int palmos_systrap_gen(uint16_t trap) {
  uint32_t sp;
  uint16_t idx;
  int handled = 1;
  uint8_t *ram = pumpkin_heap_base();
  UInt32 res = 0;
  uint16_t cardNo = 0;
  uint32_t cardNameP = 9;
  char *s_cardNameP = NULL;
  uint32_t manufNameP = 0;
  char *s_manufNameP = NULL;
  uint32_t versionP = 0;
  UInt16 l_versionP = 0;
  uint32_t crDateP = 0;
  UInt32 l_crDateP = 0;
  uint32_t romSizeP = 0;
  UInt32 l_romSizeP = 0;
  uint32_t ramSizeP = 0;
  UInt32 l_ramSizeP = 0;
  uint32_t freeBytesP = 0;
  UInt32 l_freeBytesP = 0;
  uint16_t heapIndex = 0;
  uint16_t heapID = 0;
  uint32_t freeP = 0;
  UInt32 l_freeP = 0;
  uint32_t maxP = 0;
  UInt32 l_maxP = 0;
  uint16_t numHandles = 0;
  uint8_t initContents = 0;
  uint16_t index = 0;
  LocalID dbID = 0;
  uint16_t ownerID = 0;
  uint32_t size = 0;
  uint16_t attr = 0;
  MemPtr resm = NULL;
  uint32_t r_res = 0;
  uint32_t chunkDataP = 0;
  char *l_chunkDataP = NULL;
  uint32_t p = 0;
  void *l_p = NULL;
  MemHandle resmem = NULL;
  LocalID resid = 0;
  uint16_t owner = 0;
  uint32_t newSize = 0;
  uint32_t h = 0;
  MemHandle l_h = NULL;
  LocalID local = 0;
  MemPtr resmp = NULL;
  LocalIDKind reslid;
  uint32_t s1 = 0;
  uint32_t s2 = 0;
  int32_t numBytes = 0;
  uint8_t writeAccess = 0;
  uint32_t nameP = 0;
  char *s_nameP = NULL;
  uint32_t creator = 0;
  uint32_t type = 0;
  uint8_t resDB = 0;
  uint32_t bufferP = 0;
  void *l_bufferP = NULL;
  uint32_t attributesP = 0;
  UInt16 l_attributesP = 0;
  uint32_t modDateP = 0;
  UInt32 l_modDateP = 0;
  uint32_t bckUpDateP = 0;
  UInt32 l_bckUpDateP = 0;
  uint32_t modNumP = 0;
  UInt32 l_modNumP = 0;
  uint32_t appInfoIDP = 0;
  LocalID l_appInfoIDP = 0;
  uint32_t sortInfoIDP = 0;
  LocalID l_sortInfoIDP = 0;
  uint32_t typeP = 0;
  UInt32 l_typeP = 0;
  uint32_t creatorP = 0;
  UInt32 l_creatorP = 0;
  uint32_t numRecordsP = 0;
  UInt32 l_numRecordsP = 0;
  uint32_t totalBytesP = 0;
  UInt32 l_totalBytesP = 0;
  uint32_t dataBytesP = 0;
  UInt32 l_dataBytesP = 0;
  uint8_t protect = 0;
  uint16_t mode = 0;
  DmOpenRef resor = NULL;
  char stype[8], screator[8];
  uint32_t dbP = 0;
  DmOpenRef l_dbP = 0;
  DmOpenRef l_currentP = 0;
  uint32_t dbIDP = 0;
  LocalID l_dbIDP = 0;
  uint32_t openCountP = 0;
  UInt16 l_openCountP = 0;
  uint32_t modeP = 0;
  UInt16 l_modeP = 0;
  uint32_t cardNoP = 0;
  UInt16 l_cardNoP = 0;
  uint32_t resDBP = 0;
  Boolean l_resDBP = 0;
  uint32_t dbR = 0;
  DmOpenRef l_dbR = NULL;
  UInt8 l_highest = 0;
  uint32_t count = 0;
  UInt32 l_count = 0;
  uint32_t busy = 0;
  UInt32 l_busy = 0;
  uint32_t category = 0;
  uint32_t attrP = 0;
  UInt16 l_attrP = 0;
  uint32_t uniqueIDP = 0;
  UInt32 l_uniqueIDP = 0;
  uint32_t chunkIDP = 0;
  LocalID l_chunkIDP = 0;
  uint16_t from = 0;
  uint16_t to = 0;
  uint32_t atP = 0;
  UInt16 l_atP = 0;
  uint32_t highest = 0;
  uint32_t uniqueID = 0;
  uint32_t indexP = 0;
  UInt16 l_indexP = 0;
  int16_t direction = 0;
  uint8_t dirty = 0;
  uint16_t toCategory = 0;
  uint16_t fromCategory = 0;
  uint16_t categoryNum = 0;
  uint32_t recordP = 0;
  void *s_recordP = NULL;
  uint32_t offset = 0;
  uint32_t bytes = 0;
  uint32_t srcP = 0;
  void *s_srcP = NULL;
  uint16_t resID = 0;
  char buf[8];
  uint16_t value = 0;
  uint32_t resourceH = 9;
  MemHandle l_resourceH = NULL;
  uint32_t resType = 0;
  uint16_t typeIndex = 0;
  uint32_t resH = 0;
  MemHandle l_resH = NULL;
  uint32_t resTypeP = 0;
  DmResType l_resTypeP;
  uint32_t resIDP = 0;
  DmResID l_resIDP;
  uint32_t chunkLocalIDP = 0;
  LocalID l_chunkLocalIDP;
  uint32_t winHandle = 0;
  WinHandle l_winHandle = NULL;
  WinHandle reswh = NULL;
  uint32_t r = 0;
  RectangleType l_r;
  uint32_t source = 0;
  RectangleType l_source;
  uint32_t error = 0;
  UInt16 l_error = 0;
  int16_t destX = 0;
  int16_t destY = 0;
  uint32_t srcWin = 0;
  WinHandle l_srcWin = NULL;
  uint32_t dstWin = 0;
  WinHandle l_dstWin = NULL;
  uint32_t srcRect = 0;
  RectangleType l_srcRect;
  uint32_t rP = 0;
  RectangleType l_rP;
  int16_t distance = 0;
  uint32_t vacatedP = 0;
  RectangleType l_vacatedP;
  uint32_t winH = 0;
  WinHandle l_winH = NULL;
  uint32_t extentX = 0;
  Coord l_extentX = 0;
  uint32_t extentY = 0;
  Coord l_extentY = 0;
  uint32_t listP = 0;
  ListType *s_listP = NULL;
  int16_t itemCount = 0;
  BitmapType *resbm = NULL;
  FontID resfont = 0;
  uint32_t bitmapP = 0;
  BitmapType *l_bitmapP = NULL;
  Char *resc = NULL;
  uint32_t stream = 0;
  FileHand l_stream = NULL;
  uint32_t dataP = 0;
  void *s_dataP = NULL;
  int32_t objSize = 0;
  int32_t numObj = 0;
  uint32_t errP = 0;
  Err l_errP = 0;
  uint32_t openMode = 0;
  uint8_t newMode = 0;
  uint8_t foreColor = 0;
  IndexedColorType resic;
  uint8_t backColor = 0;
  uint8_t textColor = 0;
  uint32_t newRgbP = 0;
  RGBColorType l_newRgbP;
  uint32_t prevRgbP = 0;
  RGBColorType l_prevRgbP;
  CustomPatternType *s_patternP = NULL;
  PatternType respt;
  uint8_t newPattern = 0;
  uint32_t rgbP = 0;
  RGBColorType l_rgbP;
  uint16_t numPoints = 0;
  uint32_t pts = 0;
  uint32_t i = 0;
  int16_t x1 = 0;
  int16_t y1 = 0;
  int16_t x2 = 0;
  int16_t y2 = 0;
  uint16_t cornerDiam = 0;
  uint16_t theChar = 0;
  uint32_t chars = 0;
  char *s_chars = NULL;
  int16_t len = 0;
  int16_t maxWidth = 0;
  UnderlineModeType resumt;
  uint32_t patternP = 0;
  uint32_t newForeColorP = 0;
  RGBColorType l_newForeColorP;
  uint32_t oldForeColorP = 0;
  RGBColorType l_oldForeColorP;
  uint32_t newBackColorP = 0;
  RGBColorType l_newBackColorP;
  uint32_t oldBackColorP = 0;
  RGBColorType l_oldBackColorP;
  uint8_t compType = 0;
  void *resv = NULL;
  WinDrawOperation resdo;
  UInt8 *res8 = NULL;
  uint32_t dataSizeP = 0;
  UInt32 l_dataSizeP = 0;
  ColorTableType *resctt = NULL;
  uint32_t headerSizeP = 0;
  UInt32 l_headerSizeP = 0;
  uint32_t widthP = 0;
  Coord l_widthP = 0;
  uint32_t heightP = 0;
  Coord l_heightP = 0;
  uint32_t rowBytesP = 0;
  UInt16 l_rowBytesP = 0;
  uint8_t font = 0;
  int8_t ch = 0;
  uint16_t iChar = 0;
  char *s_pChars = NULL;
  int16_t pixelWidth = 0;
  uint32_t leadingEdge = 0;
  Boolean l_leadingEdge = 0;
  uint32_t truncWidth = 0;
  Int16 l_truncWidth = 0;
  uint32_t string = 0;
  char *s_string = NULL;
  uint32_t stringWidthP = 0;
  Int16 l_stringWidthP = 0;
  uint32_t stringLengthP = 0;
  Int16 l_stringLengthP = 0;
  uint32_t fitWithinWidth = 0;
  Boolean l_fitWithinWidth = 0;
  uint32_t linesToScrollP = 0;
  UInt16 l_linesToScrollP = 0;
  uint32_t scrollPosP = 0;
  UInt16 l_scrollPosP = 0;
  uint16_t width = 0;
  uint16_t scrollPos = 0;
  uint32_t linesP = 0;
  UInt16 l_linesP = 0;
  uint32_t topLine = 0;
  UInt16 l_topLine = 0;
  uint32_t dst = 0;
  char *s_dst = NULL;
  uint32_t src = 0;
  char *s_src = NULL;
  FontPtr fontp = NULL;
  uint32_t pChars = 0;
  int32_t n = 0;
  uint16_t length = 0;
  char *s_s1 = NULL;
  char *s_s2 = NULL;
  uint32_t s = 0;
  char *s_s = NULL;
  uint8_t initMode = 0;
  int8_t thousandSeparator = 0;
  int8_t decimalSeparator = 0;
  uint32_t str = 0;
  uint32_t token = 0;
  uint32_t fldP = 0;
  FieldType *s_fldP = NULL;
  char *s_str = NULL;
  uint16_t chr = 0;
  char *s_token = NULL;
  uint32_t rect = 0;
  RectangleType l_rect;
  uint32_t startPosition = 0;
  UInt16 l_startPosition = 0;
  uint32_t endPosition = 0;
  UInt16 l_endPosition = 0;
  uint32_t eventP = 0;
  EventType l_eventP;
  uint8_t redraw = 0;
  FontID fontID = 0;
  uint32_t textHandle = 0;
  MemHandle l_textHandle = NULL;
  uint32_t textP = 0;
  char *s_textP = NULL;
  uint8_t usable = 0;
  uint16_t pos = 0;
  uint32_t textHeightP = 0;
  UInt16 l_textHeightP = 0;
  uint32_t fieldHeightP = 0;
  UInt16 l_fieldHeightP = 0;
  uint16_t linesToScroll = 0;
  uint32_t insertChars = 0;
  char *s_insertChars = NULL;
  uint16_t insertLen = 0;
  uint16_t start = 0;
  uint16_t end = 0;
  UInt16 attrBits = 0;
  FieldAttrType attrFields;
  uint16_t allocatedSize = 0;
  uint8_t maxLines = 0;
  uint32_t formPP = 0;
  uint32_t maxChars = 0;
  uint8_t editable = 0;
  uint8_t underlined = 0;
  uint8_t singleLine = 0;
  uint8_t dynamicSize = 0;
  uint8_t justification = 0;
  uint8_t autoShift = 0;
  uint8_t hasScrollBar = 0;
  uint8_t numeric = 0;
  uint32_t formP = 0;
  void *form = NULL;
  FieldType *fld = NULL;
  uint32_t a = 0;
  uint32_t tableP = 0;
  TableType *s_tableP = NULL;
  uint32_t event = 0;
  EventType l_event;
  int16_t row = 0;
  int16_t column = 0;
  int16_t numLines = 0;
  uint16_t height = 0;
  void *s_value = NULL;
  uint8_t selectable = 0;
  Coord resco;
  uint32_t rowP = 0;
  Int16 l_rowP = 0;
  uint32_t data = 0;
  int16_t spacing = 0;
  uint32_t columnP = 0;
  Int16 l_columnP = 0;
  FieldType *resft = NULL;
  uint8_t editIndicator = 0;
  uint8_t staticHeight = 0;
  uint8_t masked = 0;
  int16_t itemNum = 0;
  int16_t visibleItems = 0;
  uint32_t itemsText = 0;
  char **s_itemsText = NULL;
  int16_t numItems = 0;
  MenuBarType *resmbt = NULL;
  uint32_t menuP = 0;
  MenuBarType *s_menuP = NULL;
  uint8_t where = 0;
  uint16_t bitmapId = 0;
  uint8_t resultType = 0;
  uint32_t result = 0;
  uint16_t resourceId = 0;
  int16_t buttonIndex = 0;
  uint32_t bitmapIdP = 0;
  UInt16 l_bitmapIdP = 0;
  MenuCmdBarResultType l_resultTypeP;
  UInt32 l_resultP = 0;
  uint32_t resultTypeP = 0;
  uint32_t resultP = 0;
  int16_t positionId = 0;
  uint16_t cmd = 0;
  uint32_t x = 0;
  Int16 l_x = 0;
  uint32_t y = 0;
  Int16 l_y = 0;
  uint32_t controlP = 0;
  ControlType *s_controlP = NULL;
  uint16_t newValue = 0;
  uint32_t newLabel = 0;
  char *s_newLabel = NULL;
  uint32_t ctlP = 0;
  ControlType *s_ctlP = NULL;
  int16_t newBitmapID = 0;
  int16_t newSelectedBitmapID = 0;
  uint32_t minValueP = 0;
  UInt16 l_minValueP = 0;
  uint32_t maxValueP = 0;
  UInt16 l_maxValueP = 0;
  UInt16 l_pageSizeP = 0;
  uint32_t pageSizeP = 0;
  uint32_t valueP = 0;
  UInt16 l_valueP = 0;
  EventType l_pEvent;
  uint32_t pEvent = 0;
  FileHand resfh = NULL;
  uint32_t baseP = 0;
  void *s_baseP = NULL;
  uint8_t dataStoreBased = 0;
  uint8_t origin = 0;
  uint32_t fileSizeP = 0;
  Int32 l_fileSizeP = 0;
  uint32_t cmdPBP = 0;
  uint8_t set = 0;
  uint8_t newContrastLevel = 0;
  uint16_t launchFlags = 0;
  launch_union_t param;
  uint8_t newBrightnessLevel = 0;
  uint32_t warnThresholdP = 0;
  UInt16 l_warnThresholdP = 0;
  uint32_t criticalThresholdP = 0;
  UInt16 l_criticalThresholdP = 0;
  uint32_t maxTicksP = 0;
  Int16 l_maxTicksP = 0;
  uint32_t kindP = 0;
  SysBatteryKind l_kindP;
  uint32_t pluggedIn = 0;
  Boolean l_pluggedIn = 0;
  uint32_t percentP = 0;
  UInt8 l_percentP = 0;
  uint32_t initDelayP = 0;
  UInt16 l_initDelayP = 0;
  uint32_t periodP = 0;
  UInt16 l_periodP = 0;
  uint32_t doubleTapDelayP = 0;
  UInt16 l_doubleTapDelayP = 0;
  uint32_t queueAheadP = 0;
  Boolean l_queueAheadP = 0;
  uint32_t db = 0;
  DmOpenRef l_db = NULL;
  uint32_t lst = 0;
  ListType *s_lst = NULL;
  uint16_t currentCategory = 0;
  uint8_t showAll = 0;
  uint8_t showUneditables = 0;
  uint8_t numUneditableCategories = 0;
  uint32_t editingStrID = 0;
  uint8_t resizeList = 0;
  uint32_t name = 0;
  char *s_name = NULL;
  UInt16 l_category = 0;
  uint32_t titleStrID = 0;
  uint32_t frm = 0;
  FormType *s_frm = NULL;
  uint16_t ctlID = 0;
  uint16_t lstID = 0;
  uint8_t title = 0;
  uint32_t categoryP = 0;
  UInt16 l_categoryP = 0;
  uint32_t categoryName = 0;
  char *s_categoryName = NULL;
  uint32_t ctl = 0;
  ControlType *s_ctl = NULL;
  uint32_t appInfoP = 0;
  AppInfoType *l_appInfoP = NULL;
  uint16_t localizedAppInfoStrID = 0;
  uint16_t id = 0;
  UIntPtr resUP = 0;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;

  switch (trap) {
    #include "switch.c"
    default:
      handled = 0;
  }

  return handled;
}

uint32_t palmos_systrap(uint16_t trap) {
  uint32_t sp = 0;
  uint16_t idx = 0, selector = 0;
  char buf[256], buf2[8];
  char *s = NULL;
  Err err = 0;
  emu_state_t *state = m68k_get_emu_state();
  uint32_t r = 0;
  uint32_t appInfoPP = 0;
  uint32_t prevGlobalsP = 0;
  uint32_t globalsPtrP = 0;
  int32_t delay = 0;
  uint32_t nameP = 0;
  uint32_t refNumP = 0;
  char *name = NULL;
  UInt16 refNum = 0;
  uint32_t libType = 0;
  uint32_t libCreator = 0;
  uint32_t type = 0;
  uint32_t creator = 0;
  Boolean exists = 0;
  uint32_t id = 0;
  uint32_t code = 0;
  uint32_t size = 0;
  uint32_t dispatchTblP = 0;
  uint32_t globalsP = 0;
  SysLibTblEntryType tbl;
  uint8_t *p = NULL;
  uint32_t a = 0;
  uint32_t pP = 0;
  uint16_t featureNum = 0;
  uint32_t valueP = 0;
  uint32_t value = 0;
  void *s_valueP = NULL;
  uint8_t op = 0;
  uint32_t stream = 0;
  FileHand l_stream = NULL;
  uint32_t valueLenP = 0;
  Int32 l_valueLenP = 0;
  UInt32 res = 0;
  uint32_t filenameP = 0;
  uint16_t lineNo = 0;
  uint32_t msgP = 0;
  char *filename = NULL;
  char *msg = NULL;
  char *nameBuf = NULL;
  char *logBuf = NULL;
  Int32 logLen = 0;
  uint32_t succSyncDateP = 0;
  uint32_t lastSyncDateP = 0;
  uint32_t syncStateP = 0;
  uint32_t nameBufP = 0;
  uint32_t logBufP = 0;
  uint32_t logLenP = 0;
  UInt32 succSyncDate = 0;
  UInt32 lastSyncDate = 0;
  DlkSyncStateType syncState;
  uint16_t cardNo = 0;
  uint32_t dbID = 0;
  uint32_t iterationData = 0;
  uint16_t enableIt = 0;
  uint16_t rscID = 0;
#ifdef ARMEMU
  uint32_t nativeFuncP = 0;
  uint32_t userDataP = 0;
#endif
  uint32_t startPP = 0;
  uint32_t endPP = 0;
  uint16_t trapNum = 0;
  uint32_t procP = 0;
  uint32_t token = 0;
  uint32_t dataP = 0;
  UInt8 *l_dataP = NULL;
  uint32_t sizeP = 0;
  UInt16 l_sizeP = 0;
  uint32_t panelCountP = 0;
  uint32_t panelIDsP = 0;
  UInt16 panelCount = 0;
  MemHandle panelIDs = NULL;
  uint32_t baseP = 0;
  uint32_t comparF = 0;
  int32_t other = 0;
  uint8_t *base = NULL;
  uint32_t searchF = 0;
  uint32_t searchData = 0;
  uint32_t positionP = 0;
  uint8_t findFirst = 0;
  uint32_t notifyType = 0;
  uint32_t callbackP = 0;
  int32_t priority = 0;
  SysNotifyProcPtr callback = NULL;
  void *userData = NULL;
  SysNotifyParamType notify;
  Boolean resb = 0;
  uint16_t numOfElements = 0;
  int16_t stringCount = 0;
  uint32_t stringP = 0;
  int16_t theID = 0;
  char *string = NULL;
  uint32_t resID = 0;
  uint32_t strP = 0;
  uint16_t maxLen = 0;
  char *str = NULL;
  uint32_t c = 0;
  uint32_t notifyP = 0;
  Char *resc = NULL;
  uint32_t p32 = 0;
  uint32_t newPtrP = 0;
  uint32_t newValue = 0;
  uint32_t hourP = 0;
  uint32_t minP = 0;
  uint32_t titleP = 0;
  Int16 hour = 0;
  Int16 min = 0;
  uint8_t selectDayBy = 0;
  uint32_t monthP = 0;
  uint32_t dayP = 0;
  uint32_t yearP = 0;
  Int16 month = 0, day = 0, year = 0;
  char *title = NULL;
  uint32_t seconds = 0;
  uint32_t dateP = 0;
  uint8_t months = 0;
  uint32_t days = 0;
  uint16_t years = 0;
  uint8_t dateFormat = 0;
  union {
	  UInt16 bits;
	  DateType fields;
  } date;
  int16_t timeZone = 0;
  uint32_t localeP = 0;
  uint32_t templateP = 0;
  uint32_t dateTimeP = 0;
  int32_t adjustment = 0;
  uint8_t hours = 0;
  uint8_t minutes = 0;
  uint8_t timeFormat = 0;
  LmLocaleType locale;
  DateTimeType dateTime;
  int16_t stringLen = 0;
  char *template = NULL;
  uint8_t operation = 0;
  uint32_t widthP = 0;
  uint32_t heightP = 0;
  uint32_t depthP = 0;
  uint32_t enableColorP = 0;
  uint32_t width = 0, height = 0, depth = 0;
  Boolean enableColor = 0;
  int16_t startIndex = 0;
  uint16_t paletteEntries = 0;
  uint32_t tableP = 0;
  uint32_t i = 0;
  RGBColorType table[256];
  uint32_t index = 0;
  uint8_t font = 0;
  uint32_t fontP = 0;
  FontPtr fontp = NULL;
  uint32_t boundsP = 0;
  uint16_t frame = 0;
  uint8_t modal = 0;
  uint8_t focusable = 0;
  uint32_t errorP = 0;
  UInt16 error = 0;
  WinHandle wh = NULL;
  RectangleType bounds;
  uint32_t w = 0;
  BitmapType *bitmap = NULL;
  uint32_t format = 0;
  uint8_t eraseIt = 0;
  RectangleType rect;
  uint32_t rP = 0;
  int16_t left = 0;
  int16_t top = 0;
  int16_t insetAmt = 0;
  int16_t deltaX = 0;
  int16_t deltaY = 0;
  uint32_t dstRectP = 0;
  RectangleType src, dst;
  int16_t x = 0;
  int16_t y = 0;
  RectangleType rect1, rect2, rect3;
  uint32_t r1P = 0;
  uint32_t r2P = 0;
  uint32_t r3P = 0;
  uint32_t colorTableP = 0;
  uint32_t bitmapP = 0;
  uint32_t srcRectP = 0;
  uint8_t fontID = 0;
  uint8_t oldFontID = 0;
  RGBColorType rgb;
  uint8_t which = 0;
  uint32_t rgbP = 0;
  IndexedColorType cct = 0;
  SystemPreferencesType prefs;
  uint8_t choice = 0;
  uint8_t saved = 0;
  DmOpenRef dbRef = NULL;
  int16_t version = 0;
  uint32_t prefsP = 0;
  uint16_t prefsSize = 0;
  uint32_t prefsSizeP = 0;
  uint32_t dstP = 0;
  uint32_t numBytes = 0;
  UInt32 start = 0, end = 0;
  uint32_t sP = 0;
  int32_t newSeed = 0;
  uint32_t dbP = 0;
  uint32_t oldHP = 0;
  MemHandle old = NULL;
  uint32_t ih = 0;
  uint32_t dbPP = 0;
  MemHandle h = NULL;
  Boolean b = 0;
  DmOpenRef dbPOR;
  uint8_t newSearch = 0;
  uint32_t stateInfoP = 0;
  uint8_t onlyLatestVers = 0;
  uint32_t cardNoP = 0;
  uint32_t dbIDP = 0;
  DmSearchStateType stateInfo;
  uint32_t comparP = 0;
  uint32_t newRecordP = 0;
  uint32_t newRecordInfoP = 0;
  uint32_t atP = 0;
  uint32_t newH = 0;
  UInt16 at = 0;
  uint32_t formatStr = 0;
  char *f = NULL;
  int vararg = 0;
  int j = 0, k = 1, t = 0, sz, arglen = 0;
  uint32_t arg = 0, v_arg = 0;
  char *q = NULL, fmt[16];
  uint16_t formID = 0;
  uint32_t titleStrP = 0;
  uint16_t defaultButton = 0;
  uint16_t helpRscID = 0;
  uint16_t menuRscID = 0;
  char *titleStr = NULL;
  FormType *form = NULL;
  uint32_t formP = 0;
  uint32_t objP = 0;
  void *obj = NULL;
  uint16_t updateCode = 0;
  uint32_t newTitleP = 0;
  uint32_t handlerP = 0;
  uint16_t objIndex = 0;
  uint32_t f32 = 0;
  FieldType *fld = NULL;
  uint32_t s32 = 0;
  char *newTitle = NULL;
  uint16_t upIndex = 0;
  uint16_t downIndex = 0;
  uint8_t scrollableUp = 0;
  uint8_t scrollableDown = 0;
  uint32_t gadgetP = 0;
  Coord xc = 0;
  Coord yc = 0;
  uint32_t xP = 0;
  uint32_t yP = 0;
  uint8_t groupNum = 0;
  uint16_t objID = 0;
  uint32_t ptr = 0;
  FormObjectKind objType;
  uint16_t labelID = 0;
  const Char *label = NULL;
  uint16_t fieldIndex = 0;
  uint32_t maxP = 0;
  uint32_t pageSizeP = 0;
  Int16 max = 0, pageSize = 0;
  ScrollBarType *bar = NULL;
  uint32_t barP = 0;
  EventType event;
  uint32_t eventP = 0;
  uint32_t newLabelP = 0;
  uint16_t controlID = 0;
  uint16_t alertId = 0;
  uint32_t s1P = 0;
  uint32_t s2P = 0;
  uint32_t s3P = 0;
  char *s1 = NULL;
  char *s2 = NULL;
  char *s3 = NULL;
  uint32_t entryStringBufP = 0;
  int16_t entryStringBufLength = 0;
  char *entryStringBuf = NULL;
  FormCheckResponseFuncPtr callbackfp;
  uint32_t formPP = 0;
  uint32_t stateP = 0;
  uint8_t save = 0;
  ControlType *ctl = NULL;
  uint8_t style = 0;
  uint32_t textP = 0;
  uint8_t group = 0;
  uint8_t leftAnchor = 0;
  char *text = NULL;
  uint32_t controlP = 0;
  ControlType *control = NULL;
  ControlStyleType stylest = 0;
  ControlType *s_controlP = NULL;
  uint32_t listP = 0;
  uint32_t funcP = 0;
  ListType *list = NULL;
  int16_t column = 0;
  TableType *tablet = NULL;
  FormGadgetType *gadget = NULL;
  FrmGraffitiStateType *gsi = NULL;
  uint32_t r_res = 0;
  uint8_t inPlace = 0;
  FormBitmapType *bitmapf = NULL;
  FormActiveStateType *statef = NULL;
  char *eventName = NULL;
  uint16_t ascii = 0;
  uint16_t keycode = 0;
  uint16_t modifiers = 0;
  int32_t timeout = 0;
  uint32_t sourceP = 0;
  uint32_t destP = 0;
  EventType source, dest;
  uint32_t pScreenX = 0;
  uint32_t pScreenY = 0;
  uint32_t pPenDown = 0;
  Int16 screenX = 0, screenY = 0;
  Boolean penDown = 0;
  uint32_t lengthP = 0;
  UInt16 length = 0;
  uint32_t numP = 0;
  uint32_t countP = 0;
  uint8_t lookupName = 0;
  UInt16 count = 0;
  uint32_t indexP = 0;
  uint32_t tipP = 0;
  uint8_t numberFormat = 0;
  uint32_t thousandSeparatorP = 0;
  uint32_t decimalSeparatorP = 0;
  uint32_t chanP = 0;
  uint8_t cmd = 0;
  uint32_t smfP = 0;
  uint32_t selP = 0;
  uint32_t chanRangeP = 0;
  uint32_t callbacksP = 0;
  uint8_t bNoWait = 0;
  SndSmfOptionsType options;
  uint32_t resType = 0;
  uint8_t volumeSelector = 0;
  uint32_t ptrP = 0;
  void *ptrv = NULL;
  IndexedColorType indexc;
  uint8_t multipleDBs = 0;
  uint32_t wCountP = 0;
  uint32_t entHP = 0;
  UInt16 wCount = 0;
  MemHandle entH = NULL;
  uint8_t beepID = 0;
  uint32_t sndP = 0;
  int32_t volume = 0;
  uint32_t flags = 0;
  uint8_t kbd = 0;
  char *tip = NULL;
  uint32_t channelP = 0;
  uint32_t cmdP = 0;
  uint8_t noWait = 0;
  uint32_t alarmAmpP = 0;
  uint32_t sysAmpP = 0;
  uint32_t masterAmpP = 0;
  UInt16 alarmAmp = 0, sysAmp = 0, masterAmp = 0;
  uint32_t defAmpP = 0;
  UInt16 defAmp = 0;
  uint32_t samplerate = 0;
  uint32_t buffsize = 0;
  uint8_t armNative = 0;
  SndStreamRef *channel = NULL;
  SndStreamBufferCallback func = NULL;
  uint32_t channel32 = 0;
  uint32_t capsLockP = 0;
  uint32_t numLockP = 0;
  uint32_t tempShiftP = 0;
  uint32_t autoShiftedP = 0;
  Boolean capsLock = 0, numLock = 0, autoShifted = 0;
  UInt16 tempShift = 0;
  uint8_t upperShift = 0;
  int16_t paramSize = 0;
  uint32_t bufP = 0;
  uint16_t crc = 0;
  uint16_t lockFlags = 0;
  void *textCallback = NULL;
  ProgressPtr prg = NULL;
  uint32_t prgP = 0;
  uint8_t force = 0;
  uint16_t strLen = 0;
  uint32_t digestP = 0;
  UInt8 *digest = NULL;
  UInt8 *resu8 = NULL;
  uint16_t stage = 0;
  uint32_t messageP = 0;
  uint8_t updateNow = 0;
  char *message = NULL;
  char thousandSeparator = 0, decimalSeparator = 0;
  void *sndPtr = NULL;
  uint8_t mode = 0;
  uint32_t ax = 0;
  uint8_t *e = NULL;
  uint32_t exceptionP = 0;
  uint32_t nextP = 0;
  SndStreamVariableBufferCallback sndfunc = NULL;
  UInt32 ref = 0;
  uint32_t textCallbackP = 0;
  uint32_t alarmSeconds = 0;
  uint8_t quiet = 0;
  uint32_t refP = 0;
  void *bufp = NULL;
  int16_t result = 0;
  uint16_t errCode = 0;
  uint32_t errMsgP = 0;
  uint32_t preMsgP = 0;
  uint32_t postMsgP = 0;
  char *errMsg = NULL;
  char *preMsg = NULL;
  char *postMsg = NULL;
  uint16_t level = 0;
  uint32_t sysP = 0;
  char *sys = NULL;
  uint32_t len = 0;
  uint16_t offset = 0;
  UInt16 *dispatch = NULL;
  uint8_t *addr = NULL;
  uint32_t num = 0;

  // typedef struct ErrExceptionType {
  //   struct ErrExceptionType *nextP;  // next exception type
  //   ErrJumpBuf state;                // setjmp/longjmp storage
  //   Int32 err;                       // Error code
  // } ErrExceptionType;
  nextP = m68k_read_memory_32(exceptionP);
  m68k_write_memory_32(a, nextP);
  bufP = exceptionP + 4;

  // MathLib seems to use trap numbers like 0x0306 instead of 0xA306.
  trap = (trap & 0x0FFF) | 0xA000;
  s = logtrap_trapname(state->lt, trap, &selector, 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "trap 0x%04X begin (%s) pc=0x%08X", trap, s ? s : "unknown", m68k_get_reg(NULL, M68K_REG_PC));

  if (palmos_systrap_gen(trap)) {
    debug(DEBUG_TRACE, "EmuPalmOS", "trap 0x%04X end (gen)", trap);
    pumpkin_trace(trap);
    return 0;
  }

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;

  switch (trap) {
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
      dbID = (uint32_t)id;
      err = SysLibRegister68K(refNum, (LocalID)dbID, emupalmos_trap_in(code, trap, 2), size, emupalmos_trap_in(dispatchTblP, trap, 4), emupalmos_trap_in(globalsP, trap, 5));
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
      // UInt32 PceNativeCall(NativeFuncType *nativeFuncP, void *userDataP)
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
      SysQSort68k(base, numOfElements, (uint16_t)width, comparF, other);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysQSort68k(0x%08X, %d, %d, 0x%08X, %d)", baseP, numOfElements, width, comparF, other);
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
      debug(DEBUG_TRACE, "EmuPalmOS", "SysBinarySearch(0x%08X, %d, %d, 0x%08X, 0x%08X, %d, 0x%08X, %d) native 0x%08X", baseP, numOfElements, width, searchF, searchData, other, positionP, findFirst, state->SysQSort_addr);
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
      // Err SysNotifyRegister(UInt16 cardNo, LocalID dbID, UInt32 notifyType, SysNotifyProcPtr callbackP, Int8 priority, void *userDataP)
      cardNo = ARG16;
      dbID = ARG32;
      notifyType = ARG32;
      callbackP = ARG32;
      priority = ARG8;
      userDataP = ARG32;
      callback = emupalmos_trap_in(callbackP, trap, 3);
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
      // void SysCopyStringResource(Char *string, Int16 theID)
      stringP = ARG32;
      theID = ARG16;
	  string = emupalmos_trap_in(stringP, trap, 0);
      SysCopyStringResource(string, theID);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysCopyStringResource(0x%08X, %d)", stringP, theID);
      }
      break;
    case sysTrapSysStringByIndex: {
      // Char *SysStringByIndex(UInt16 resID, index, Char *strP, UInt16 maxLen)
      resID = ARG16;
      index = ARG16;
      strP = ARG32;
      maxLen = ARG16;
      str = emupalmos_trap_in(strP, trap, 2);
      resc = SysStringByIndex((uint16_t)resID, (uint16_t)index, strP ? str : NULL, maxLen);
      p32 = emupalmos_trap_out(resc);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysStringByIndex(%d, %d, 0x%08X, %d): 0x%08X", resID, index, strP, maxLen, p32);
      m68k_set_reg(M68K_REG_A0, p32);
      }
      break;
    case sysTrapSysReset:
      // void SysReset(void)
      SysReset();
      debug(DEBUG_TRACE, "EmuPalmOS", "SysReset()");
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
      nameBuf = emupalmos_trap_in(nameBufP, trap, 3);
      logBuf = emupalmos_trap_in(logBufP, trap, 4);
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
      // Boolean SelectOneTime(Int16 *hour, Int16 *minute, const Char *titleP)
      hourP = ARG32;
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
      // Boolean SelectDay(const SelectDayType selectDayBy, Int16 *month, Int16 *day, Int16 *year, const Char *title)
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
	  m68k_set_reg(M68K_REG_D0, (Int16)res);
      }
      break;
    case sysTrapDayOfWeek: {
      // Int16 DayOfWeek(Int16 month, Int16 day, Int16 year)
      month = ARG16;
      day = ARG16;
      year = ARG16;
      res = DayOfWeek(month, day, year);
      debug(DEBUG_TRACE, "EmuPalmOS", "DayOfWeek(%d, %d, %d): %d", month, day, year, res);
	  m68k_set_reg(M68K_REG_D0, (Int16)res);
      }
      break;
    case sysTrapDateSecondsToDate: {
      // void DateSecondsToDate(UInt32 seconds, DateType *dateP)
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
      debug(DEBUG_TRACE, "EmuPalmOS", "DateToDOWDMFormat(%u, %u, %u, %u, 0x%08X)", months, days, years, dateFormat, stringP);
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
      debug(DEBUG_TRACE, "EmuPalmOS", "DateToAscii(%u, %u, %u, %u, 0x%08X)", months, days, years, dateFormat, stringP);
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
      uint32_t days = ARG32;
      dateP = ARG32;
      emupalmos_trap_in(dateP, trap, 1);
      DateDaysToDate(days, dateP ? &date.fields : NULL);
      if (dateP) m68k_write_memory_16(dateP, date.bits);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateDaysToDate(%u, 0x%08X)", days, dateP);
      }
      break;
    case sysTrapTimSetSeconds: {
      // void TimSetSeconds(UInt32 seconds)
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
      // void TimeZoneToAscii(Int16 timeZone, const LmLocaleType *localeP, Char *string)
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
      // UInt16 DateTemplateToAscii(const Char *templateP, UInt8 months, UInt8 days, UInt16 years, Char *stringP, Int16 stringLen)
      templateP = ARG32;
      months = ARG8;
      days = ARG8;
      years = ARG16;
      stringP = ARG32;
      stringLen = ARG16;
      template = (char *)emupalmos_trap_in(templateP, trap, 0);
      string = (char *)emupalmos_trap_in(stringP, trap, 4);
      res = DateTemplateToAscii(template, months, (uint8_t)days, years, string, stringLen);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateTemplateToAscii(0x%08X, %u, %u, %u, 0x%08X \"%s\", %d): %u", templateP, months, days, years, stringP, string ? string : "", stringLen, res);
	  m68k_set_reg(M68K_REG_D0, (UInt16)res);
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
      // void TimSecondsToDateTime(UInt32 seconds, DateTimeType *dateTimeP)
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
      UInt32 t = TimGetSeconds();
      debug(DEBUG_TRACE, "EmuPalmOS", "TimGetSeconds(): %u", t);
      m68k_set_reg(M68K_REG_D0, t);
      }
      break;
    case sysTrapTimGetTicks: {
      // UInt32 TimGetTicks(void)
      UInt32 t = TimGetTicks();
      debug(DEBUG_TRACE, "EmuPalmOS", "TimGetTicks(): %u", t);
      m68k_set_reg(M68K_REG_D0, t);
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
    case sysTrapFlpDispatch:
      palmos_flptrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapFlpEmDispatch:
      palmos_flpemtrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapIntlDispatch:
      palmos_intltrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapFileSystemDispatch:
      palmos_filesystemtrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapSerialDispatch:
      palmos_serialtrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapHighDensityDispatch:
      if (pumpkin_get_density() == kDensityDouble) {
        palmos_highdensitytrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      } else {
        emupalmos_panic("high density trap called on low density system", EMUPALMOS_INVALID_TRAP);
      }
      break;
    case sysTrapOmDispatch:
      palmos_omtrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapPinsDispatch:
      palmos_pinstrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapAccessorDispatch:
      palmos_accessortrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapExpansionDispatch:
      palmos_expansiontrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapTsmDispatch:
      palmos_tsmtrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapLmDispatch:
      palmos_lmtrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapNavSelector:
      selector = ARG16;
      palmos_navtrap(sp, idx, selector);
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
            index = m68k_read_memory_8(tableP + i*4);
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
      // Err FntDefineFont(FontID font, FontPtr fontP)
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
      boundsP = ARG32;
      frame = ARG16;
      modal = ARG8;
      focusable = ARG8;
      errorP = ARG32;
      emupalmos_trap_in(boundsP, trap, 0);
      emupalmos_trap_in(errorP, trap, 4);
      decode_rectangle(boundsP, &bounds);
      wh = WinCreateWindow(boundsP ? &bounds : NULL, frame, modal, focusable, errorP ? &error : NULL);
      encode_rectangle(boundsP, &bounds);
      if (errorP) m68k_write_memory_16(errorP, error);
      w = emupalmos_trap_out(wh);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinCreateWindow([%d,%d,%d,%d], %d, %d, %d, 0x%08X [%d]): 0x%08X", bounds.topLeft.x, bounds.topLeft.y, bounds.extent.x, bounds.extent.y, frame, modal, focusable, errorP, error, w);
      m68k_set_reg(M68K_REG_A0, w);
      }
      break;
    case sysTrapWinCreateBitmapWindow: {
      // WinHandle WinCreateBitmapWindow(BitmapType *bitmapP, UInt16 *error)
      bitmapP = ARG32;
      errorP = ARG32;

      bitmap = (BitmapType *)emupalmos_trap_in(bitmapP, trap, 0);
      wh = WinCreateBitmapWindow(bitmap, &error);
      if (errorP) m68k_write_memory_16(errorP, error);
      w = emupalmos_trap_out(wh);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinCreateBitmapWindow(0x%08X, 0x%08X [%d]): 0x%08X", bitmapP, errorP, error, w);
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

      wh = WinCreateOffscreenWindow((uint16_t)width, (uint16_t)height, (uint8_t)format, errorP ? &error : NULL);
      if (errorP) m68k_write_memory_16(errorP, error);
      w = emupalmos_trap_out(wh);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinCreateOffscreenWindow(%d, %d, %d, 0x%08X [%d]): 0x%08X", width, height, format, errorP, error, w);
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

	  RctSetRectangle(rP ? &rect : NULL, left, top, (uint16_t)width, (uint16_t)height);
      encode_rectangle(rP, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "RctSetRectangle(0x%08X [%d,%d,%d,%d], %d, %d, %d, %d)", rP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y, left, top, width, height);
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
      resb = RctPtInRectangle(x, y, rP ? &rect : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "RctPtInRectangle(%d, %d, 0x%08X [%d,%d,%d,%d]): %d", x, y, rP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y, resb);
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

	  bitmap = BmpCreate((uint16_t)width, (uint16_t)height, (uint8_t)depth, (ColorTableType *)emupalmos_trap_in(colorTableP, trap, 3), errorP ? &error : NULL);
      a = emupalmos_trap_out(bitmap);
      if (errorP) m68k_write_memory_16(errorP, error);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpCreate(width=%d, height=%d, depth=%d, colorTableP=0x%08X, error=0x%08X [%d]): 0x%08X", width, height, depth, colorTableP, errorP, error, a);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case sysTrapBmpDelete: {
      // Err BmpDelete(BitmapType *bitmapP)
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
      privateRecordViewEnum r = SecSelectViewStatus();
      debug(DEBUG_TRACE, "EmuPalmOS", "SecSelectViewStatus(): %d", r);
      m68k_set_reg(M68K_REG_D0, r);
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
      // void PrefSetAppPreferences(UInt32 creator, UInt16 id, Int16 version, const void *prefs, UInt16 prefsSize, Boolean saved)
      creator = ARG32;
      id = ARG16;
      version = ARG16;
      prefsP = ARG32;
      prefsSize = ARG16;
      saved = ARG8;
      PrefSetAppPreferences(creator, (uint16_t)id, version, emupalmos_trap_in(prefsP, trap, 3), prefsSize, saved);
      pumpkin_id2s(creator, buf);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefSetAppPreferences('%s', %d, %d, 0x%08X, %d, %d)", buf, id, version, prefsP, prefsSize, saved);
      }
      break;
    case sysTrapPrefSetAppPreferencesV10: {
      // void PrefSetAppPreferencesV10(UInt32 creator, Int16 version, void *prefs, UInt16 prefsSize)
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
      version = PrefGetAppPreferences(creator, (uint16_t)id, emupalmos_trap_in(prefsP, trap, 2), prefsSizeP ? &prefsSize : NULL, saved);
      pumpkin_id2s(creator, buf);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetAppPreferences('%s', %d, 0x%08X, 0x%08X, %d): %d", buf, id, prefsP, prefsSizeP, saved, version);
      if (prefsSizeP) m68k_write_memory_16(prefsSizeP, prefsSize);
      m68k_set_reg(M68K_REG_D0, version);
      }
      break;
    case sysTrapPrefGetAppPreferencesV10: {
      // Boolean PrefGetAppPreferencesV10(UInt32 type, Int16 version, void *prefs, UInt16 prefsSize)
      type = ARG32;
	  version = ARG16;
      prefsP = ARG32;
      prefsSize = ARG16;
      b = PrefGetAppPreferencesV10(type, version, emupalmos_trap_in(prefsP, trap, 2), prefsSize);
      pumpkin_id2s(type, buf);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetAppPreferencesV10('%s', %d, 0x%08X, %d): %d", buf, version, prefsP, prefsSize, b);
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
        debug(DEBUG_TRACE, "EmuPalmOS", "MemSet(0x%08X, %d, 0x%02X) inside screen", dstP, numBytes, value);
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
      debug(DEBUG_TRACE, "EmuPalmOS", "MemSet(0x%08X, %d, 0x%02X): %d", dstP, numBytes, value, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapMemMove: {
      // Err MemMove(void *dstP, const void *sP, Int32 numBytes)
      dstP = ARG32;
      sP = ARG32;
      numBytes = ARG32;
      WinLegacyGetAddr(&start, &end);
      if ((dstP >= start && dstP < end) ||
          (dstP+numBytes-1 >= start && dstP+numBytes-1 < end) ||
          (dstP < start && dstP+numBytes >= end) ||
          (sP >= start && sP < end) ||
          (sP+numBytes-1 >= start && sP+numBytes-1 < end) ||
          (sP < start && sP+numBytes >= end)) {
        debug(DEBUG_TRACE, "EmuPalmOS", "MemMove(0x%08X, 0x%08X, %d) inside screen", dstP, sP, numBytes);
        for (i = 0; i < numBytes; i++) {
          uint8_t value = m68k_read_memory_8(sP+i);
          m68k_write_memory_8(dstP+i, value);
        }
        err = 0;
      } else {
        if (emupalmos_check_address(dstP, numBytes, 0) && emupalmos_check_address(sP, numBytes, 1)) {
          err = MemMove(emupalmos_trap_in(dstP, trap, 0), emupalmos_trap_in(sP, trap, 1), numBytes);
        } else {
          err = dmErrInvalidParam;
        }
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "MemMove(0x%08X, 0x%08X, %d): %d", dstP, sP, numBytes, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapSysRandom: {
      // Int16 SysRandom(Int32 newSeed)
      newSeed = ARG32;
      res = SysRandom(newSeed);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysRandom(%d): %d", newSeed, res);
	  m68k_set_reg(M68K_REG_D0, (Int16)res);
      }
      break;
    case sysTrapDmDetachRecord: {
      // Err DmDetachRecord(DmOpenRef dbP, index, MemHandle *oldHP)
      dbP = ARG32;
      index = ARG16;
      oldHP = ARG32;
      emupalmos_trap_in(oldHP, trap, 2);
      dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
	  res = DmDetachRecord(dbRef, (uint16_t)index, oldHP ? &old : NULL);
      if (oldHP) m68k_write_memory_32(oldHP, emupalmos_trap_out(old));
      debug(DEBUG_TRACE, "EmuPalmOS", "DmDetachRecord(0x%08X, %d, 0x%08X): %d", dbP, index, oldHP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapDmDetachResource: {
      // Err DmDetachResource(DmOpenRef dbP, index, MemHandle *oldHP)
      dbP = ARG32;
      index = ARG16;
      oldHP = ARG32;
      emupalmos_trap_in(oldHP, trap, 2);
      dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
	  res = DmDetachResource(dbRef, (uint16_t)index, oldHP ? &old : NULL);
      if (oldHP) m68k_write_memory_32(oldHP, emupalmos_trap_out(old));
      debug(DEBUG_TRACE, "EmuPalmOS", "DmDetachsource(0x%08X, %d, 0x%08X): %d", dbP, index, oldHP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapDmSearchResource: {
      // UInt16 DmSearchResource(DmResType resType, DmResID resID, MemHandle resH, DmOpenRef *dbPP)
      type = ARG32;
      resID = ARG16;
      ih = ARG32;
      dbPP = ARG32;
      h = emupalmos_trap_in(ih, trap, 2);
      emupalmos_trap_in(dbPP, trap, 3);
	  index = DmSearchResource(type, (uint16_t)resID, h, dbPP ? &dbPOR : NULL);
      if (dbPP) m68k_write_memory_32(dbPP, emupalmos_trap_out(dbPOR));
      pumpkin_id2s(type, buf);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmSearchResource('%s', %d, 0x%08X, 0x%08X): %d", buf, resID, ih, dbPP, index);
      m68k_set_reg(M68K_REG_D0, index);
      }
      break;
    case sysTrapMemHandleLock: {
      // MemPtr MemHandleLock(MemHandle h)
      ih = ARG32;
      h = emupalmos_trap_in(ih, trap, 0);
      p = MemHandleLock(h);
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
      dbID = 0;
      if (stateInfoP && !newSearch) {
        uint32_t info = m68k_read_memory_32(stateInfoP);
        stateInfo.p = emupalmos_trap_in(info, trap, -1);
      }
      err = DmGetNextDatabaseByTypeCreator(newSearch, stateInfoP ? &stateInfo : NULL, type, creator, onlyLatestVers, cardNoP ? &cardNo : NULL, dbIDP ? &dbID : NULL);
      if (stateInfoP) {
        uint32_t info = emupalmos_trap_out(stateInfo.p);
        m68k_write_memory_32(stateInfoP, info);
      }
      if (cardNoP) m68k_write_memory_16(cardNoP, cardNo);
      if (dbIDP) m68k_write_memory_32(dbIDP, dbID);
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
      debug(DEBUG_TRACE, "EmuPalmOS", "DmInsertionSort(0x%08X, 0x%08X, %d): %d", dbP, comparP, other, res);
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
      debug(DEBUG_TRACE, "EmuPalmOS", "DmQuickSort(0x%08X, 0x%08X, %d): %d", dbP, comparP, other, res);
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
	  res = DmFindSortPosition68K(dbRef, newRecordP, newRecordInfoP, comparP, (int16_t)other);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmFindSortPosition(0x%08X, 0x%08X, 0x%08X, 0x%08X, %d): %d", dbP, newRecordP, newRecordInfoP, comparP, other, res);
	  m68k_set_reg(M68K_REG_D0, (UInt16)res);
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
      h = emupalmos_trap_in(newH, trap, 2);
      emupalmos_trap_in(oldHP, trap, 3);

      res = DmAttachRecord(dbRef, atP ? &at : NULL, h, oldHP ? &old : NULL);
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
      // Err DmSyncDatabase(DmOpenRef dbRef)
      dbP = ARG32;
      dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
      res = DmSyncDatabase(dbRef);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmSyncDatabase(0x%08X): %d", dbP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapStrVPrintF:
    case sysTrapStrPrintF: {
      // Int16 StrVPrintF(Char *s, const Char *formatStr, _Palm_va_list arg)
      // Int16 StrPrintF(Char *s, const Char *formatStr, ...)
      strP = ARG32;
      formatStr = ARG32;
      s = emupalmos_trap_in(strP, trap, 0);
      f = emupalmos_trap_in(formatStr, trap, 1);
      vararg = trap == sysTrapStrVPrintF;
      res = 0;
      if (s && f) {
	    j = 0; k = 1; t = 0; sz = 0; arglen = 0;
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
                  p += sys_strlen((const char *)p);
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
        res = (UInt32)sys_strlen(s);
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "StrPrintF(0x%08X \"%s\", 0x%08X \"%s\", ...): %d", strP, str, formatStr, s ? s : "", f ? res : 0);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmNewForm: {
      // FormType *FrmNewForm(UInt16 formID, const Char *titleStrP, Coord x, Coord y, Coord width, Coord height, Boolean modal, UInt16 defaultButton, UInt16 helpRscID, UInt16 menuRscID)
      formID = ARG16;
      titleStrP = ARG32;
      x = ARG16;
      y = ARG16;
      width= ARG16;
      height = ARG16;
      modal = ARG8;
      defaultButton = ARG16;
      helpRscID = ARG16;
      menuRscID = ARG16;
      titleStr = (char *)emupalmos_trap_in(titleStrP, trap, 1);
	  form = FrmNewForm(formID, titleStr, x, y, (uint16_t)width, (uint16_t)height, modal, defaultButton, helpRscID, menuRscID);
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
      // void FrmDeleteForm(FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmDeleteForm(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDeleteForm(0x%08X)", formP);
      }
      break;
    case sysTrapFrmGetFormId: {
      // UInt16 FrmGetformID(const FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      id = FrmGetFormId(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetformID(0x%08X): %d", formP, id);
      m68k_set_reg(M68K_REG_D0, id);
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
      // UInt16 FrmGetObjectIndexFromPtr(const FormType *formP, void *objP)
      formP = ARG32;
      objP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      obj = emupalmos_trap_in(objP, trap, 1);
      res = FrmGetObjectIndexFromPtr(form, obj);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectIndexFromPtr(0x%08X, 0x%08X): %d", formP, objP, res);
      m68k_set_reg(M68K_REG_D0, res);
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
      // void FrmDrawForm(FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDrawForm begin");
      FrmDrawForm(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDrawForm(0x%08X)", formP);

/*
      if (form) {
        int hasNativeHandler = 0;
        for (uint16_t objIndex = 0;; objIndex++) {
          void *obj = FrmGetObjectPtr(form, objIndex);
          if (obj == NULL) break;
          objType = FrmGetObjectType(form, objIndex);
          if (objType == frmGadgetObj) {
            FormGadgetType *gadget = (FormGadgetType *)obj;
            if (gadget->m68k_handler) {
              hasNativeHandler = 1;
              break;
            }
          }
        }
        if (hasNativeHandler) {
          debug(DEBUG_TRACE, "EmuPalmOS", "FrmDrawForm(0x%08X) native", formP);
          r = FrmDrawForm_addr;
        }
      }
*/
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDrawForm end");
      }
      break;
    case sysTrapFrmEraseForm: {
      // void FrmEraseForm(FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmEraseForm(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmEraseForm(0x%08X)", formP);
      }
      break;
    case sysTrapFrmVisible: {
      // Boolean FrmVisible(const FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      resb = FrmVisible(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmVisible(0x%08X): %d", formP, resb);
      m68k_set_reg(M68K_REG_D0, resb);
      }
      break;
    case sysTrapFrmHideObject: {
      // void FrmHideObject(FormType *formP, UInt16 objIndex)
      formP = ARG32;
      index = ARG16;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
	  FrmHideObject(form, (uint16_t)index);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmHideObject(0x%08X, %d)", formP, index);
      // XXX must handle 68K code because of gadget handler
      }
      break;
    case sysTrapFrmShowObject: {
      // void FrmShowObject(FormType *formP, UInt16 objIndex)
      formP = ARG32;
      index = ARG16;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
	  FrmShowObject(form, (uint16_t)index);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmShowObject(0x%08X, %d)", formP, index);
      // XXX must handle 68K code because of gadget handler
      }
      break;
    case sysTrapFrmGetFocus: {
      // UInt16 FrmGetFocus(const FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      res = FrmGetFocus(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFocus(0x%08X): %d", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmSetMenu: {
      // void FrmSetMenu(FormType *formP, UInt16 menuRscID)
      formP = ARG32;
      menuRscID = ARG16;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetMenu(form, menuRscID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetMenu(0x%08X, %d)", formP, menuRscID);
      }
      break;
    case sysTrapFrmGetTitle: {
      // const Char *FrmGetTitle(const FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      title = (char *)FrmGetTitle(form);
      s32 = emupalmos_trap_out(title);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetTitle(0x%08X): 0x%08X \"%s\"", formP, s32, title ? title : "");
      m68k_set_reg(M68K_REG_A0, s32);
      }
      break;
    case sysTrapFrmCopyTitle: {
      // void FrmCopyTitle(FormType *formP, const Char *newTitle)
      formP = ARG32;
      newTitleP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      newTitle = (char *)emupalmos_trap_in(newTitleP, trap, 1);
      FrmCopyTitle(form, newTitle);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmCopyTitle(0x%08X, 0x%08X \"%s\")", formP, newTitleP, newTitle ? newTitle : "");
      }
      break;
    case sysTrapFrmSetTitle: {
      // void FrmSetTitle(FormType *formP, Char *newTitle)
      formP = ARG32;
      newTitleP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      newTitle = (char *)emupalmos_trap_in(newTitleP, trap, 1);
      FrmSetTitle(form, newTitle);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetTitle(0x%08X, 0x%08X \"%s\")", formP, newTitleP, newTitle ? newTitle : "");
      }
      break;
    case sysTrapFrmUpdateScrollers: {
      // void FrmUpdateScrollers(FormType *formP, UInt16 upIndex, UInt16 downIndex, Boolean scrollableUp, Boolean scrollableDown)
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
      // void FrmSetActiveForm(FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetActiveForm(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetActiveForm(0x%08X)", formP);
      }
      break;
    case sysTrapFrmSetEventHandler: {
      // void FrmSetEventHandler(FormType *formP, FormEventHandlerType *handler)
      formP = ARG32;
      handlerP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      if (form) form->m68k_handler = handlerP;
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetEventHandler(0x%08X, 0x%08X)", formP, handlerP);
      }
      break;
    case sysTrapFrmGetEventHandler68K: { // custom trap created for use in 68K code
      // FormEventHandlerType *FrmGetEventHandler68K(FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      handlerP = form ? form->m68k_handler : 0;
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetEventHandler68K(0x%08X): 0x%08X", formP, handlerP);
      m68k_set_reg(M68K_REG_A0, handlerP);
      }
      break;
    case sysTrapFrmSetGadgetHandler: {
      // void FrmSetGadgetHandler(FormType *formP, UInt16 objIndex, FormGadgetHandlerType *attrP)
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
      // void *FrmGetGadgetData(const FormType *formP, UInt16 objIndex)
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
      // void FrmSetGadgetData(FormType *formP, UInt16 objIndex, const void *data)
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
      // FormGadgetTypeInCallback *FrmGetGadgetPtr68k(FormType *formP, UInt16 objIndex)
      formP = ARG32;
      objIndex = ARG16;
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
      // WinHandle FrmGetWindowHandle(const FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      wh = FrmGetWindowHandle(form);
      w = emupalmos_trap_out(wh);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetWindowHandle(0x%08X): 0x%08X", formP, w);
      m68k_set_reg(M68K_REG_A0, w);
      }
      break;
    case sysTrapFrmGetFormBounds: {
      // void FrmGetFormBounds(const FormType *formP, RectangleType *rP)
      formP = ARG32;
      rP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);

      FrmGetFormBounds(form, &rect);
      encode_rectangle(rP, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFormBounds(0x%08X, 0x%08X)", formP, rP);
      }
      break;
    case sysTrapFrmSetObjectBounds: {
      // void FrmSetObjectBounds(FormType *formP, UInt16 objIndex, const RectangleType *bounds)
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
      // UInt16 FrmGetNumberOfObjects(const FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      res = FrmGetNumberOfObjects(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetNumberOfObjects(0x%08X): %d", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmSetObjectPosition: {
      // void FrmSetObjectPosition(FormType *formP, UInt16 objIndex, Coord x, Coord y)
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
      // UInt16 FrmGetObjectId(const FormType *formP, UInt16 objIndex)
      formP = ARG32;
      objIndex = ARG16;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      res = FrmGetObjectId(form, objIndex);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectId(0x%08X, %d): %d", formP, objIndex, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmGetObjectPosition: {
      // void FrmGetObjectPosition(const FormType *formP, UInt16 objIndex, Coord *x, Coord *y)
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
      // void FrmGetObjectBounds(const FormType *formP, UInt16 objIndex, RectangleType *rP)
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
      // UInt16 FrmGetControlGroupSelection(const FormType *formP, UInt8 groupNum)
      formP = ARG32;
      groupNum = ARG8;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      res = FrmGetControlGroupSelection(form, groupNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetControlGroupSelection(0x%08X, %u): %u", formP, groupNum, res);
      m68k_set_reg(M68K_REG_D0, res);
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
      // UInt16 FrmGetActiveformID(void)
      id = FrmGetActiveFormID();
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetActiveformID(): %d", id);
      m68k_set_reg(M68K_REG_D0, id);
      }
      break;
    case sysTrapFrmGetObjectIndex: {
      // UInt16 FrmGetObjectIndex(const FormType *formP, UInt16 objID)
      formP = ARG32;
      objID = ARG16;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      index = FrmGetObjectIndex(form, objID);
	  debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectIndex(0x%08X, %d): %d", formP, objID, index);
	  m68k_set_reg(M68K_REG_D0, (UInt16)index);
      }
      break;
    case sysTrapFrmGetObjectPtr: {
      // void *FrmGetObjectPtr(const FormType *formP, UInt16 objIndex)
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
      // FormObjectKind FrmGetObjectType(const FormType *formP, UInt16 objIndex)
      formP = ARG32;
      objIndex = ARG16;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      objType = FrmGetObjectType(form, objIndex);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectType(0x%08X, %d): %d", formP, objIndex, objType);
      m68k_set_reg(M68K_REG_D0, objType);
      }
      break;
    case sysTrapFrmGetLabel: {
      // const Char *FrmGetLabel(const FormType *formP, UInt16 labelID)
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
      // void FrmSetFocus(FormType *formP, UInt16 fieldIndex)
      formP = ARG32;
      fieldIndex = ARG16;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetFocus(form, fieldIndex);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetFocus(0x%08X, %d)", formP, fieldIndex);
      }
      break;
    case sysTrapFrmGetControlValue: {
      // Int16 FrmGetControlValue(const FormType *formP, UInt16 objIndex)
      formP = ARG32;
      objIndex = ARG16;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      value = FrmGetControlValue(form, objIndex);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetControlValue(0x%08X, %d): %d", formP, objIndex, value);
	  m68k_set_reg(M68K_REG_D0, (Int16)value);
      }
      break;
    case sysTrapFrmSetControlValue: {
      // void FrmSetControlValue(const FormType *formP, UInt16 objIndex, Int16 newValue)
      formP = ARG32;
      objIndex = ARG16;
      newValue = ARG16;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetControlValue(form, objIndex, newValue);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetControlValue(0x%08X, %d, %d)", formP, objIndex, newValue);
      }
      break;
    case sysTrapFrmSetControlGroupSelection: {
      // void FrmSetControlGroupSelection(const FormType *formP, UInt8 groupNum, UInt16 controlID)
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

      if (eventP) decode_event(eventP, &event);
      resb = FrmDispatchEvent(&event);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDispatchEvent(0x%08X): %d", eventP, resb);
      m68k_set_reg(M68K_REG_D0, resb);
      }
      break;
    case sysTrapFrmHandleEvent: {
      // Boolean FrmHandleEvent(FormType *formP, EventType *eventP)
      formP = ARG32;
      eventP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);

      if (eventP) decode_event(eventP, &event);
      resb = FrmHandleEvent(form, &event);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmHandleEvent(0x%08X, 0x%08X): %d", formP, eventP, resb);
      m68k_set_reg(M68K_REG_D0, resb);
      }
      break;
    case sysTrapFrmCopyLabel: {
      // void FrmCopyLabel(FormType *formP, UInt16 labelID, const Char *newLabel)
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
      // UInt16 FrmDoDialog(FormType *formP)
      formP = ARG32;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      res = FrmDoDialog(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDoDialog(0x%08X): %d", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
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
      uint16_t helpMsgId = ARG16;
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
      // UInt16 FrmCustomAlert(UInt16 alertId, const Char *s1, const Char *s2, const Char *s3)
      alertId = ARG16;
      s1P = ARG32;
      s2P = ARG32;
      s3P = ARG32;
      s1 = (char *)emupalmos_trap_in(s1P, trap, 1);
      s2 = (char *)emupalmos_trap_in(s2P, trap, 2);
      s3 = (char *)emupalmos_trap_in(s3P, trap, 3);
      res = FrmCustomAlert(alertId, s1, s2, s3);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmCustomAlert(%d, 0x%08X, 0x%08X, 0x%08X): %d", alertId, s1P, s2P, s3P, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmCustomResponseAlert: {
      // UInt16 FrmCustomResponseAlert(UInt16 alertId, const Char *s1, const Char *s2, const Char *s3, Char *entryStringBuf, Int16 entryStringBufLength, FormCheckResponseFuncPtr callback)
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
      res = FrmCustomResponseAlert(alertId, s1, s2, s3, entryStringBuf, entryStringBufLength, callbackfp);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmCustomResponseAlert(%d, 0x%08X, 0x%08X, 0x%08X, 0x%08X, %d, 0x%08X): %d", alertId, s1P, s2P, s3P, entryStringBufP, entryStringBufLength, callbackP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmAlert: {
      // UInt16 FrmAlert(UInt16 alertId)
      alertId = ARG16;
      res = FrmAlert(alertId);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmAlert(%d): %d", alertId, res);
      m68k_set_reg(M68K_REG_D0, res);
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
	  bitmapf = FrmNewBitmap(&form, (uint16_t)id, rscID, x, y);
      a = emupalmos_trap_out(bitmapf);
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
      width= ARG16;
      height = ARG16;
      formP = formPP ? m68k_read_memory_32(formPP) : 0;
      form = (FormType *)emupalmos_trap_in(formP, trap, 0);
	  gadget = FrmNewGadget(&form, (uint16_t)id, x, y, (uint16_t)width, (uint16_t)height);
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
      gsi = FrmNewGsi(&form, x, y);
      a = emupalmos_trap_out(gsi);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmNewGsi(0x%08X, %d, %d): 0x%08X", formPP, x, y, a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;
    case sysTrapCtlNewControl: {
      // ControlType *CtlNewControl(void **formPP, UInt16 ID, ControlStyleType style, const Char *textP, Coord x, Coord y, Coord width, Coord height, FontID font, UInt8 group, Boolean leftAnchor)
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
      form = emupalmos_trap_in(formP, trap, 0);
      text = (char *)emupalmos_trap_in(textP, trap, 3);
	  ctl = CtlNewControl((void **)&form, (uint16_t)id, style, text, x, y, (uint16_t)width, (uint16_t)height, font, group, leftAnchor);
      a = emupalmos_trap_out(ctl);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlNewControl(0x%08X, %u, %d, 0x%08X [%s], %d, %d, %d, %d, %d, %d, %d): 0x%08X", formPP, id, style, textP, text, x, y, width, height, font, group, leftAnchor, a);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case sysTrapCtlGetStyle68K: { // custom trap created for use in 68K code
      // ControlStyleType CtlGetStyle(ControlType *controlP)
      controlP = ARG32;
      control = (ControlType *)emupalmos_trap_in(controlP, trap, 0);
      stylest = control ? control->style : 0;
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGetStyle(0x%08X): %d", controlP, stylest);
      m68k_set_reg(M68K_REG_D0, stylest);
      }
      break;
    case sysTrapCtlGetLabel: {
      // const Char *CtlGetLabel(ControlType *controlP)
      controlP = ARG32;
      s_controlP = (ControlType *)emupalmos_trap_in(controlP, trap, 0);
      resc = (Char *)CtlGetLabel(s_controlP);
      r_res = emupalmos_trap_out(resc);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGetLabel(controlP=0x%08X): 0x%08X", controlP, r_res);
      }
      break;
    case sysTrapLstSetDrawFunction: {
      // void LstSetDrawFunction(ListType *listP, ListDrawDataFuncPtr func)
      listP = ARG32;
      funcP = ARG32;
      list = (ListType *)emupalmos_trap_in(listP, trap, 0);
      emupalmos_trap_in(funcP, trap, 1);
      if (list) list->m68k_drawfunc = funcP;
      debug(DEBUG_TRACE, "EmuPalmOS", "LstSetDrawFunction(0x%08X, 0x%08X)", listP, funcP);
      }
      break;
    case sysTrapLstDrawList: {
      // void LstDrawList(ListType *listP)
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
      // void SclSetScrollBar(ScrollBarType *bar, Int16 value, Int16 min, Int16 max, Int16 pageSize)
      barP = ARG32;
      value = ARG16;
      min = ARG16;
      max = ARG16;
      pageSize = ARG16;
      bar = (ScrollBarType *)emupalmos_trap_in(barP, trap, 0);
	  SclSetScrollBar(bar, (int16_t)value, (int16_t)min, (int16_t)max, (int16_t)pageSize);
      debug(DEBUG_TRACE, "EmuPalmOS", "SclSetScrollBar(0x%08X, %d, %d, %d, %d)", barP, value, min, max, pageSize);
      }
      break;
    case sysTrapSclGetScrollBar: {
      // void SclGetScrollBar(ScrollBarType *bar, Int16 *valueP, Int16 *minP, Int16 *maxP, Int16 *pageSizeP)
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
	  SclGetScrollBar(bar, (Int16 *)&value, &min, &max, &pageSize);
	  if (valueP) m68k_write_memory_16(valueP, (Int16)value);
      if (minP) m68k_write_memory_16(minP, min);
      if (maxP) m68k_write_memory_16(maxP, max);
      if (pageSizeP) m68k_write_memory_16(pageSizeP, pageSize);
      }
      break;
    case sysTrapSclDrawScrollBar: {
      // void SclDrawScrollBar(ScrollBarType *bar)
      barP = ARG32;
      bar = (ScrollBarType *)emupalmos_trap_in(barP, trap, 0);
      SclDrawScrollBar(bar);
      }
      break;
    case sysTrapSclHandleEvent: {
      // Boolean SclHandleEvent(ScrollBarType *bar, EventType *event)
      barP = ARG32;
      eventP = ARG32;

      if (eventP) decode_event(eventP, &event);
      bar = (ScrollBarType *)emupalmos_trap_in(barP, trap, 0);
      resb = SclHandleEvent(bar, &event);
      m68k_set_reg(M68K_REG_D0, resb);
      }
      break;
    case sysTrapEvtEnableGraffiti: {
      // void EvtEnableGraffiti(Boolean enable)
      uint8_t enable = ARG8;
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

      if (eventP) decode_event(eventP, &event);
      EvtAddUniqueEventToQueue(eventP ? &event : NULL, id, inPlace);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtAddUniqueEventToQueue(0x%08X [0x%04X], %d, %d)", eventP, event.eType, id, inPlace);
      }
      break;
    case sysTrapEvtAddEventToQueue: {
      // void EvtAddEventToQueue(const EventType *event)
      eventP = ARG32;
      emupalmos_trap_in(eventP, trap, 0);

      if (eventP) decode_event(eventP, &event);
      EvtAddEventToQueue(eventP ? &event : NULL);
      eventName = EvtGetEventName(event.eType);
      if (eventName) {
        debug(DEBUG_TRACE, "EmuPalmOS", "EvtAddEventToQueue(0x%08X [%s])", eventP, eventName);
      } else {
        debug(DEBUG_TRACE, "EmuPalmOS", "EvtAddEventToQueue(0x%08X [0x%04X])", eventP, event.eType);
      }
      if (eventP) encode_event(eventP, &event);
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

      MemSet(&event, sizeof(EventType), 0);
      EvtGetEvent(eventP ? &event : NULL, timeout);
      eventName = EvtGetEventName(event.eType);
      if (eventName) {
        debug(DEBUG_TRACE, "EmuPalmOS", "EvtGetEvent(0x%08X [%s], %d)", eventP, eventName, timeout);
      } else {
        debug(DEBUG_TRACE, "EmuPalmOS", "EvtGetEvent(0x%08X [0x%04X], %d)", eventP, event.eType, timeout);
      }
      if (eventP) encode_event(eventP, &event);
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
      // Boolean EvtSysEventAvail(Boolean ignorePenUps)
      Boolean ignorePenUps = ARG8;
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
      // Boolean EvtSetNullEventTick(UInt32 tick)
      UInt32 tick = ARG32;
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
	  ClipboardAddItem((uint8_t)format, ptrv, (uint16_t)length);
      debug(DEBUG_TRACE, "EmuPalmOS", "ClipboardAddItem(%d, 0x%08X, %d)", format, ptrP, length);
      }
      break;
    case sysTrapClipboardGetItem: {
      // MemHandle ClipboardGetItem(const ClipboardFormatType format, UInt16 *length)
      format = ARG8;
      lengthP = ARG32;
      emupalmos_trap_in(lengthP, trap, 1);
	  h = ClipboardGetItem((uint8_t)format, &length);
      r = emupalmos_trap_out(h);
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
      r = SysCreateDataBaseList68K(type, creator, &count, (void **)&list, lookupName);
      if (countP) m68k_write_memory_16(countP, count);
      if (listP) m68k_write_memory_32(listP, emupalmos_trap_out(list));
      debug(DEBUG_TRACE, "EmuPalmOS", "SysCreateDataBaseList(0x%08X, 0x%08X, 0x%08X, 0x%08X, %d)", type, creator, countP, listP, lookupName);
      m68k_set_reg(M68K_REG_D0, (Boolean)r);
      }
      break;
    case sysTrapUIPickColor: {
      // Boolean UIPickColor(IndexedColorType *indexP, RGBColorType *rgbP, UIPickColorStartType start, const Char *titleP, const Char *tipP)
      indexP = ARG32;
      rgbP = ARG32;
      start = ARG16;
      titleP = ARG32;
      tipP = ARG32;
      emupalmos_trap_in(indexP, trap, 0);
      emupalmos_trap_in(rgbP, trap, 1);
      if (indexP) index = m68k_read_memory_8(indexP);
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
      decode_smfoptions(selP, &options);
      res = SndPlaySmf(NULL, cmd, (UInt8 *)emupalmos_trap_in(smfP, trap, 2), selP ? &options : NULL, NULL, NULL, bNoWait);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndPlaySmf(0x%08X, %d, 0x%08X, 0x%08X, 0x%08X, 0x%08X, %d): %d", chanP, cmd, smfP, selP, chanRangeP, callbacksP, bNoWait, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapSndPlaySmfResource: {
      //Err SndPlaySmfResource(UInt32 resType, Int16 resID, SystemPreferencesChoice volumeSelector)
      resType = ARG32;
      resID = ARG32;
      volumeSelector = ARG8;
	  res = SndPlaySmfResource(resType, (int16)resID, volumeSelector);
	  debug(DEBUG_TRACE, "EmuPalmOS", "SndPlaySmfResource(0x%08X, %d, %d): %d", resType, (int16)resID, volumeSelector, res);
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
      beepID = ARG8;
      SndPlaySystemSound(beepID);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndSysBeepType(%d)", beepID);
      }
      break;
    case sysTrapSndPlayResource: {
      // Err SndPlayResource(SndPtr sndP, Int32 volume, UInt32 flags)
      sndP = ARG32;
      volume = ARG32;
      flags = ARG32;
      sndPtr = (void *)emupalmos_trap_in(sndP, trap, 0);
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
      err = errNone;
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
	  uint32_t userdataP = 0;
	  void *userdata = NULL;
      // Err SndStreamCreate(SndStreamRef *channel, SndStreamMode mode, UInt32 samplerate, SndSampleType type, SndStreamWidth width, SndStreamBufferCallback func, void *userdata, UInt32 buffsize, Boolean armNative)
      channelP = ARG32;
      mode = ARG8;
      samplerate = ARG32;
      type = ARG16;
      width = ARG8;
      funcP = ARG32;
      userDataP = ARG32;
      buffsize = ARG32;
      armNative = ARG8;
      channel = (SndStreamRef *)emupalmos_trap_in(channelP, trap, 0);
      func = (SndStreamBufferCallback)emupalmos_trap_in(funcP, trap, 5);
      userData = emupalmos_trap_in(userdataP, trap, 6);
	  err = SndStreamCreate(channel, (uint8_t)mode, samplerate, (uint16_t)type, (uint8_t)width, func, userdata, buffsize, armNative);
      if (channelP) m68k_write_memory_32(channelP, *channel);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamCreate(0x%08X, %d, %d, %d, %d, 0x%08X, 0x%08X, %d, %d): %d",
        channelP, mode, samplerate, type, width, funcP, userdataP, buffsize, armNative, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapSndStreamCreateExtended: {
	  uint32_t userdataP = 0;
	  void *userdata = NULL;
      // Err SndStreamCreateExtended(SndStreamRef *channel, SndStreamMode mode, SndFormatType format, UInt32 samplerate, SndSampleType type, SndStreamWidth width, SndStreamVariableBufferCallback func, void *userdata, UInt32 buffsize, Boolean armNative)
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
      channel = (SndStreamRef *)emupalmos_trap_in(channelP, trap, 0);
      sndfunc = (SndStreamVariableBufferCallback)emupalmos_trap_in(funcP, trap, 6);
      userData = emupalmos_trap_in(userdataP, trap, 7);
      err = SndStreamCreateExtended(channel, mode, format, samplerate, type, (uint8_t)width, sndfunc, userdata, buffsize, armNative);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamCreateExtented(0x%08X, %d, %d, %d %d, %d, 0x%08X, 0x%08X, %d, %d): %d",
        channelP, mode, format, samplerate, (uint16_t)type, width, funcP, userdataP, buffsize, armNative, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapSndStreamDelete: {
      // Err SndStreamDelete(SndStreamRef channel)
      channel32 = ARG32;
      err = SndStreamDelete(channel32);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamDelete(0x%08X): %d", channel32, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapSndStreamSetVolume: {
      // Err SndStreamSetVolume(SndStreamRef channel, Int32 volume)
      channel32 = ARG32;
      volume = ARG32;
      err = SndStreamSetVolume(channel32, volume);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamSetVolume(0x%08X, %d): %d", channel32, volume, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapSndStreamStart: {
      // Err SndStreamStart(SndStreamRef channel)
      channel32 = ARG32;
      err = SndStreamStart(channel32);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamStart(0x%08X): %d", channel32, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapSndStreamStop: {
      // Err SndStreamStop(SndStreamRef channel)
      channel32 = ARG32;
      err = SndStreamStop(channel32);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamStop(0x%08X): %d", channel32, err);
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
	  err = GrfSetState((uint8_t)capsLock, (uint8_t)numLock, upperShift);
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
      res = Crc16CalcBlock(bufp, count, crc);
      debug(DEBUG_TRACE, "EmuPalmOS", "Crc16CalcBlock(0x%08X, %d, 0x%04X): 0x%04X", bufP, count, crc, res);
      m68k_set_reg(M68K_REG_D0, res);
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
      GsiEnable((uint8_t)enableIt);
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
      // ProgressPtr PrgStartDialogV31(const Char *title, PrgCallbackFunc textCallback)
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
      // ProgressPtr PrgStartDialog(const Char *title, PrgCallbackFunc textCallback, void *userDataP)
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
      // void PrgStopDialog(ProgressPtr prgP, Boolean force)
      prgP = ARG32;
      force = ARG8;
      prg = emupalmos_trap_in(prgP, trap, 0);
      PrgStopDialog(prg, force);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrgStopDialog(0x%08X)", prgP);
    }
      break;
    case sysTrapPrgHandleEvent: {
      // Boolean PrgHandleEvent(ProgressPtr prgGP, EventType *eventP)
      prgP = ARG32;
      eventP = ARG32;
      prg = emupalmos_trap_in(prgP, trap, 0);
      emupalmos_trap_in(eventP, trap, 1);

      if (eventP) decode_event(eventP, &event);
      resb = PrgHandleEvent(prg, &event);
      eventName = EvtGetEventName(event.eType);
      if (eventName) {
        debug(DEBUG_TRACE, "EmuPalmOS", "PrgHandleEvent(0x%08X, 0x%08X [%s]): %d", prgP, eventP, eventName, resb);
      } else {
        debug(DEBUG_TRACE, "EmuPalmOS", "PrgHandleEvent(0x%08X, 0x%08X [0x%04X]): %d", prgP, eventP, event.eType, resb);
      }
      m68k_set_reg(M68K_REG_D0, resb);
    }
      break;
    case sysTrapPrgUpdateDialog: {
      // void PrgUpdateDialog(ProgressPtr prgGP, UInt16 err, UInt16 stage, const Char *messageP, Boolean updateNow)
      prgP = ARG32;
      err = ARG16;
      stage = ARG16;
      messageP = ARG32;
      updateNow = ARG8;
      prg = emupalmos_trap_in(prgP, trap, 0);
      message = emupalmos_trap_in(messageP, trap, 3);
      PrgUpdateDialog(prg, err, stage, message, updateNow);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrgUpdateDialog(0x%08X, %d, %d, 0x%08X [%s], %d)", prgP, err, stage, messageP, messageP ? message : "", updateNow);
    }
      break;
    case sysTrapEncDigestMD5: {
      // Err EncDigestMD5(UInt8 *strP, UInt16 strLen, UInt8 digestP[16])
      strP = ARG32;
      strLen = ARG16;
      digestP = ARG32;
      str = emupalmos_trap_in(strP, trap, 0);
      digest = emupalmos_trap_in(digestP, trap, 2);
      res = EncDigestMD5((uint8_t *)str, strLen, digest);
      debug(DEBUG_TRACE, "EmuPalmOS", "EncDigestMD5(0x%08X, %u, 0x%08X): %d", strP, strLen, digestP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;
    case sysTrapGetCharCaselessValue: {
      // const UInt8 *GetCharCaselessValue(void)
      resu8 = (UInt8 *)GetCharCaselessValue();
      a = emupalmos_trap_out(resu8);
      debug(DEBUG_TRACE, "EmuPalmOS", "GetCharCaselessValue(): 0x%08X", a);
      m68k_set_reg(M68K_REG_A0, a);
    }
      break;
    case sysTrapGetCharSortValue: {
      // const UInt8 *GetCharSortValue(void) 
      resu8 = (UInt8 *)GetCharSortValue();
      a = emupalmos_trap_out(resu8);
      debug(DEBUG_TRACE, "EmuPalmOS", "GetCharSortValue(): 0x%08X", a);
      m68k_set_reg(M68K_REG_A0, a);
    }
      break;
    case sysTrapAlmSetAlarm: {
      // Err AlmSetAlarm(UInt16 cardNo, LocalID dbID, UInt32 ref, UInt32 alarmSeconds, Boolean quiet)
      cardNo = ARG16;
      dbID = ARG32;
      ref = ARG32;
      alarmSeconds = ARG32;
      quiet = ARG8;
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
      ax = m68k_read_memory_32(bufP);
      m68k_set_reg(M68K_REG_D3, ax);
      ax = m68k_read_memory_32(bufP + 4);
      m68k_set_reg(M68K_REG_D4, ax);
      ax = m68k_read_memory_32(bufP + 8);
      m68k_set_reg(M68K_REG_D5, ax);
      ax = m68k_read_memory_32(bufP + 12);
      m68k_set_reg(M68K_REG_D6, ax);
      ax = m68k_read_memory_32(bufP + 16);
      m68k_set_reg(M68K_REG_D7, ax);
      ax = m68k_read_memory_32(bufP + 20);
      m68k_set_reg(M68K_REG_PC, ax);
      ax = m68k_read_memory_32(bufP + 24);
      m68k_set_reg(M68K_REG_A2, ax);
      ax = m68k_read_memory_32(bufP + 28);
      m68k_set_reg(M68K_REG_A3, ax);
      ax = m68k_read_memory_32(bufP + 32);
      m68k_set_reg(M68K_REG_A4, ax);
      ax = m68k_read_memory_32(bufP + 36);
      m68k_set_reg(M68K_REG_A5, ax);
      ax = m68k_read_memory_32(bufP + 40);
      m68k_set_reg(M68K_REG_A6, ax);
      ax = m68k_read_memory_32(bufP + 44);
      m68k_set_reg(M68K_REG_A7, ax);
      debug(DEBUG_TRACE, "EmuPalmOS", "ErrThrow(%d)", code);
      m68k_set_reg(M68K_REG_D0, code);
    }
      break;
    case sysTrapErrSetJump: {
      // Int16 ErrSetJump(ErrJumpBuf buf)
      bufP = ARG32;
      emupalmos_trap_in(bufP, trap, 0);
      // typedef long *ErrJumpBuf[12];  // D3-D7,PC,A2-A7
      ax = m68k_get_reg(NULL, M68K_REG_D3);
      m68k_write_memory_32(bufP, ax);
      ax = m68k_get_reg(NULL, M68K_REG_D4);
      m68k_write_memory_32(bufP + 4, ax);
      ax = m68k_get_reg(NULL, M68K_REG_D5);
      m68k_write_memory_32(bufP + 8, ax);
      ax = m68k_get_reg(NULL, M68K_REG_D6);
      m68k_write_memory_32(bufP + 12, ax);
      ax = m68k_get_reg(NULL, M68K_REG_D7);
      m68k_write_memory_32(bufP + 16, ax);
      ax = m68k_get_reg(NULL, M68K_REG_PC);
      m68k_write_memory_32(bufP + 20, ax);
      ax = m68k_get_reg(NULL, M68K_REG_A2);
      m68k_write_memory_32(bufP + 24, ax);
      ax = m68k_get_reg(NULL, M68K_REG_A3);
      m68k_write_memory_32(bufP + 28, ax);
      ax = m68k_get_reg(NULL, M68K_REG_A4);
      m68k_write_memory_32(bufP + 32, ax);
      ax = m68k_get_reg(NULL, M68K_REG_A5);
      m68k_write_memory_32(bufP + 36, ax);
      ax = m68k_get_reg(NULL, M68K_REG_A6);
      m68k_write_memory_32(bufP + 40, ax);
      ax = m68k_get_reg(NULL, M68K_REG_A7);
      m68k_write_memory_32(bufP + 44, ax);
      debug(DEBUG_TRACE, "EmuPalmOS", "ErrSetJump(0x%08X): %d", bufP, 0);
      m68k_set_reg(M68K_REG_D0, 0); // XXX not calling ErrSetJump()
    }
      break;
    case sysTrapErrLongJump: {
      // void ErrLongJump(ErrJumpBuf buf, Int16 result)
      bufP = ARG32;
      result = ARG16;
      emupalmos_trap_in(bufP, trap, 0);
      ax = m68k_read_memory_32(bufP);
      m68k_set_reg(M68K_REG_D3, ax);
      ax = m68k_read_memory_32(bufP + 4);
      m68k_set_reg(M68K_REG_D4, ax);
      ax = m68k_read_memory_32(bufP + 8);
      m68k_set_reg(M68K_REG_D5, ax);
      ax = m68k_read_memory_32(bufP + 12);
      m68k_set_reg(M68K_REG_D6, ax);
      ax = m68k_read_memory_32(bufP + 16);
      m68k_set_reg(M68K_REG_D7, ax);
      ax = m68k_read_memory_32(bufP + 20);
      m68k_set_reg(M68K_REG_PC, ax);
      ax = m68k_read_memory_32(bufP + 24);
      m68k_set_reg(M68K_REG_A2, ax);
      ax = m68k_read_memory_32(bufP + 28);
      m68k_set_reg(M68K_REG_A3, ax);
      ax = m68k_read_memory_32(bufP + 32);
      m68k_set_reg(M68K_REG_A4, ax);
      ax = m68k_read_memory_32(bufP + 36);
      m68k_set_reg(M68K_REG_A5, ax);
      ax = m68k_read_memory_32(bufP + 40);
      m68k_set_reg(M68K_REG_A6, ax);
      ax = m68k_read_memory_32(bufP + 44);
      m68k_set_reg(M68K_REG_A7, ax);
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
    case sysTrapPumpkinDebug: {
      // changes in M68K /opt/palmdev/<sdk>/include/Core/CoreTraps.h:
      // #define sysTrapPumpkinDebug 0xA506
      // #define sysTrapLastTrapNumber 0xA507

      // changes in M68K /opt/palmdev/<sdk>/include/Core/System/SysUtils.h:
      // void PumpkinDebug(UInt16 level, Char *sys, Char *buf) SYS_TRAP(sysTrapPumpkinDebug);

      level = ARG16;
      sysP = ARG32;
      bufP = ARG32;
      sys = emupalmos_trap_in(sysP, trap, 1);
      bufp = emupalmos_trap_in(bufP, trap, 2);
      debug(level, sys, "%s", (char *)bufp);
      break;
    }
    case sysTrapPumpkinDebugBytes: {
      level = ARG16;
      sysP = ARG32;
      bufP = ARG32;
      len = ARG32;
      sys = emupalmos_trap_in(sysP, trap, 1);
      bufp = emupalmos_trap_in(bufP, trap, 2);
      debug_bytes(level, sys, (uint8_t *)bufp, len);
      break;
    }

    default:
      if (trap > sysLibTrapName) {
        refNum = ARG16;
        if (refNum > MAX_SYSLIBS) {
          palmos_libtrap(refNum, trap);
          break;
        }
        index = trap - sysLibTrapName;
        num = 0;
        s = NULL;
        dispatch = SysLibGetDispatch68K(refNum);

        switch (trap) {
          case sysLibTrapOpen:  s = "Open";  break;
          case sysLibTrapClose: s = "Close"; break;
          case sysLibTrapSleep: s = "Sleep"; break;
          case sysLibTrapWake:  s = "Wake";  break;
          default:
            num = trap - sysLibTrapCustom;
            break;
        }

        if (dispatch) {
          get2b(&offset, (uint8_t *)dispatch, index*2);
          addr = (uint8_t *)dispatch + offset;

          if (s) {
            debug(DEBUG_INFO, "EmuPalmOS", "sysLibTrap%s refNum=%d index=%d", s, refNum, index);
          } else {
            debug(DEBUG_TRACE, "EmuPalmOS", "sysLibTrapCustom %d refNum=%d index=%d", num, refNum, index);
          }
          r = emupalmos_trap_out(addr);
        } else {
          if (s) {
            sys_snprintf(buf, sizeof(buf)-1, "sysLibTrap%s refNum=%d index=%d: no dispatch table", s, refNum, index);
          } else {
            sys_snprintf(buf, sizeof(buf)-1, "sysLibTrapCustom %d refNum=%d index=%d: no dispatch table", num, refNum, index);
          }
          emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
        }
      } else {
  
        sys_snprintf(buf, sizeof(buf)-1, "trap 0x%04X %s not mapped", trap, logtrap_trapname(state->lt, trap, &selector, 0));
        emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      }
      break;
  }

  debug(DEBUG_TRACE, "EmuPalmOS", "trap 0x%04X end (int)", trap);
  pumpkin_trace(trap);

  return r;
}
