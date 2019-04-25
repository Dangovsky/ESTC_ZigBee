BUILD_HOME=../../zboss-v1.0/
include $(BUILD_HOME)/Options

FNAME1 = zc
FNAME3 = ze

SRCS1 = $(FNAME1).c
OBJS1 = ${SRCS1:.c=.o}
SRCS3 = $(FNAME3).c
OBJS3 = ${SRCS3:.c=.ed.o}

$(BIN)zc$(EXE) : $(OBJS1)  $(LIBS) ./libled/libled.a ./libzbulb/libzbulb.a
	$(LINK_BINARY) -lm
	$(OBJCOPY) -O ihex $(FNAME1) $(FNAME1).hex
	$(OBJCOPY) -I ihex -O binary $(FNAME1).hex $(FNAME1).bin

$(BIN)ze$(EXE) : $(OBJS3)  $(LIBS_ED) ./libbuttons/libbuttons.a	./libzbulb/libzbulb.a
	$(LINK_BINARY_ED)
	$(OBJCOPY) -O ihex $(FNAME3) $(FNAME3).hex
	$(OBJCOPY) -I ihex -O binary $(FNAME3).hex $(FNAME3).bin

BINS =  $(BIN)zc$(EXE)
BINS += $(BIN)ze$(EXE)
all: $(BINS)

burn_$(FNAME1): $(FNAME1).bin
	st-flash write $(FNAME1).bin 0x08000000

burn_$(FNAME3): $(FNAME3).bin
	st-flash write $(FNAME3).bin 0x08000000

tags:
	($(FIND) . -name \*.[ch] -print) | grep -v ".*~" | ctags -
