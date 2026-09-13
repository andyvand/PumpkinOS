#ifndef MAIL_RESOURCE_H
#define MAIL_RESOURCE_H

/* forms */
#define MainForm        1000
#define ViewForm        1100
#define ComposeForm     1200
#define AccountForm     1300
#define WaitForm        1400

/* menus */
#define MainMenu        1000
#define ViewMenu        1100
#define ComposeMenu     1200

/* main form */
#define msgGad          1001
#define newBtn          1002
#define getBtn          1003
#define folderTrig      1004
#define folderList      1005
#define accountBtn      1006
#define msgScl          1007

/* view form */
#define bodyFld         1101
#define bodyScl         1102
#define doneBtn         1103
#define replyBtn        1104
#define fwdBtn          1105
#define delBtn          1106
#define headerGad       1107

/* compose form */
#define toFld           1201
#define ccFld           1202
#define subjFld         1203
#define cbodyFld        1204
#define cbodyScl        1205
#define sendBtn         1206
#define cancelBtn       1207
#define toLbl           1210
#define ccLbl           1211
#define subjLbl         1212

/* account form */
#define nameFld         1301
#define emailFld        1302
#define userFld         1303
#define passFld         1304
#define imapHostFld     1305
#define imapPortFld     1306
#define imapSecTrig     1307
#define imapSecList     1308
#define smtpHostFld     1309
#define smtpPortFld     1310
#define smtpSecTrig     1311
#define smtpSecList     1312
#define fetchFld        1313
#define okBtn           1314
#define acancelBtn      1315
#define nameLbl         1320
#define emailLbl        1321
#define userLbl         1322
#define passLbl         1323
#define imapLbl         1324
#define imapSecLbl      1325
#define smtpLbl         1326
#define smtpSecLbl      1327
#define fetchLbl        1328
#define fetchUnitLbl    1329
#define protoTrig       1330
#define protoList       1331

/* wait form */
#define waitCancelBtn   1401
#define waitGad         1402

/* alerts */
#define ErrorAlert      2001
#define InfoAlert       2002
#define ConfirmAlert    2003
#define NoAccountAlert  2004

/* menu commands */
#define newCmd          1
#define getCmd          2
#define foldersCmd      3
#define accountCmd      4
#define aboutCmd        5
#define replyCmd        6
#define fwdCmd          7
#define delCmd          8
#define doneCmd         9
#define sendCmd         10
#define discardCmd      11

#endif
