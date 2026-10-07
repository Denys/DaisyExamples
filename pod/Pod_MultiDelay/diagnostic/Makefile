TARGET = Pod_MultiDelay_Test
DEMO_DIR ?= ..
DAISY_EXAMPLES_DIR ?= C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples
LIBDAISY_DIR = $(DAISY_EXAMPLES_DIR)/libDaisy
CPP_SOURCES = Pod_MultiDelay_Test.cpp $(DAISY_EXAMPLES_DIR)/DaisyHost/src/PedalDelayEngine.cpp
C_INCLUDES += -I$(DEMO_DIR) -I$(DAISY_EXAMPLES_DIR)/DaisyHost/include
CPP_STANDARD = -std=gnu++17
OPT = -Os
APP_TYPE = BOOT_NONE
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
