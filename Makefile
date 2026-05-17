## CoD1 MP (decomp) simple build Makefile
## Usage:
##   mingw32-make -j%NUMBER_OF_PROCESSORS%  (on Windows)
##   make -j$(nproc)                        (on Linux/WSL)
##   make clean
##
## Notes:
## - Defaults to native MinGW gcc on Windows, or cross-compiler on Linux.
## - Override toolchain if needed:
##     make CC=gcc CXX=g++ TARGET=CoDMP

# 1. Platform Detection and Toolchain Defaults
ifeq ($(OS),Windows_NT)
  DEFAULT_CC := gcc
  DEFAULT_CXX := g++
  DEFAULT_WINDRES := windres
  DEFAULT_STRIP := strip
else
  DEFAULT_CC := i686-w64-mingw32-gcc
  DEFAULT_CXX := i686-w64-mingw32-g++
  DEFAULT_WINDRES := i686-w64-mingw32-windres
  DEFAULT_STRIP := i686-w64-mingw32-strip
endif

# Check origin of toolchain variables to override default/predefined values
ifeq ($(origin CC),default)
  CC := $(DEFAULT_CC)
endif
ifeq ($(origin CC),undefined)
  CC := $(DEFAULT_CC)
endif

ifeq ($(origin CXX),default)
  CXX := $(DEFAULT_CXX)
endif
ifeq ($(origin CXX),undefined)
  CXX := $(DEFAULT_CXX)
endif

ifeq ($(origin WINDRES),default)
  WINDRES := $(DEFAULT_WINDRES)
endif
ifeq ($(origin WINDRES),undefined)
  WINDRES := $(DEFAULT_WINDRES)
endif

ifeq ($(origin STRIP),default)
  STRIP := $(DEFAULT_STRIP)
endif
ifeq ($(origin STRIP),undefined)
  STRIP := $(DEFAULT_STRIP)
endif

CSTD ?= gnu99

# Auto-detect if we are compiling for Windows / MinGW
ifeq ($(OS),Windows_NT)
  IS_MINGW := 1
else
  CC_BASENAME := $(notdir $(CC))
  ifneq (,$(findstring mingw,$(CC_BASENAME)))
    IS_MINGW := 1
  else
    IS_MINGW := 0
  endif
endif

ifeq ($(IS_MINGW),1)
  TARGET ?= CoDMP.exe
else
  TARGET ?= CoDMP
endif

# Common flags for early decomp stage: permissive, no PIE, keep frame pointers.
CFLAGS ?= -std=$(CSTD) -m32 -w -O0 -g -fno-omit-frame-pointer -fno-pie -no-pie \
	-Wno-error=incompatible-pointer-types \
	-Wno-error=implicit-function-declaration \
	-Wno-error=implicit-int \
	-Wno-error=int-conversion \
	-I.

CPPFLAGS ?=
LDFLAGS ?= -m32 -fno-pie -no-pie
ifeq ($(IS_MINGW),1)
  LIBS ?= -lopengl32 -lwinmm -lws2_32 -luser32 -lgdi32 -lole32 -luuid -lcomdlg32 -lcomctl32 -lshlwapi -ladvapi32 -lversion -lz
else
  LIBS ?= -lm -ldl -lpthread -lz
endif

# 2. OS-independent recursive wildcard function (pure GNU Make)
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC_DIRS := PC Mac/Main
ROOT_C_SRCS := $(wildcard *.c)

# Find all C and assembly sources under source directories
ALL_C_SRCS := $(foreach dir,$(SRC_DIRS),$(call rwildcard,$(dir),*.c))
ALL_S_SRCS := $(foreach dir,$(SRC_DIRS),$(call rwildcard,$(dir),*.S))

# Exclude Mac-only sources (inside PC/mac)
C_SRCS := $(sort $(filter-out PC/mac/%,$(ALL_C_SRCS)) $(ROOT_C_SRCS))
S_SRCS := $(sort $(filter-out PC/mac/%,$(ALL_S_SRCS)))

OBJS := $(C_SRCS:.c=.o) $(S_SRCS:.S=.o)
COMPILER_STAMP := .compiler-$(notdir $(CC)).stamp

.PHONY: all clean list count

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) -Wl,--allow-multiple-definition

$(COMPILER_STAMP):
	@rm -f .compiler-*.stamp
	@echo > $@

%.o: %.c $(COMPILER_STAMP)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o: %.S $(COMPILER_STAMP)
	$(CC) -m32 -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS) .compiler-*.stamp

list:
	@printf '%s\n' $(C_SRCS) $(S_SRCS)

count:
	@echo "C sources: $$(printf '%s\n' $(C_SRCS) | sed '/^$$/d' | wc -l)"
	@echo "S sources: $$(printf '%s\n' $(S_SRCS) | sed '/^$$/d' | wc -l)"
	@echo "Object files: $$(printf '%s\n' $(OBJS) | sed '/^$$/d' | wc -l)"
