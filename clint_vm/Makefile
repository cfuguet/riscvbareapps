TARGET = clint_vm
OBJS = clint_vm.o vm_init.o
CFLAGS += -O2
CFLAGS += -I$(RVB_HOME)/drivers/clint/include

ifeq ($(DEBUG),1)
CFLAGS += -DVM_DEBUG
endif

include $(RVB_BUILD)/makefile.include
