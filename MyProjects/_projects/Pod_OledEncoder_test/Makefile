# ============================================================
# PodTonestack_NlFilt — Daisy Pod + ToneStack + NlFilt Filter
# Prototype for Pod_OledEncoder_test
# ============================================================

TARGET = PodTonestack_NlFilt

# C++ sources
CPP_SOURCES = PodTonestack_NlFilt.cpp \
              $(DAFX_DIR)/src/effects/tonestack.cpp

# REQUIRED for LGPL modules (NlFilt)
USE_DAISYSP_LGPL = 1

# Additional include paths (libDaisy device drivers + DAFX lib)
C_INCLUDES += -I$(LIBDAISY_DIR)/src/ \
               -I$(LIBDAISY_DIR)/src/dev/ \
               -I$(DAISYSP_DIR)/Source/ \
               -I$(DAISYSP_DIR)/DaisySP-LGPL/Source/ \
               -I$(DAFX_DIR)/src/

# Library locations — 3 levels up from _projects/<ProjectName>/
LIBDAISY_DIR = ../../../libDaisy
DAISYSP_DIR  = ../../../DaisySP
DAFX_DIR     = ../../../MyProjects/DAFX_2_Daisy_lib

# Core Makefile
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
