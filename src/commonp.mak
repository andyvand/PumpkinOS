OSNAME    = PumpkinOS
PCC       = /c/msys64/usr/bin/m68k-palmos-gcc
PILRC     = $(TOOLS)/pilrc -q -allowLargeRes
PRCBUILD  = $(TOOLS)/prcbuild
BUILDPRC  = /c/msys64/usr/bin/build-prc
ifeq ($(OSNAME),Msys)
ifeq ($(MACHINE),x86_64)
DLIBID    = 146
DLIB      = dlib0092.bin
else ifeq ($(MACHINE),arm)
DLIBID    = 137
DLIB      = dlib0089.bin
else ifeq ($(MACHINE),aarch64)
DLIBID    = 138
DLIB      = dlib008a.bin
else ifeq ($(MACHINE),i686)
DLIBID    = 145
DLIB      = dlib0091.bin
else
DLIBID    = 154
DLIB      = dlib009a.bin
endif
else
DLIBID    = $(shell expr $(SYS_OS) \* 64 + $(SYS_CPU) \* 8 + $(SYS_SIZE))
DLIB      = $(shell printf "dlib%04x.bin" $(DLIBID))
endif

PALMOS    = $(SRC)/PalmOS
CUSTOMFLAGS=-DUNIX -DOSNAME=\"$(OSNAME)\" -I$(SRC)/libpumpkin -I$(SRC)/font -I$(PALMOS) -I$(PALMOS)/Core -I$(PALMOS)/Core/System -I$(PALMOS)/Dynamic -I$(PALMOS)/Core/UI -I$(PALMOS)/Libraries -I$(PALMOS)/Libraries/PalmOSGlue -I$(PALMOS)/Libraries/Pdi -I$(PALMOS)/Libraries/CPMLib -I$(PALMOS)/Libraries/INet -I$(PALMOS)/Libraries/SSL -I$(PALMOS)/Extensions/ExpansionMgr -I$(PALMOS)/Extensions/Bluetooth -I$(PALMOS)/Core/Hardware -I$(PALMOS)/Garmin -I$(PALMOS)/PalmOne -I$(PALMOS)/PalmOne/Common/System -I$(PALMOS)/PalmOne/68K/System -I$(PALMOS)/PalmOne/68K/Libraries/filebrowser -I$(PALMOS)/PalmOne/Common/Libraries/filebrowser -I$(PALMOS)/Sony -I$(PALMOS)/Sony/System -I$(PALMOS)/Sony/Libraries $(CUSTOMPFLAGS)
