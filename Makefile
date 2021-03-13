MUSL_BASE_DIR =
MUSL_PATH =
MUSL_TARGET =
MUSL_CROSS = cross

MUSL_BIN_DIR = 

PREFIX = 
CC = gcc
AR = ar

SRC_DIR = src/
INC_DIR = include/

BIN_DIR = bin/
LIB_DIR = lib/
OBJ_DIR = obj/

SRC =
OUT ?= cstar

CVERSION = c99

CFLAGS = 
LDFLAGS = 

-include config.mak

ifeq ($(CFLAGS),)
    CFLAGS = -Wall -std=$(CVERSION) -static -O2
endif

INCLUDE += -I$(INC_DIR) -I$(SRC_DIR)

ifneq ($(MUSL_TARGET),)
    PREFIX = $(if $(MUSL_TARGET),$(MUSL_TARGET)-,)
    MUSL_PATH = $(MUSL_BASE_DIR)/$(PREFIX)$(MUSL_CROSS)
endif

ifneq ($(MUSL_PATH),)
    MUSL_BIN_DIR = $(MUSL_PATH)/bin/
    MUSL_INCLUDE_FOLDER = $(MUSL_PATH)/$(MUSL_TARGET)/include/
endif

LIBNAME ?= lib$(OUT)
DLIBNAME ?= $(LIBNAME).so
SLIBNAME ?= $(LIBNAME).a

ifdef MUSL_INCLUDE_FOLDER
    INCLUDE += -I$(MUSL_INCLUDE_FOLDER)
endif

ifeq ($(OS),Windows_NT)
    export Path := $(MUSL_BIN_DIR);$(Path)
else
    export PATH := $(MUSL_BIN_DIR):$(PATH)
endif

CROSS_CC = $(PREFIX)$(CC)
CROSS_AR = $(PREFIX)$(AR)

ifeq ($(SRC),)
    SRC := $(wildcard $(SRC_DIR)/*.c)  
endif

O_DIR = $(dir $(OBJ_DIR))
L_DIR = $(dir $(LIB_DIR))
B_DIR = $(dir $(BIN_DIR))

OBJ = $(SRC:%.c=$(O_DIR)/%.o)

SOBJ = $(OBJ:%.o=%.s.o)
DOBJ = $(OBJ:%.o=%.d.o)

LDFLAGS += -L$(LIB_DIR) -l$(OUT)
FOLDERS = $(BIN_DIR) $(LIB_DIR) $(OBJ_DIR)


.PHONY: all
.SECONDARY: $(SOBJ) $(DOBJ)

all: setup $(L_DIR)/$(SLIBNAME) $(L_DIR)/$(DLIBNAME) $(B_DIR)/$(OUT).bin

$(FOLDERS):
	@mkdir -p $@

setup: $(FOLDERS)

cstar: cstar.c
	gcc $< -o cstar

%.bin:
	@echo "********************************************************"
	@echo "** COMPILING $@" 
	@echo "********************************************************"
	$(CROSS_CC) main.c -o $@ $(CFLAGS) $(INCLUDE) $(LDFLAGS) 
	@echo ""

%.a: $(SOBJ)
	@echo "********************************************************"
	@echo "** CREATING $@"
	@echo "********************************************************"
	$(CROSS_AR) rcs $@ $(SOBJ)
	@echo ""

%.so: $(DOBJ)
	@echo "********************************************************"
	@echo "** CREATING $@"
	@echo "********************************************************"
	$(CROSS_CC) -shared -o $@ $(DOBJ)
	@echo ""

$(O_DIR)/%.s.o: %.c
	@echo "********************************************************"
	@echo "** $(SLIBNAME): COMPILING SOURCE $<"
	@echo "********************************************************"
	@mkdir -p '$(@D)'
	$(CROSS_CC) -c $< -o $@ $(CFLAGS) $(INCLUDE) $(LDFLAGS)

$(O_DIR)/%.d.o: %.c
	@echo "********************************************************"
	@echo "** $(DLIBNAME): COMPILING SOURCE $<"
	@echo "********************************************************"
	@mkdir -p '$(@D)'
	$(CROSS_CC) -c $< -o $@ -fPIC $(CFLAGS) $(INCLUDE) $(LDFLAGS)

clean:
	rm -rf $(OUT)
	rm -rf $(DLIBNAME) $(SLIBNAME)
	rm -rf $(FOLDERS)
