# Project Name
TARGET = Pod_MarkovAIDrummer

# Sources
CPP_SOURCES = Pod_MarkovAIDrummer.cpp

# Library Locations
LIBDAISY_DIR = ../../../libDaisy
DAISYSP_DIR = ../../../DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

# Tolerant DFU programming target for Daisy on some Windows setups.
# If dfu-util exits with 74 after successful transfer (device leaves DFU mode),
# treat it as success.
program-dfu-safe:
	@set -e; \
	dfu-util -a 0 -s $(FLASH_ADDRESS):leave -D $(BUILD_DIR)/$(TARGET_BIN) -d ,0483:$(USBPID) || { \
		code=$$?; \
		if [ $$code -eq 74 ]; then \
			echo "Ignoring dfu-util exit code 74 after successful download"; \
			exit 0; \
		else \
			exit $$code; \
		fi; \
	}
