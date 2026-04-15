# Project Name
TARGET = Pod_MultiFX_Chain

# Sources
CPP_SOURCES = Pod_MultiFX_Chain.cpp

# Add DAFX_2_Daisy_lib
C_INCLUDES += -I../../DAFX_2_Daisy_lib/src
CPP_SOURCES += ../../DAFX_2_Daisy_lib/src/effects/tube.cpp

# Library Locations
LIBDAISY_DIR = ../../../libDaisy
DAISYSP_DIR = ../../../DaisySP

# Enable LGPL modules (ReverbSc)
USE_DAISYSP_LGPL = 1

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
