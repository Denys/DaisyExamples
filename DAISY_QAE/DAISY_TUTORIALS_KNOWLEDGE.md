# Daisy Tutorials Knowledge Base
## Comprehensive Reference from Official Electrosmith Documentation

> **Source**: [daisy.audio/tutorials](https://daisy.audio/tutorials/)

---

## 1. GPIO (General Purpose Input/Output)

> **Reference**: [GPIO Tutorial](https://daisy.audio/tutorials/_a1_Getting-Started-GPIO/)

### GPIO Initialization

```cpp
GPIO my_gpio;
my_gpio.Init(Pin p, Mode m, Pull pu, Speed sp);
```

### Pin Modes

| Mode | Purpose |
|------|---------|
| `Mode::INPUT` | Read digital signals (default) |
| `Mode::OUTPUT` | Write digital signals (push-pull) |
| `Mode::OUTPUT_OD` | Open-drain output (less common) |
| `Mode::ANALOG` | Connect to ADC/DAC peripherals |

### Pull Resistors

| Pull | Effect |
|------|--------|
| `Pull::NOPULL` | No resistor (default) |
| `Pull::PULLUP` | Idles at 3.3V, button to GND |
| `Pull::PULLDOWN` | Idles at 0V, button to 3.3V |

**Why use pull resistors?** Without them, an unconnected GPIO pin has undefined voltage ("floating"). Use `PULLUP` when button connects to GND, `PULLDOWN` when button connects to 3.3V.

### Speed Setting

| Speed | Use Case |
|-------|----------|
| `Speed::LOW` | Default, lowest noise (recommended) |
| `Speed::MEDIUM` | Faster switching |
| `Speed::HIGH` | High-speed applications |
| `Speed::VERY_HIGH` | Maximum slew rate |

**Rule**: Use lowest speed possible to reduce electrical noise.

---

## 2. Audio Processing

> **Reference**: [Audio Tutorial](https://daisy.audio/tutorials/_a3_Getting-Started-Audio/)

### Audio Fundamentals

| Parameter | Daisy Default | Range |
|-----------|---------------|-------|
| **Sample Rate** | 48kHz | 8kHz - 96kHz |
| **Bit Depth** | 24-bit (hardware) | 32-bit float (internal) |
| **Block Size** | 4 samples (template) | 1-256 samples |

### Setting Audio Parameters

```cpp
hw.SetAudioBlockSize(48);  // Samples per callback (1-256)
hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
```

**Available Sample Rates:**
- `SAI_8KHZ`, `SAI_16KHZ`, `SAI_32KHZ`
- `SAI_48KHZ` (default)
- `SAI_96KHZ` (high-fidelity)

### Audio Callback Timing

```
Max callback time = (1 / sample_rate) * block_size
Example: 48 samples @ 48kHz = 1ms max processing time
```

### ⚠️ What NOT to Do in Audio Callback

| Forbidden | Why |
|-----------|-----|
| `malloc` / dynamic allocation | Non-deterministic time |
| Blocking peripheral calls (DAC, SPI, I2C) | Takes too long |
| File I/O, SD card access | Unbounded time |
| `hw.PrintLine()` or serial output | Blocks on buffer |

**Solution: Use flags**
```cpp
volatile bool action_flag = false;

void AudioCallback(...) {
    if(some_event)
        action_flag = true;  // Just set flag, don't process
}

// In main loop:
while(1) {
    if(action_flag) {
        do_slow_thing();  // Safe here
        action_flag = false;
    }
}
```

### CPU Load Monitoring

```cpp
CpuLoadMeter cpuMeter;

void AudioCallback(...) {
    cpuMeter.OnBlockStart();
    // ... your DSP code ...
    cpuMeter.OnBlockEnd();
}

// In main loop (NOT callback!):
float load = cpuMeter.GetAvgCpuLoad();
hw.PrintLine("CPU: %d%%", (int)(load * 100));
```

---

## 3. ADC (Analog-to-Digital Converter)

> **Reference**: [ADC Tutorial](https://daisy.audio/tutorials/_a4_Getting-Started-ADCs/)

### Key Concepts

- Daisy uses STM32 ADC1 peripheral (16-bit, multiplexed)
- ADC scans all configured inputs in background (no CPU usage)
- Maximum 16 ADC channels

### Hardware Connection

```
Potentiometer wiring:
- Pin 1 → GND
- Pin 2 (wiper) → Daisy ADC pin
- Pin 3 → 3v3_A (analog 3.3V)
```

**Important**: Always connect DGND and AGND pins externally.

### ADC Channel Configuration

```cpp
AdcChannelConfig adc_config[2];
adc_config[0].InitSingle(seed::A0);  // First pot
adc_config[1].InitSingle(seed::A1);  // Second pot

hw.adc.Init(adc_config, 2);
hw.adc.Start();

// Reading values (0.0 - 1.0):
float pot1 = hw.adc.GetFloat(0);
float pot2 = hw.adc.GetFloat(1);
```

### Multiplexed Inputs

For more than 16 inputs, use external multiplexer (CD4051):
```cpp
adc_config[0].InitMux(seed::A0, 8, mux_pins);  // 8 inputs via mux
```

---

## 4. External SDRAM

> **Reference**: [SDRAM Tutorial](https://daisy.audio/tutorials/_a6_Getting-Started-External-SDRAM/)

### Specifications

| Property | Value |
|----------|-------|
| Size | 64 MB |
| Base Address | `0xC0000000` |
| Access Speed | Slower than internal RAM |
| Use Cases | Delay buffers, samples, loopers |

### Declaring SDRAM Variables

```cpp
// Normal memory (internal):
float my_buffer[1024];  // 4KB

// SDRAM (external) - shorthand macro:
float DSY_SDRAM_BSS my_buffer[1024];

// SDRAM - longform:
float __attribute__((section(".sdram_bss"))) my_buffer[1024];
```

### ⚠️ SDRAM Limitations

| Limitation | Solution |
|------------|----------|
| Must be global | Declare outside functions |
| No constructors | Initialize in `Init()` function |
| Undefined initial state | Zero or fill after `DaisySeed::Init()` |

### Example: Large Delay Buffer

```cpp
// Allocate 10 seconds of stereo audio @ 48kHz
#define DELAY_SAMPLES (48000 * 10 * 2)
float DSY_SDRAM_BSS delay_buffer[DELAY_SAMPLES];

void Init() {
    // Zero the buffer after hardware init
    memset(delay_buffer, 0, sizeof(delay_buffer));
}
```

---

## 5. SPI (Serial Peripheral Interface)

> **Reference**: [SPI Tutorial](https://daisy.audio/tutorials/_a8_Getting-Started-SPI/)

### SPI Configuration

```cpp
SpiHandle spi_handle;
SpiHandle::Config spi_conf;

spi_conf.peripheral = SpiHandle::Config::Peripheral::SPI_2;
spi_conf.mode = SpiHandle::Config::Mode::MASTER;
spi_conf.direction = SpiHandle::Config::Direction::TWO_LINES;
spi_conf.pin_config.mosi = seed::D10;
spi_conf.pin_config.miso = seed::D9;
spi_conf.pin_config.sclk = seed::D8;
spi_conf.pin_config.nss  = seed::D7;

spi_handle.Init(spi_conf);
```

### SPI Modes

| Mode | Description |
|------|-------------|
| `MASTER` | Daisy controls the bus (clock, NSS) |
| `SLAVE` | Daisy follows another device |

### Data Direction

| Direction | Description |
|-----------|-------------|
| `TWO_LINES` | Full duplex (MOSI→, ←MISO) |
| `TWO_LINES_TX_ONLY` | Simplex transmit only |
| `TWO_LINES_RX_ONLY` | Simplex receive only |
| `ONE_LINE` | Half duplex (bidirectional) |

### Blocking Transfer

```cpp
uint8_t tx_buffer[4] = {0, 1, 2, 3};
uint8_t rx_buffer[4];

spi_handle.BlockingTransmit(tx_buffer, 4);
spi_handle.BlockingReceive(rx_buffer, 4);
spi_handle.BlockingTransmitAndReceive(tx_buffer, rx_buffer, 4);
```

**⚠️ Warning**: Blocking calls wait for completion. Don't use in audio callback!

---

## 6. I2C (Inter-Integrated Circuit)

> **Reference**: [I2C Tutorial](https://daisy.audio/tutorials/_a9_Getting_Started-I2C/)

### I2C Configuration

```cpp
I2CHandle i2c;
I2CHandle::Config i2c_conf;

i2c_conf.periph = I2CHandle::Config::Peripheral::I2C_1;
i2c_conf.pin_config.sda = seed::B7;
i2c_conf.pin_config.scl = seed::B6;
i2c_conf.speed = I2CHandle::Config::Speed::I2C_400KHZ;
i2c_conf.mode = I2CHandle::Config::Mode::I2C_MASTER;

i2c.Init(i2c_conf);
```

### I2C Speeds

| Speed | Rate | Compatibility |
|-------|------|---------------|
| `I2C_100KHZ` | 100 kHz | All devices (standard) |
| `I2C_400KHZ` | 400 kHz | Most devices (fast) |
| `I2C_1MHZ` | ~886 kHz | Some devices (fast+) |

### I2C Modes

| Mode | Description |
|------|-------------|
| `I2C_MASTER` | Daisy initiates transactions |
| `I2C_SLAVE` | Daisy responds to external controller |

### Blocking Transfer

```cpp
uint8_t address = 0x48;  // Device address
uint8_t tx_data[2] = {0x01, 0x00};
uint8_t rx_data[2];

i2c.TransmitBlocking(address, tx_data, 2, 1000);  // 1000ms timeout
i2c.ReceiveBlocking(address, rx_data, 2, 1000);
```

---

## 7. Daisy Bootloader

> **Reference**: [Bootloader Tutorial](https://daisy.audio/tutorials/_a7_Getting-Started-Daisy-Bootloader/)

### Bootloader Advantages

| Feature | Benefit |
|---------|---------|
| Larger programs | Up to 480KB SRAM, ~8MB QSPI |
| Easy updates | SD card or USB drive |
| No debug probe needed | End-user can update via media |

### Memory Regions

| Region | Size | Speed |
|--------|------|-------|
| FLASH (internal) | 128 KB | Fastest |
| SRAM | 512 KB | Fast |
| DTCMRAM | 128 KB | Fast |
| QSPIFLASH | ~8 MB | Slower |
| SDRAM | 64 MB | Slowest |

### Installing Bootloader

```bash
# Enter DFU mode (hold BOOT, press RESET)
make program-boot
```

### Flashing via Bootloader

| Method | Command |
|--------|---------|
| DFU (USB) | `make program-dfu` |
| SD Card | Copy `.bin` to SD, power on |
| USB Drive | Copy `.bin` to USB, power on |

### Generating Bootloader-Compatible Programs

```bash
# For QSPI flash:
make clean
APP_TYPE=BOOT_QSPI make
```

---

## 8. Serial Printing

> **Reference**: [Serial Printing Tutorial](https://daisy.audio/tutorials/_a2_Getting-Started-Serial-Printing/)

### Quick Reference

```cpp
hw.StartLog();        // Non-blocking start
hw.StartLog(true);    // Wait for USB connection

hw.PrintLine("Hello");
hw.PrintLine("Value: %d", my_int);
hw.PrintLine("Float: " FLT_FMT3, FLT_VAR3(my_float));
```

**Baud Rate**: 115200 (default)

---

## Quick Reference Summary

### Critical Rules

| Rule | Reason |
|------|--------|
| No dynamic allocation in callback | Non-deterministic time |
| Initialize DSP with sample rate | Prevents undefined behavior |
| Use `DSY_SDRAM_BSS` for large buffers | Internal RAM is limited |
| Pull resistors for buttons | Prevents floating inputs |
| Lowest GPIO speed | Reduces EMI noise |
| Blocking calls in main loop only | Prevents audio underruns |

### Common Patterns

```cpp
// Button with pull-up (common wiring):
gpio.Init(pin, Mode::INPUT, Pull::PULLUP);
bool pressed = !gpio.Read();  // Active low

// Knob reading:
float value = hw.adc.GetFloat(channel);

// Large buffer:
float DSY_SDRAM_BSS buffer[48000 * 10];  // 10 sec @ 48kHz

// CPU monitoring:
CpuLoadMeter meter;
```

---

## Quality Assurance Ecosystem

This document is part of an interconnected quality assurance system for Daisy development:

```
┌─────────────────────────────────────────────────────────────────┐
│                    DAISY QA DOCUMENT SYSTEM                     │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────────────┐     ┌──────────────────────────────┐ │
│  │ TUTORIALS_KNOWLEDGE  │────▶│ DEVELOPMENT_STANDARDS        │ │
│  │ (Foundation)         │     │ (Workflow & Code Patterns)   │ │
│  └──────────────────────┘     └──────────────────────────────┘ │
│           │                              │                      │
│           ▼                              ▼                      │
│  ┌──────────────────────┐     ┌──────────────────────────────┐ │
│  │ DEBUG_STRATEGY       │◀───▶│ TECHNICAL_REPORT             │ │
│  │ (Troubleshooting)    │     │ (Comprehensive Reference)    │ │
│  └──────────────────────┘     └──────────────────────────────┘ │
│           │                              │                      │
│           └──────────────┬───────────────┘                      │
│                          ▼                                      │
│                 ┌────────────────┐                              │
│                 │   DAISY_BUGS   │                              │
│                 │ (Issue Tracker)│                              │
│                 └────────────────┘                              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Related Documents

| Document | Purpose | When to Use |
|----------|---------|-------------|
| [DAISY_DEVELOPMENT_STANDARDS.md](DAISY_DEVELOPMENT_STANDARDS.md) | Workflow, code templates, anti-patterns | Starting a new project |
| [DAISY_DEBUG_STRATEGY.md](DAISY_DEBUG_STRATEGY.md) | Serial/hardware debugging techniques | When something isn't working |
| [DAISY_TECHNICAL_REPORT.md](DAISY_TECHNICAL_REPORT.md) | Complete process documentation | Deep reference, onboarding |
| [DAISY_BUGS.md](DAISY_BUGS.md) | Bug tracking, investigation methodology | Documenting issues, searching past fixes |
| [field_defaults.h](foundation_examples/field_defaults.h) | Hardware helper library | Using Daisy Field LEDs/display |

### How This Document Fits

**DAISY_TUTORIALS_KNOWLEDGE.md** provides the **foundational "what"**:
- What GPIO modes exist and when to use them
- What constraints apply to the audio callback
- What memory regions are available

Other documents build on this foundation:
- **DEVELOPMENT_STANDARDS**: "How" to apply this knowledge in workflow
- **DEBUG_STRATEGY**: "How" to troubleshoot when things fail
- **TECHNICAL_REPORT**: "Why" decisions were made
- **BUGS.md**: "What" went wrong and how it was fixed

---

**Document Version**: 1.1
**Last Updated**: 2026-02-08

## Changelog

| Version | Date | Changes |
|---------|------|---------|
| 1.1 | 2026-02-08 | Added QA Ecosystem cross-reference section, common patterns summary |
| 1.0 | 2026-02-08 | Initial version: GPIO, Audio, ADC, SDRAM, SPI, I2C, Bootloader, Serial |

