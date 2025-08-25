K ?= all
N ?= 1000
C ?= 100000

TARGET = rs
OBJS = RS_RISCVBARELIB.o RS_RISCVBARELIB_IMPL.o RS_Main.o RSOpts.o
EXTRA_INCLUDES = -I. -I../../../../include
EXTRA_LIBS=-lstdc++

CFLAGS += -D_ENABLE_RISCVBARELIB_
CFLAGS += -DSTREAM_KERNEL=$(K)
CFLAGS += -DSTREAM_ARRAY_SIZE=$(N)
CFLAGS += -DCYCLES_PER_SEC=$(C)
ifdef GDB
CFLAGS += -Og -ggdb
else
CFLAGS += -O2
endif

ifdef DEBUG
CFLAGS += -D_DEBUG_
endif

#CFLAGS += -D_INITARRAY_

VPATH += ../../

include $(RVB_BUILD)/makefile.include
