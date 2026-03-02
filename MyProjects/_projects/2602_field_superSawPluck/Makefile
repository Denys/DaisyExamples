# Project Name
TARGET = Super_Saw_Pluck

# Sources
CPP_SOURCES = Super_Saw_Pluck.cpp

# Library Locations
LIBDAISY_DIR = ../../../libDaisy
DAISYSP_DIR = ../../../DaisySP

USE_DAISYSP_LGPL = 1
C_INCLUDES += -I$(DAISYSP_DIR)/DaisySP-LGPL/Source
C_INCLUDES += -I$(DAISYSP_DIR)/Source/Utility
CPP_SOURCES += $(DAISYSP_DIR)/DaisySP-LGPL/Source/Effects/reverbsc.cpp
CPP_SOURCES += $(DAISYSP_DIR)/DaisySP-LGPL/Source/Filters/moogladder.cpp
# Core location, and generic Makefile
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
