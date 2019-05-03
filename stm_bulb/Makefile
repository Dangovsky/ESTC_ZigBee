BUILD_HOME=../../zboss-v1.0/
include $(BUILD_HOME)/Options

FNAME1 = zc
FNAME2 = ze
FNAME3 = ztest

SRCS1 = $(FNAME1).c
OBJS1 = ${SRCS1:.c=.o}
SRCS2 = $(FNAME2).c
OBJS2 = ${SRCS2:.c=.ed.o}
SRCS3 = $(FNAME3).c
OBJS3 = ${SRCS3:.c=.o}

$(BIN)zc$(EXE) : $(OBJS1)  $(LIBS) ./libled/libled.a ./libzbulb/libzbulb.a
	$(LINK_BINARY)
	$(OBJCOPY) -O ihex $(FNAME1) $(FNAME1).hex
	$(OBJCOPY) -I ihex -O binary $(FNAME1).hex $(FNAME1).bin

$(BIN)ze$(EXE) : $(OBJS2)  $(LIBS_ED) ./libbuttons/libbuttons.a	./libzbulb/libzbulb.a
	$(LINK_BINARY_ED)
	$(OBJCOPY) -O ihex $(FNAME2) $(FNAME2).hex
	$(OBJCOPY) -I ihex -O binary $(FNAME2).hex $(FNAME2).bin

$(BIN)ztest$(EXE) : $(OBJS3)  $(LIBS) ./libbuttons/libbuttons.a	./libzbulb/libzbulb.a ./libled/libled.a
	$(LINK_BINARY)
	$(OBJCOPY) -O ihex $(FNAME3) $(FNAME3).hex
	$(OBJCOPY) -I ihex -O binary $(FNAME3).hex $(FNAME3).bin

BINS =  $(BIN)zc$(EXE)
BINS += $(BIN)ze$(EXE)
BINS += $(BIN)ztest$(EXE)

all: $(BINS)

libs:
	cd libled     && $(MAKE) rebuild
	cd libzbulb   && $(MAKE) rebuild
	cd libbuttons && $(MAKE) rebuild

lclean:
	cd libled     && $(MAKE) clean
	cd libzbulb   && $(MAKE) clean
	cd libbuttons && $(MAKE) clean

burn_$(FNAME1): $(FNAME1).bin
	st-flash write $(FNAME1).bin 0x08000000

burn_$(FNAME2): $(FNAME2).bin
	st-flash write $(FNAME2).bin 0x08000000

tags:
	($(FIND) . -name \*.[ch] -print) | grep -v ".*~" | ctags -L -
