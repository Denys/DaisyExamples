# Project Name
TARGET = field_wavetable_morph_synth

# Sources
CPP_SOURCES = main.cpp wavetable_osc.cpp morph_processor.cpp voice.cpp wavetables.cpp ui_handler.cpp midi_handler.cpp field_ux.cpp

# Library Locations
LIBDAISY_DIR = ../../../libDaisy
DAISYSP_DIR = ../../../DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
LDFLAGS += -u _printf_float
include $(SYSTEM_FILES_DIR)/Makefile