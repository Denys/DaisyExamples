# Project Name
TARGET = Field_delay_FunBox

# Sources
CPP_SOURCES = Field_delay_FunBox.cpp ../../../DaisyHost/src/DaisyDelayFxCore.cpp

# Library Locations
LIBDAISY_DIR = ../../../libDaisy
DAISYSP_DIR = ../../../DaisySP
C_INCLUDES += -I../../../DaisyHost/include
OPT = -Os

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
