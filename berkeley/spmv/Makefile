VPATH = ../common
TARGET = spmv
DATASET ?= dataset1.h
OBJS = spmv.o util.o
CFLAGS = -I../common -O2 -funroll-loops -finline-functions -DDATASET=\"$(DATASET)\" -DPREALLOCATE=1

include $(RVB_BUILD)/makefile.include
