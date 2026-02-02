OSNAME    = PumpkinOS
PCC       = /c/msys64/usr/bin/m68k-palmos-gcc
PILRC     = $(TOOLS)/pilrc -q -allowLargeRes
PRCBUILD  = $(TOOLS)/prcbuild
BUILDPRC  = /c/msys64/usr/bin/build-prc
ifeq ($(OSNAME),Msys)
DLIBID    = 146
DLIB      = dlib0092.bin
else
DLIBID    = $(shell expr $(SYS_OS) \* 64 + $(SYS_CPU) \* 8 + $(SYS_SIZE))
DLIB      = $(shell printf "dlib%04x.bin" $(DLIBID))
endif

PALMOS    = $(SRC)/PalmOS
CUSTOMFLAGS=-DUNIX -DOSNAME=\"$(OSNAME)\" -I$(SRC)/libpumpkin -I$(SRC)/font -I$(PALMOS) -I$(PALMOS)/Core -I$(PALMOS)/Core/System -I$(PALMOS)/Dynamic -I$(PALMOS)/Core/UI -I$(PALMOS)/Libraries -I$(PALMOS)/Libraries/PalmOSGlue -I$(PALMOS)/Libraries/Pdi -I$(PALMOS)/Libraries/CPMLib -I$(PALMOS)/Libraries/INet -I$(PALMOS)/Libraries/SSL -I$(PALMOS)/Extensions/ExpansionMgr -I$(PALMOS)/Extensions/Bluetooth -I$(PALMOS)/Core/Hardware -I$(PALMOS)/Garmin -I$(PALMOS)/PalmOne -I$(PALMOS)/PalmOne/Common/System -I$(PALMOS)/PalmOne/68K/System -I$(PALMOS)/PalmOne/68K/Libraries/filebrowser -I$(PALMOS)/PalmOne/Common/Libraries/filebrowser $(CUSTOMPFLAGS)
