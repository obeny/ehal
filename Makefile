#
# INCLUDE CONFIGURATION
#
ifneq ("$(wildcard local.cnf)", "")
include local.cnf
endif

include Makefile.cnf

#
# GLOBAL DEFINITIONS
#

# paths to tool executables
AS=$(ARCH_PREFIX)as
CPP=$(ARCH_PREFIX)cpp
OBJCOPY=$(ARCH_PREFIX)objcopy
OBJDUMP=$(ARCH_PREFIX)objdump
LD=$(ARCH_PREFIX)ld
NM=$(ARCH_PREFIX)nm
SIZE=$(ARCH_PREFIX)size
GDB=$(ARCH_PREFIX)gdb

SHELL=/bin/bash

CC=$(ARCH_PREFIX)gcc
CXX=$(ARCH_PREFIX)g++

# target and dependencies for compilation modules
CSRCS=$(wildcard src/*.c) $(wildcard src/ext/*.c)
XSRCS=$(wildcard src/*.cxx) $(wildcard src/ext/*.cxx)
ASRCS=$(wildcard src/*.S) $(wildcard src/ext/*.S)

COBJS=$(subst src,obj,$(subst .c,.o,$(CSRCS)))
XOBJS=$(subst src,obj,$(subst .cxx,.o,$(XSRCS)))
AOBJS=$(subst src,obj,$(subst .S,.o,$(ASRCS)))

OBJS=$(AOBJS) $(COBJS) $(XOBJS)

# target name and versioning information
TARGET=$(shell basename `pwd`)
VERSION=$(shell echo "$(TARGET)" | cut -d "-" -f 2-)
BUILD=build:$(shell date +%Y%m%d)

# color definitions for unix
ifneq ($(COLORS),no)
C_BLUE=\033[1;34m
C_GREEN=\033[1;32m
C_RED=\033[1;31m
C_YELLOW=\033[1;33m
C_NC=\033[0m
endif

#
# COMPILATION FLAGS
#

# assembly listings
CFLAGS_LST_C=-Wa,-adhlns=$(<:src/%.c=obj/%.lst) 
CFLAGS_LST_CXX=-Wa,-adhlns=$(<:src/%.cxx=obj/%.lst) 
CFLAGS_LST_ASM=-Wa,-adhlns=$(<:src/%.S=obj/%.lst)

# base compiler flags
COMP_FLAGS_BASE=-I./include -I./$(LIB_VARIANT) $(CFLAGS_INC)
COMP_FLAGS_BASE+=-pipe -DFVERSION="\"$(VERSION)\"" -DFBUILD="\"$(BUILD)\""
COMP_FLAGS_BASE+=$(CFLAGS_MCU) $(CFLAGS_ARCH)
# module dependencies
COMP_FLAGS_BASE+=-MD -MP -MF .dep/$(@F).d
# color gcc output
ifneq ($(COLORS),no)
COMP_FLAGS_BASE+=-fdiagnostics-color=auto
endif

# C module compilation flags
CFLAGS=$(COMP_FLAGS_BASE)
CFLAGS+=$(CFLAGS_LST_C) $(CFLAGS_STD) $(CFLAGS_WRN) $(CFLAGS_CUSTOM)

# C++ module compilation flags
CXXFLAGS=$(COMP_FLAGS_BASE)
ifneq ($(CPP_EXCEPTIONS),yes)
CXXFLAGS+=-fno-exceptions
endif
ifneq ($(CPP_RTTI),yes)
CXXFLAGS+=-fno-rtti
endif

CXXFLAGS+=$(CFLAGS_LST_CXX) $(CXXFLAGS_STD) $(CXXFLAGS_WRN) $(CXXFLAGS_CUSTOM)

# default linker flags
LDFLAGS=-Wl,-Map=$(TARGET).map

# default assembler flags
ASFLAGS=$(CFLAGS_MCU) -Wa,-I./src,--warn $(CFLAGS_LST_ASM)

# default objcopy options
OBJCOPY_FLAGS=-R .comment -R .note

ifndef BUILD_DEBUG
# release build
CFLAGS+=$(CFLAGS_REL_OPT) $(CFLAGS_REL_DBG)
CXXFLAGS+=$(CFLAGS_REL_OPT) $(CFLAGS_REL_DBG)
LDFLAGS+=$(LDFLAGS_REL)
ASFLAGS+=$(ASFLAGS_REL)

BUILD_TYPE=$(C_GREEN)release$(C_NC)
else
# debug build
CFLAGS+=$(CFLAGS_DBG_OPT) $(CFLAGS_DBG_DBG) -DBUILD_DEBUG
CXXFLAGS+=$(CFLAGS_DBG_OPT) $(CFLAGS_DBG_DBG) -DBUILD_DEBUG
LDFLAGS+=$(LDFLAGS_DBG)
ASFLAGS+=$(ASFLAGS_DBG) -DBUILD_DEBUG

BUILD_TYPE=$(C_RED)debug$(C_NC)
endif

#
# BUILD RULES
#
all : begin elf lss sym info

# informational targets
begin :
	@echo -e "$(C_BLUE)>>>$(C_NC) STARTING $(BUILD_TYPE) BUILD OF: $(TARGET) $(C_BLUE)<<<$(C_NC)\n"
	@echo "using toolchain:"
	@echo -e "\tgcc:\t\t`$(CC) --version | head -n 1`"
	@echo -e "\tbinutils:\t`$(LD) -v | head -n 1`\n"
	@mkdir -p obj
	@mkdir -p obj/ext

info : arch
	@echo -e "\nbuilt binary sections size:"
	@$(SIZE) -A $(TARGET).elf

swupload : $(TARGET).bin
	@echo -e "Loading software to device..."
	mono $(LIB_VARIANT)/lib_mcu_noarch/_tools/sw_uploader.exe $(SWUPLOAD_PORT) $(TARGET).bin -n $(SWUPLOAD_OPTS)
# distribution targets
clean : clean_arch
	rm -fr obj/* $(TARGET) $(TARGET).* *.tar.bz2 .dep

dist : clean
	@echo -e "\nCREATED: $(TARGET).tar.bz2"
	$(shell cd .. ; tar --exclude=$(TARGET).tar.bz2 -cjf $(TARGET)/$(TARGET).tar.bz2 $(TARGET))

# sources compilation
$(COBJS) : obj/%.o : src/%.c
	@echo -n -e "$(C_BLUE)[CC]:$(C_NC) "
	$(CC) -c $< -o $@ $(CFLAGS)

$(XOBJS) : obj/%.o : src/%.cxx
	@echo -n -e "$(C_BLUE)[CXX]:$(C_NC) "
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(AOBJS) : obj/%.o : src/%.S
	@echo -n -e "$(C_BLUE)[ASM]:$(C_NC) "
	$(CC) -c $< -o $@ $(ASFLAGS)

objs : $(OBJS)

# linking
$(TARGET).elf : $(OBJS)
	@echo -n -e "$(C_GREEN)[LINK]:$(C_NC) "
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

# listing
$(TARGET).lss : $(TARGET).elf
	@echo -n -e "$(C_YELLOW)[LSS]:$(C_NC) "
	$(OBJDUMP) -h -S -z "$<" > $@
	
# symbol map
$(TARGET).sym : $(TARGET).elf
	@echo -n -e "$(C_YELLOW)[SYM]:$(C_NC) "
	$(NM) -n -l "$<" > $@
	
# helper targets
lss : $(TARGET).lss
sym : $(TARGET).sym

# include dependencies
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*.d)

#
# ARCHITECTURE SPECIFIC BUILD RULES
#
include Makefile.arch

.PHONY : all clean elf lss sym arch begin info program
