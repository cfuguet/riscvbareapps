TARGET = coremark
OBJS = core_portme.o \
	   core_list_join.o \
	   core_main.o \
	   core_matrix.o \
	   core_state.o \
	   core_util.o

vpath %.c ..

__CFLAGS += -I. -I.. -O2 -funroll-loops

ifndef CLOCKS_PER_SEC
  CLOCKS_PER_SEC=1000000
endif
__CFLAGS += -DCLOCKS_PER_SEC=$(CLOCKS_PER_SEC)

ifndef ITERATIONS
  ITERATIONS=40
endif
__CFLAGS += -DITERATIONS=$(ITERATIONS)

ifndef PERFORMANCE_RUN
  ifndef PROFILE_RUN
    __CFLAGS += -DVALIDATION_RUN=1
  else
    __CFLAGS += -DPROFILE_RUN=1
  endif
else
  __CFLAGS += -DPERFORMANCE_RUN=1
endif

CFLAGS += $(__CFLAGS) -DFLAGS_STR="\"$(__CFLAGS)\""

include $(RVB_BUILD)/makefile.include