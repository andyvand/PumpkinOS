ifeq ($(PUMPKIN_APP),)
LIBS=$(EXTRALIBS) -L$(BIN) -lpumpkin -lpit
else
CUSTOMPFLAGS=-DPUMPKIN_APP -fvisibility=hidden
STUBS_OPT=$(STUBS).o
endif

ifeq ($(PROGRAM),)
PROGRAM=$(APPNAME)
endif

ifeq ($(APPTYPE),)
APPTYPE=appl
endif

ifeq ($(OBJS),)
ifneq ($(SOURCE),)
OBJS=$(SOURCE:%.c=%.o)
endif
endif

ifneq ($(OBJS),)
DLIB_OPT=$(DLIB)
endif

ifneq ($(RCP),)
RESFLAG=res.flag
endif

PRC=$(VFS)/$(VFS_INSTALL_DIR)/$(PROGRAM).prc

ifeq ($(OSNAME),msdosdjgpp)
# Static build: the application code goes into $(BIN)/lib$(PROGRAM).a (linked
# into pumpkin.exe by src/msdos) and the PRC only carries the resources. The
# installed PRC gets an 8.3 file name (the database name comes from the PRC
# header, not from the file name).
STATIC_APP=1
PRC=$(VFS)/$(VFS_INSTALL_DIR)/$(shell printf '%.8s' $(PROGRAM)).prc
else ifeq ($(OSNAME),vita)
# Static build as well (linked into eboot.bin by src/vita), regular file names
STATIC_APP=1
else ifeq ($(OSNAME),switch)
# Static build as well (linked into PumpkinOS.nro by src/switch), regular file names
STATIC_APP=1
else ifeq ($(OSNAME),psp)
# Static build as well (linked into EBOOT.PBP by src/psp), regular file names
STATIC_APP=1
else ifeq ($(OSNAME),ps2)
# Static build as well (linked into PUMPKINOS.ELF by src/ps2), regular file names
STATIC_APP=1
else ifeq ($(OSNAME),gamecube)
# Static build as well (linked into pumpkinos.dol by src/gamecube), regular file names
STATIC_APP=1
else ifeq ($(OSNAME),wii)
# Static build as well (linked into boot.dol by src/wii), regular file names
STATIC_APP=1
endif

ifeq ($(STATIC_APP),1)
ifneq ($(OBJS),)
APPLIB=$(BIN)/lib$(PROGRAM).a
endif

$(PROGRAM).prc: $(APPLIB) $(RESFLAG) $(RESOPT)
	@echo Building $(PROGRAM).prc
	@$(PRCBUILD) -f $(PROGRAM).prc -t $(APPTYPE) -c $(APPID) -n "$(APPNAME)" resources/*
	@echo Installing $(PROGRAM).prc
	@cp $(PROGRAM).prc $(PRC)

$(APPLIB): $(OBJS)
	@echo Creating lib$(PROGRAM).a
	@rm -f $(APPLIB)
	@$(AR) -cr $(APPLIB) $(OBJS)
else
$(PROGRAM).prc: $(DLIB_OPT) $(RESFLAG) $(RESOPT)
	@echo Building $(PROGRAM).prc
	@$(PRCBUILD) -f $(PROGRAM).prc -t $(APPTYPE) -c $(APPID) -n "$(APPNAME)" $(DLIB_OPT) resources/*
	@echo Installing $(PROGRAM).prc
	@cp $(PROGRAM).prc $(PRC)
endif

$(DLIB): $(PROGRAM).dlib
	@cp $(PROGRAM).dlib $(DLIB)
	@$(STRIP) $(DLIB)

ifeq ($(OSNAME),Darwin)
$(PROGRAM).dlib: $(OBJS) $(STUBS_OPT)
	@echo Linking $(PROGRAM).dlib
	@$(CC) -dynamiclib -o $(PROGRAM).dlib $(OBJS) $(STUBS_OPT) $(LIBS)
	@codesign -f -s "Developer ID Application" $(PROGRAM).dlib
else ifeq ($(OSNAME),Msys)
$(PROGRAM).dlib: $(OBJS) $(STUBS_OPT)
	@echo Linking $(PROGRAM).dlib
	@$(CC) -shared -o $(PROGRAM).dlib $(OBJS) $(STUBS_OPT) $(LIBS)
$(PROGRAM).dlib: $(OBJS) $(STUBS_OPT)
else
$(PROGRAM).dlib: $(OBJS) $(STUBS_OPT)
	@echo Linking $(PROGRAM).dlib
	@$(CC) $(OSDEFS) -shared -o $(PROGRAM).dlib $(OBJS) $(STUBS_OPT) $(LIBS)
endif

$(STUBS_OPT): $(STUBS).c
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c $(STUBS).c

$(STUBS).c: $(OBJS)
	@echo Generating $<
	@$(NM) $(OBJS) | sort | uniq > $(STUBS).txt
	@$(LIBPUMPKIN)/gen_stubs.sh $(STUBS).txt $(LIBPUMPKIN)/traps.txt > $(STUBS).c

res.flag: $(RCP) $(RCPDEP)
	@echo Compiling resources
	@mkdir -p resources
	@$(PILRC) $(RCPINC) $(RCP) resources
	@touch res.flag

clean:
	@rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(OBJS) $(DLIB) $(APPLIB) $(STUBS).c $(STUBS_OPT) $(STUBS).txt resources/*.bin *.flag *.mod
