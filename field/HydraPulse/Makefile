# Run from DaisyExamples/field/HydraPulse. Default is the diagnostic image, not the instrument.
APP ?= truth
LIBDAISY_DIR ?= ../../libDaisy
CPP_STANDARD = -std=gnu++17
OPT = -Os -fno-fast-math -ffp-contract=off
APP_TYPE ?= BOOT_NONE
override BUILD_DIR = build-$(APP)
C_INCLUDES += -I.
BUILD_ID ?= unrecorded
C_DEFS += -DHPF_BUILD_ID=\"$(BUILD_ID)\"

ifeq ($(APP),truth)
TARGET = HydraPulseTruth
CPP_SOURCES = firmware/FieldTruth.cpp
else ifeq ($(APP),beat)
TARGET = HydraPulseBeat
CPP_SOURCES = firmware/HydraPulse.cpp src/dsp/Voices.cpp src/app/Engine.cpp src/app/Controller.cpp
else
$(error APP must be truth or beat)
endif

SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
ifeq ($(wildcard $(SYSTEM_FILES_DIR)/Makefile),)
$(error libDaisy core/Makefile missing; use tools/build_target.py with an inspected checkout)
endif
include $(SYSTEM_FILES_DIR)/Makefile
# BUILD_DIR is an explicit GNU make override so libDaisy cannot merge variant objects.

# Keep public BSP enum layouts consistent with the libDaisy library build.
# The upstream application makefile adds -fshort-enums; our library qualification
# build uses default-width enums. The later option wins without editing upstream.
CPPFLAGS += -fno-short-enums

# The pinned upstream SWD recipe hardcodes ./build. Keep the actual variant path.
# This intentional recipe override is local; never edit libDaisy for this project.
ifeq ($(APP_TYPE),BOOT_NONE)
program: $(BUILD_DIR)/$(TARGET).elf
	$(OCD) -s $(OCD_DIR) $(OCDFLAGS) -c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"
endif
