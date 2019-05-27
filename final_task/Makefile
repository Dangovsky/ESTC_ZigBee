BUILD_HOME=../../zboss-v1.0/
include $(BUILD_HOME)/Options

FNAME1 = zc

SRCS1 = $(FNAME1).c
OBJS1 = ${SRCS1:.c=.o}

$(BIN)zc$(EXE) : $(OBJS1)  $(LIBS)
	$(LINK_BINARY) ./microrl/libmicrorl.a
	$(OBJCOPY) -O ihex $(FNAME1) $(FNAME1).hex
	$(OBJCOPY) -I ihex -O binary $(FNAME1).hex $(FNAME1).bin

BINS =  $(BIN)zc$(EXE)

all: $(BINS)

libs:
	cd microrl && $(MAKE) rebuild

lclean:
	cd microrl && $(MAKE) clean
tags:
	($(FIND) . -name \*.[ch] -print) | grep -v ".*~" | ctags -L -
