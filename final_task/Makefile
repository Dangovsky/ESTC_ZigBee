BUILD_HOME=../../zboss-v1.0/
include $(BUILD_HOME)/Options

#CFLAGS_USER += -DIEEE_TEST

FNAME1 = zc
FNAME2 = zr

SRCS1 = $(FNAME1).c
SRCS2 = $(FNAME2).c
OBJS1 = ${SRCS1:.c=.o}
OBJS2 = ${SRCS2:.c=.o}

$(BIN)zc$(EXE) : $(OBJS1)  $(LIBS)
	$(LINK_BINARY) ./microrl/libmicrorl.a
	$(OBJCOPY) -O ihex $(FNAME1) $(FNAME1).hex
	$(OBJCOPY) -I ihex -O binary $(FNAME1).hex $(FNAME1).bin

$(BIN)zr$(EXE) : $(OBJS2)  $(LIBS)
	$(LINK_BINARY) 
	$(OBJCOPY) -O ihex $(FNAME2) $(FNAME2).hex
	$(OBJCOPY) -I ihex -O binary $(FNAME2).hex $(FNAME2).bin

BINS =  $(BIN)zc$(EXE)
BINS += $(BIN)zr$(EXE)

all: $(BINS)

libs:
	cd microrl && $(MAKE) rebuild

lclean:
	cd microrl && $(MAKE) clean

tags:
	($(FIND) . -name \*.[ch] -print) | grep -v ".*~" | ctags -L -
