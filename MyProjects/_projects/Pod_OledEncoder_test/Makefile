# ============================================================
# ExtControls — Daisy Pod + External OLED-Rotary-Encoder
# ============================================================

TARGET = ExtControls

# C++ sources
CPP_SOURCES = ExtControls.cpp

# Additional include paths (libDaisy device drivers)
C_INCLUDES += -I$(LIBDAISY_DIR)/src/ \
              -I$(LIBDAISY_DIR)/src/dev/

# Library locations — 3 levels up from _projects/<ProjectName>/
LIBDAISY_DIR = ../../../libDaisy
DAISYSP_DIR  = ../../../DaisySP

# Core Makefile
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
