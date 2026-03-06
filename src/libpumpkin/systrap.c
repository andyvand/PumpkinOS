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

uint32_t palmos_systrap(uint16_t trap) {
  uint32_t sp = 0;
  uint16_t idx = 0, selector = 0;
  char buf[256], buf2[8];
  char *s = NULL;
  Err err = errNone;
  uint8_t *ram = pumpkin_heap_base();
  emu_state_t *state = m68k_get_emu_state();
  uint32_t r = 0;
  privateRecordViewEnum rprv = (privateRecordViewEnum)0;
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
  SysLibTblEntryType tbl = { 0 };
  uint32_t a = 0;
  uint32_t pP = 0;
  uint16_t featureNum = 0;
  uint32_t valueP = 0;
  uint32_t value = 0;
  uint32_t value16 = 0;
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
  Int32 logLen = 0;
  uint32_t succSyncDateP = 0;
  uint32_t lastSyncDateP = 0;
  uint32_t syncStateP = 0;
  uint32_t nameBufP = 0;
  uint32_t logBufP = 0;
  uint32_t logLenP = 0;
  UInt32 succSyncDate = 0;
  UInt32 lastSyncDate = 0;
  DlkSyncStateType syncState = (DlkSyncStateType)0;
  uint16_t cardNo = 0;
  uint32_t dbID = 0;
  uint32_t iterationData = 0;
  uint16_t enableIt = 0;
  uint16_t rscID = 0;
  uint32_t nativeFuncP = 0;
  uint32_t userDataP = 0;
  uint32_t startPP = 0;
  uint32_t endPP = 0;
  uint16_t trapNum = 0;
  uint32_t procP = 0;
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
  void *userData = NULL;
  SysNotifyParamType notify = { 0 };
  Boolean resb = 0;
  uint16_t numOfElements = 0;
  int16_t stringCount = 0;
  uint32_t stringP = 0;
  int16_t theID = 0;
  char *string = NULL;
  uint32_t resID = 0;
  uint32_t strP = 0;
  uint16_t maxLen = 0;
  uint32_t c = 0;
  uint32_t notifyP = 0;
  Char *resc = NULL;
  uint32_t newPtrP = 0;
  uint32_t newValue = 0;
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
  UInt16 l_categoryP = 0;
  uint32_t categoryP = 0;
  uint16_t lstID = 0;
  uint16_t ctlID = 0;
  char *s_categoryName = NULL;
  FormType *s_frm = NULL;
  uint32_t frm = 0;
  uint32_t titleStrID = 0;
  uint16_t l_category = 0;
  uint8_t title8 = 0;
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
  uint16_t frame = 0;
  uint8_t modal = 0;
  uint8_t focusable = 0;
  uint32_t errorP = 0;
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
  uint8_t oldFontID = 0;
  RGBColorType rgb = { 0, 0, 0, 0 };
  uint8_t which = 0;
  uint32_t rgbP = 0;
  IndexedColorType cct = 0;
  uint8_t choice = 0;
  uint8_t saved = 0;
  DmOpenRef dbRef = NULL;
  int16_t version = 0;
  uint16_t versionu = 0;
  uint32_t prefsP = 0;
  uint16_t prefsSize = 0;
  uint32_t prefsSizeP = 0;
  uint32_t dstP = 0;
  uint32_t numBytes = 0;
  int32_t numBytesi = 0;
  UInt32 start = 0, end = 0;
  uint32_t sP = 0;
  int32_t newSeed = 0;
  uint32_t dbP = 0;
  uint32_t oldHP = 0;
  MemHandle old = NULL;
  uint32_t ih = 0;
  uint32_t dbPP = 0;
  MemHandle hm = NULL;
  Boolean b = 0;
  DmOpenRef dbPOR = NULL;
  uint8_t newSearch = 0;
  uint32_t stateInfoP = 0;
  uint8_t onlyLatestVers = 0;
  uint32_t cardNoP = 0;
  uint32_t dbIDP = 0;
  DmSearchStateType stateInfo = { 0 };
  uint32_t comparP = 0;
  uint32_t newRecordP = 0;
  uint32_t newRecordInfoP = 0;
  uint32_t atP = 0;
  uint32_t newH = 0;
  UInt16 at = 0;
  uint32_t formatStr = 0;
  char *f = NULL;
  uint32_t f32 = 0;
  int vararg = 0;
  int j = 0, k = 1, t = 0, sz = 0, arglen = 0;
  uint32_t arg = 0, v_arg = 0;
  char *q = NULL, fmt[16];
  uint16_t formID = 0;
  uint32_t titleStrP = 0;
  uint16_t defaultButton = 0;
  uint16_t helpRscID = 0;
  uint16_t menuRscID = 0;
  char *titleStr = NULL;
  FormType *form = NULL;
  void *formv = NULL;
  uint32_t formP = 0;
  uint32_t objP = 0;
  void *obj = NULL;
  uint16_t updateCode = 0;
  uint32_t newTitleP = 0;
  uint32_t handlerP = 0;
  uint16_t objIndex = 0;
  FieldType *fld = NULL;
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
  uint16_t labelID = 0;
  const Char *label = NULL;
  uint16_t fieldIndex = 0;
  uint32_t maxP = 0;
  uint32_t pageSizeP = 0;
  Int16 max = 0, pageSize = 0;
  ScrollBarType *bar = NULL;
  uint32_t barP = 0;
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
  FormCheckResponseFuncPtr callbackfp = NULL;
  uint32_t formPP = 0;
  uint32_t stateP = 0;
  uint8_t save = 0;
  uint32_t ctl = 0;
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
  uint32_t r_res = 0;
  uint8_t inPlace = 0;
  FormActiveStateType *statef = NULL;
  char *eventName = NULL;
  uint16_t ascii = 0;
  uint16_t keycode = 0;
  uint16_t modifiers = 0;
  int32_t timeout = 0;
  uint32_t sourceP = 0;
  uint32_t destP = 0;
  EventType source = { 0 }, dest = { 0 };
  uint32_t pScreenX = 0;
  uint32_t pScreenY = 0;
  uint32_t pPenDown = 0;
  Int16 screenX = 0, screenY = 0;
  Boolean penDown = 0;
  uint32_t lengthP = 0;
  uint32_t numP = 0;
  uint32_t countP = 0;
  uint8_t lookupName = 0;
  uint32_t indexP = 0;
  uint32_t tipP = 0;
  uint8_t numberFormat = 0;
  uint32_t thousandSeparatorP = 0;
  uint32_t decimalSeparatorP = 0;
  uint32_t chanP = 0;
  uint32_t smfP = 0;
  uint32_t selP = 0;
  uint32_t chanRangeP = 0;
  uint32_t callbacksP = 0;
  uint8_t bNoWait = 0;
  uint32_t resType = 0;
  uint8_t volumeSelector = 0;
  uint32_t ptrP = 0;
  void *ptrv = NULL;
  IndexedColorType indexc = (IndexedColorType)0;
  uint8_t multipleDBs = 0;
  uint32_t wCountP = 0;
  uint32_t entHP = 0;
  UInt16 wCount = 0;
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
  char *strc = NULL;
  SndStreamRef *channelr = NULL;
  SndStreamBufferCallback func = NULL;
  SndStreamVariableBufferCallback funcv = NULL;
  uint32_t channel = 0;
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
  uint16_t stage = 0;
  uint32_t messageP = 0;
  uint8_t updateNow = 0;
  char thousandSeparator = 0, decimalSeparator = 0;
  uint8_t *e = NULL;
  uint32_t exceptionP = 0;
  uint32_t nextP = 0;
  UInt32 ref = 0;
  uint32_t textCallbackP = 0;
  uint32_t alarmSeconds = 0;
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
  void *prgv = NULL;
  UInt16 res16 = 0;
  MemHandle listm = NULL;
  UInt32 tick = 0;
  Boolean ignorePenUps = 0;
  uint8_t enable = 0;
  uint16_t helpMsgId = 0;
  uint32_t s32 = 0;
  char *strp = NULL;
  LocalID dbIDL = 0;
  uint32_t info = 0;
  uint8_t *p = 0;
  uint32_t p32 = 0;
  UInt32 tm = 0;
  uint16_t l_valueP = 0;
  uint16_t l_pageSizeP = 0;
  uint16_t l_minValueP = 0;
  uint16_t l_maxValueP = 0;
  uint32_t minValueP = 0;
  uint16_t newSelectedBitmapID = 0;
  uint16_t newBitmapID = 0;
  uint32_t newLabel = 0;
  uint16_t positionId = 0;
  uint32_t ctlP = 0;
  uint16_t localizedAppInfoStrID = 0;
  uint32_t appInfoP = 0;
  char *s_name = NULL;
  uint32_t name32 = 0;
  uint32_t categoryName = 0;
  uint8_t resizeList = 0;
  uint32_t editingStrID = 0;
  uint8_t numUneditableCategories = 0;
  uint8_t showUneditables = 0;
  uint8_t showAll = 0;
  uint16_t currentCategory = 0;
  ListType *s_lst = NULL;
  uint32_t lst = 0;
  DmOpenRef l_db = NULL;
  uint32_t db = 0;
  uint8_t l_queueAheadP = 0;
  uint32_t queueAheadP = 0;
  ControlType *s_ctl = NULL;
  uint16_t l_doubleTapDelayP = 0;
  uint16_t l_periodP = 0;
  uint32_t periodP = 0;
  uint16_t l_initDelayP = 0;
  uint32_t initDelayP = 0;
  Boolean l_pluggedIn = 0;
  SysBatteryKind l_kindP = (SysBatteryKind)0;
  Int16 l_maxTicksP = 0;
  UInt16 l_criticalThresholdP = 0;
  UInt16 l_warnThresholdP = 0;
  uint32_t percentP = 0;
  uint32_t pluggedIn = 0;
  uint32_t kindP = 0;
  uint32_t maxTicksP = 0;
  uint32_t criticalThresholdP = 0;
  uint32_t warnThresholdP = 0;
  uint8_t newBrightnessLevel = 0;
  uint8_t newContrastLevel = 0;
  uint16_t launchFlags = 0;
  Boolean set = 0;
  uint32_t cmdPBP = 0;
  launch_union_t param = { 0 };
  uint32_t doubleTapDelayP = 0;
  uint32_t cardNameP = 0;
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
  UInt32 l_maxP = 0;
  uint16_t numHandles = 0;
  uint8_t initContents = 0;
  uint16_t ownerID = 0;
  uint16_t attr = 0;
  MemPtr resm = NULL;
  uint32_t chunkDataP = 0;
  char *l_chunkDataP = NULL;
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
  uint32_t s1_32 = 0;
  uint32_t s2_32 = 0;
  uint8_t writeAccess = 0;
  char *s_nameP = NULL;
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
  DmOpenRef l_dbP = 0;
  DmOpenRef l_currentP = 0;
  LocalID l_dbIDP = 0;
  uint32_t openCountP = 0;
  UInt16 l_openCountP = 0;
  uint32_t modeP = 0;
  UInt16 l_modeP = 0;
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
  UInt16 l_atP = 0;
  uint32_t highest = 0;
  uint32_t uniqueID = 0;
  UInt16 l_indexP = 0;
  int16_t direction = 0;
  uint8_t dirty = 0;
  uint16_t toCategory = 0;
  uint16_t fromCategory = 0;
  uint16_t categoryNum = 0;
  uint32_t recordP = 0;
  void *s_recordP = NULL;
  uint32_t bytes = 0;
  uint32_t srcP = 0;
  void *s_srcP = NULL;
  uint32_t resourceH = 9;
  MemHandle l_resourceH = NULL;
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
  RectangleType l_r;
  uint32_t source32 = 0;
  RectangleType l_source;
  uint32_t error = 0;
  uint16_t error16 = 0;
  UInt16 l_error = 0;
  int16_t destX = 0;
  int16_t destY = 0;
  uint32_t srcWin = 0;
  WinHandle l_srcWin = NULL;
  uint32_t dstWin = 0;
  WinHandle l_dstWin = NULL;
  uint32_t srcRect = 0;
  RectangleType l_srcRect;
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
  ListType *s_listP = NULL;
  int16_t itemCount = 0;
  BitmapType *resbm = NULL;
  FontID resfont = 0;
  BitmapType *l_bitmapP = NULL;
  void *s_dataP = NULL;
  int32_t objSize = 0;
  int32_t numObj = 0;
  uint32_t errP = 0;
  Err l_errP = 0;
  uint32_t openMode = 0;
  uint8_t newMode = 0;
  uint8_t foreColor = 0;
  IndexedColorType resic = (IndexedColorType)0;
  uint8_t backColor = 0;
  uint8_t textColor = 0;
  uint32_t newRgbP = 0;
  RGBColorType l_newRgbP;
  uint32_t prevRgbP = 0;
  RGBColorType l_prevRgbP;
  CustomPatternType *s_patternP = NULL;
  PatternType respt;
  uint8_t newPattern = 0;
  RGBColorType l_rgbP = { 0, 0, 0, 0 };
  uint16_t numPoints = 0;
  uint32_t pts = 0;
  int16_t x1 = 0;
  int16_t y1 = 0;
  int16_t x2 = 0;
  int16_t y2 = 0;
  uint16_t cornerDiam = 0;
  uint16_t theChar = 0;
  uint32_t chars = 0;
  char *s_chars = NULL;
  int16_t maxWidth = 0;
  UnderlineModeType resumt;
  uint32_t patternP = 0;
  uint32_t newForeColorP = 0;
  RGBColorType l_newForeColorP = { 0, 0, 0, 0 };
  uint32_t oldForeColorP = 0;
  RGBColorType l_oldForeColorP = { 0, 0, 0, 0 };
  uint32_t newBackColorP = 0;
  RGBColorType l_newBackColorP = { 0, 0, 0, 0 };
  uint32_t oldBackColorP = 0;
  RGBColorType l_oldBackColorP = { 0, 0, 0, 0 };
  uint8_t compType = 0;
  void *resv = NULL;
  WinDrawOperation resdo;
  UInt8 *res8 = NULL;
  uint32_t dataSizeP = 0;
  UInt32 l_dataSizeP = 0;
  ColorTableType *resctt = NULL;
  uint32_t headerSizeP = 0;
  UInt32 l_headerSizeP = 0;
  Coord l_widthP = 0;
  Coord l_heightP = 0;
  uint32_t rowBytesP = 0;
  UInt16 l_rowBytesP = 0;
  int8_t ch = 0;
  uint16_t iChar = 0;
  char *s_pChars = NULL;
  int16_t pixelWidth = 0;
  uint32_t leadingEdge = 0;
  Boolean l_leadingEdge = 0;
  uint32_t truncWidth = 0;
  Int16 l_truncWidth = 0;
  uint32_t string32 = 0;
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
  uint16_t scrollPos = 0;
  uint32_t linesP = 0;
  UInt16 l_linesP = 0;
  uint32_t topLine = 0;
  UInt16 l_topLine = 0;
  uint32_t dst32 = 0;
  char *s_dst = NULL;
  uint32_t src32 = 0;
  char *s_src = NULL;
  uint32_t pChars = 0;
  int32_t n = 0;
  uint16_t length = 0;
  char *s_s1 = NULL;
  char *s_s2 = NULL;
  char *s_s = NULL;
  uint8_t initMode = 0;
  uint32_t str = 0;
  uint32_t token = 0;
  uint32_t fldP = 0;
  FieldType *s_fldP = NULL;
  char *s_str = NULL;
  uint16_t chr = 0;
  char *s_token = NULL;
  RectangleType l_rect;
  uint32_t rect32 = 0;
  uint32_t startPosition = 0;
  UInt16 l_startPosition = 0;
  uint32_t endPosition = 0;
  UInt16 l_endPosition = 0;
  uint32_t eventP = 0;
  EventType l_eventP = { 0 };
  uint8_t redraw = 0;
  FontID fontID = 0;
  uint32_t textHandle = 0;
  MemHandle l_textHandle = NULL;
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
  UInt16 attrBits = 0;
  FieldAttrType attrFields;
  uint16_t allocatedSize = 0;
  uint8_t maxLines = 0;
  uint32_t maxChars = 0;
  uint8_t editable = 0;
  uint8_t underlined = 0;
  uint8_t singleLine = 0;
  uint8_t dynamicSize = 0;
  uint8_t justification = 0;
  uint8_t autoShift = 0;
  uint8_t hasScrollBar = 0;
  uint8_t numeric = 0;
  TableType *s_tableP = NULL;
  uint32_t event = 0;
  EventType l_event = { 0 };
  int16_t row = 0;
  int16_t numLines = 0;
  void *s_value = NULL;
  uint8_t selectable = 0;
  Coord resco = 0;
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
  uint16_t resourceId = 0;
  int16_t buttonIndex = 0;
  uint32_t bitmapIdP = 0;
  UInt16 l_bitmapIdP = 0;
  MenuCmdBarResultType l_resultTypeP = (MenuCmdBarResultType)0;
  UInt32 l_resultP = 0;
  uint32_t resultTypeP = 0;
  uint32_t resultP = 0;
  uint16_t cmd = 0;
  Int16 l_x = 0;
  Int16 l_y = 0;
  char *s_newLabel = NULL;
  ControlType *s_ctlP = NULL;
  EventType l_pEvent = { 0 };
  uint32_t pEvent = 0;
  FileHand resfh = NULL;
  void *s_baseP = NULL;
  uint8_t dataStoreBased = 0;
  uint8_t origin = 0;
  uint32_t fileSizeP = 0;
  Int32 l_fileSizeP = 0;
  UInt8 l_percentP = 0;
  AppInfoType *l_appInfoP = NULL;
  UIntPtr resUP = 0;
  uint32_t maxValueP = 0;
  uint32_t aux = 0;
  SystemPreferencesType prefs = { 0 };
  uint16_t uresID = 0;
  FormBitmapType *Formbitmap = NULL;

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

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;

  switch (trap) {
#include "switch.c"

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
