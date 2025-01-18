TARGET = linpack
N ?= 1000
OBJS = linpack.o
CFLAGS = -DN=$(N) -O2 -funroll-loops -finline-functions
EXTRA_LIBS = -lm

include $(RVB_BUILD)/makefile.include
