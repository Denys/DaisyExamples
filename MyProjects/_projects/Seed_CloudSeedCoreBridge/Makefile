# Project Name
TARGET = Seed_CloudSeedCoreBridge

# Sources
CPP_SOURCES = Seed_CloudSeedCoreBridge.cpp

# Use fallback engine by default. Set to 1 after CloudSeedCore import is wired.
CPPFLAGS += -DUSE_CLOUDSEEDCORE=0

# Library Locations
LIBDAISY_DIR = ../../../libDaisy
DAISYSP_DIR  = ../../../DaisySP

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
