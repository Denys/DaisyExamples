# Project Name
TARGET = daisy_dual_stft

# Sources
CPP_SOURCES = daisy_dual_stft.cpp

# Library Locations
LIBDAISY_DIR ?= ../../../libDaisy
DAISYSP_DIR ?= ../../../DaisySP

CMSIS_DSP_SOURCE_DIR = $(LIBDAISY_DIR)/Drivers/CMSIS/DSP/Source
C_SOURCES += \
$(CMSIS_DSP_SOURCE_DIR)/CommonTables/arm_common_tables.c \
$(CMSIS_DSP_SOURCE_DIR)/TransformFunctions/arm_bitreversal.c \
$(CMSIS_DSP_SOURCE_DIR)/TransformFunctions/arm_cfft_f32.c \
$(CMSIS_DSP_SOURCE_DIR)/TransformFunctions/arm_cfft_radix8_f32.c \
$(CMSIS_DSP_SOURCE_DIR)/TransformFunctions/arm_rfft_fast_f32.c \
cmsis_bitreversal_32.c

# DaisyDAFX headers
C_INCLUDES += -I../../src

# Example options:
#   make STFT_DEFS="-DSTFT_BACKEND_FAST -DSTFT_PROFILE_512"
#   make STFT_DEFS="-DSTFT_BACKEND_DAFX"
#   make STFT_DEFS="-DSTFT_PROFILE_1024"
#   make STFT_DEFS="-DSTFT_BACKEND_DAFX -DSTFT_PROFILE_1024"
STFT_DEFS ?=
C_DEFS += $(STFT_DEFS)

# Linker flags for floating-point CPU log output.
LDFLAGS = -u _printf_float

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
