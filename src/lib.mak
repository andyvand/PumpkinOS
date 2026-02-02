ifeq ($(STATIC),1)
OBJS=$(SOURCE:%.c=%.o)
EXT=.a
ARCMD=$(AR) -cr
else ifeq ($(STATIC),2)
OBJS=$(SOURCE:%.c=%.wasm)
EXT=.a
ARCMD=$(EM_AR) -cru
else
OBJS=$(SOURCE:%.c=%.o)
EXT=$(SOEXT)
endif

LIB=$(BIN)/$(PROGRAM)

$(LIB)$(EXT):

ifeq ($(OSNAME),Msys)
$(LIB)$(SOEXT): $(EXTDEPS) $(OBJS)
	@echo Linking $(PROGRAM)$(SOEXT)
	@$(CC) -shared -o $(LIB)$(SOEXT) -Wl,--out-implib,$(LIB).a $(OBJS) -L$(BIN) -lpit $(LIBS) --def $(LIB).def
	@$(STRIP) $(LIB)$(SOEXT)
else ifeq ($(OSNAME),Darwin)
$(LIB)$(SOEXT): $(EXTDEPS) $(OBJS)
	@echo Linking $(PROGRAM)$(SOEXT)
	@$(CC) -dynamiclib -o $(LIB)$(SOEXT) $(OBJS) -L$(BIN) -lpit $(LIBS)
	@$(STRIP) $(LIB)$(SOEXT)
else
$(LIB)$(SOEXT): $(EXTDEPS) $(OBJS)
	@echo Linking $(PROGRAM)$(SOEXT)
	@$(CC) -shared -o $(LIB)$(SOEXT) $(OBJS) -L$(BIN) -lpit $(LIBS)
	@$(STRIP) $(LIB)$(SOEXT)
endif

$(LIB).a: $(OBJS)
	@echo Creating $(PROGRAM).a
	@$(ARCMD) $(LIB).a $(OBJS)

clean: $(EXTCLEANDEPS)
	@rm -f $(LIB)$(SOEXT) $(LIB).a $(OBJS) $(EXTCLEAN)
