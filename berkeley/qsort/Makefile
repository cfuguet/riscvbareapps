VPATH = ../common
TARGET = qsort_main
DATASET ?= dataset1.h
OBJS = qsort_main.o util.o
CFLAGS = -I../common -O2 -funroll-loops -finline-functions -DDATASET=\"$(DATASET)\" -DPREALLOCATE=1

include $(RVB_BUILD)/makefile.include
