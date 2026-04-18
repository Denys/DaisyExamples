# Project Name
TARGET = CloudSeed

# Sources
CPP_SOURCES = \
CloudSeed.cpp \
../../third_party/CloudSeedCore/Parameters.cpp \
../../third_party/CloudSeedCore/DSP/Biquad.cpp \
../../third_party/CloudSeedCore/DSP/RandomBuffer.cpp

# Library Locations
LIBDAISY_DIR = ../../libDaisy/
DAISYSP_DIR = ../../DaisySP/

C_DEFS += -DBUFFER_SIZE=48 -DMAX_STR_SIZE=32
OPT = -Os

C_INCLUDES += \
-I../../third_party/CloudSeedCore \
-I../../third_party/CloudSeedCore/DSP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
