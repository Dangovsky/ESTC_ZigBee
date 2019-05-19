BUILD_HOME=../../../zboss-v1.0
include $(BUILD_HOME)/Options

CFLAGS_USER += -DBUTTONS_TIMER

SRCS1 = ./src/buttons.c
OBJS1 = ${SRCS1:.c=.o}

all: libbuttons.a

libbuttons.a: $(OBJS1)
	$(MAKE_LIB)
