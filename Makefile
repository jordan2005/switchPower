
CROSS_COMPILE = arm-linux-
AS		= $(CROSS_COMPILE)as   #汇编
LD		= $(CROSS_COMPILE)ld   #链接
CC		= $(CROSS_COMPILE)gcc  #编译
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar  #库管理
NM		= $(CROSS_COMPILE)nm

STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

export AS LD CC CPP AR NM
export STRIP OBJCOPY OBJDUMP

CFLAGS := -Wall -O2 -g
CFLAGS += -I $(shell pwd)
CFLAGS += -I $(shell pwd)/comm
CFLAGS += -I $(shell pwd)/control
CFLAGS += -I $(shell pwd)/include
CFLAGS += -I $(shell pwd)/modbus
CFLAGS += -I $(shell pwd)/modbus/ascii
CFLAGS += -I $(shell pwd)/modbus/functions
CFLAGS += -I $(shell pwd)/modbus/include
CFLAGS += -I $(shell pwd)/modbus/rtu
CFLAGS += -I $(shell pwd)/modbus/tcp
CFLAGS += -I $(shell pwd)/port


#LDFLAGS := -lm -lts -lpthread
LDFLAGS := -lm -lpthread -lsqlite3

export CFLAGS LDFLAGS

TOPDIR := $(shell pwd)
export TOPDIR

TARGET := power


obj-y += main.o
obj-y += control/
obj-y += comm/
obj-y += modbus/
obj-y += port/

all : 
	make -C ./ -f $(TOPDIR)/Makefile.build  #执行当前目录下Makefile.build
	$(CC) $(LDFLAGS) -o $(TARGET) built-in.o


clean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(TARGET)