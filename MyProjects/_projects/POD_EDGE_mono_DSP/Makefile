TARGET = EDGE_mono_DSP

CPP_SOURCES = main.cpp \
              display.cpp \
              ui_state.cpp \
              ../../DAFX_2_Daisy_lib/src/effects/tonestack.cpp

# Include path for external OLED / SSD1306 driver
C_INCLUDES += -I$(LIBDAISY_DIR)/src/

# Include DAFX library for fdn_reverb (no LGPL flag needed)
C_INCLUDES += -I../../DAFX_2_Daisy_lib/src/effects

LIBDAISY_DIR     = ../../../libDaisy
DAISYSP_DIR      = ../../../DaisySP

# Uncomment if ReverbSc or MoogLadder are added later
# USE_DAISYSP_LGPL = 1

SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
