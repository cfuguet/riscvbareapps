VPATH = ../common
TARGET = spmv
DATASET ?= dataset1.h
OBJS = spmv.o util.o
PREALLOCATE = 0
CFLAGS = -I../common -O2 -funroll-loops -finline-functions -DDATASET=\"$(DATASET)\" -DPREALLOCATE=$(PREALLOCATE)

all: $(DATASET)

include $(RVB_BUILD)/makefile.include
