TARGET = Pod_MultiDelay
DAISY_EXAMPLES_DIR ?= C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples
LIBDAISY_DIR = $(DAISY_EXAMPLES_DIR)/libDaisy
CPP_SOURCES = Pod_MultiDelay.cpp $(DAISY_EXAMPLES_DIR)/DaisyHost/src/PedalDelayEngine.cpp
C_INCLUDES += -I$(DAISY_EXAMPLES_DIR)/DaisyHost/include
CPP_STANDARD = -std=gnu++17
OPT = -Os
APP_TYPE = BOOT_NONE
# Preparation is build-only. A hardware run must use a separately reviewed command.
ifneq ($(filter program program-dfu,$(MAKECMDGOALS)),)
$(error This demonstrator preparation target does not authorize flashing)
endif
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
