TARGET = Field_MI_Plaits

LIBDAISY_DIR = ../../../libDaisy
DAISYSP_DIR  = ../../../DaisySP

STMLIB_DIR = ../../../stmlib
PLAITS_ROOT = ../PlaitsPatchInit

CPP_SOURCES = Field_MI_Plaits.cpp
CC_SOURCES += $(wildcard $(PLAITS_ROOT)/eurorack/plaits/dsp/*.cc)
CC_SOURCES += \
$(PLAITS_ROOT)/eurorack/plaits/dsp/engine/additive_engine.cc \
$(PLAITS_ROOT)/eurorack/plaits/dsp/engine/bass_drum_engine.cc \
$(PLAITS_ROOT)/eurorack/plaits/dsp/engine/fm_engine.cc \
$(PLAITS_ROOT)/eurorack/plaits/dsp/engine/grain_engine.cc \
$(PLAITS_ROOT)/eurorack/plaits/dsp/engine/hi_hat_engine.cc \
$(PLAITS_ROOT)/eurorack/plaits/dsp/engine/particle_engine.cc \
$(PLAITS_ROOT)/eurorack/plaits/dsp/engine/snare_drum_engine.cc \
$(PLAITS_ROOT)/eurorack/plaits/dsp/engine/swarm_engine.cc \
$(PLAITS_ROOT)/eurorack/plaits/dsp/engine/virtual_analog_engine.cc
CC_SOURCES += $(PLAITS_ROOT)/eurorack/plaits/resources.cc

CC_SOURCES += $(STMLIB_DIR)/dsp/units.cc \
$(STMLIB_DIR)/utils/random.cc

C_INCLUDES += \
-I$(PLAITS_ROOT)/eurorack \
-I../../.. \
-Wno-unused-local-typedefs

OPT = -Os -s

CFLAGS += -flto
CPPFLAGS += -flto
LDFLAGS += -flto

SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CC_SOURCES:.cc=.o)))
vpath %.cc $(sort $(dir $(CC_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $(C_STANDARD) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	$(CXX) -c $(CPPFLAGS) $(CPP_STANDARD) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.cc Makefile | $(BUILD_DIR)
	$(CXX) -c $(CPPFLAGS) $(CPP_STANDARD) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cc=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@
