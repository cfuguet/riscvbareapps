VPATH = ../common
TARGET = memcpy_main
DATASET ?= datasets/dataset1.h
OBJS = memcpy_main.o util.o
CFLAGS = -I../common -O2 -funroll-loops -finline-functions -DDATASET=\"$(DATASET)\" -DPREALLOCATE=1

include $(RVB_BUILD)/makefile.include
