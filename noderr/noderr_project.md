> **[INSTANCE: DAISY-FIRMWARE]** C/C++ ARM / libDaisy / DaisySP | Noderr: `DaisyExamples/noderr/` | Mode: Firmware
> NodeIDs: `DSP_`, `FX_`, `LIBDASY_`, `DVPE_` — Wrong instance? Navigate to `noderr/noderr/`

# Project Overview: DaisyExamples - Visual Programming Environment

---

**Purpose of this Document:** This `noderr_project.md` is a core artifact of the Noderr v1.9 system. It provides a comprehensive high-level summary of the project, including its goals, scope, technology stack, architecture, coding standards, and quality priorities. The AI Agent will reference this document extensively for context and guidance throughout the development lifecycle, as detailed in `noderr_loop.md`.

---

### 1. Project Goal & Core Problem

*   **Goal:** Create a comprehensive collection of embedded audio processing examples and a visual programming environment for the Daisy audio platform, enabling musicians and developers to create audio effects and instruments.
*   **Core Problem Solved:** Provides working code examples, documentation, and tools for developing audio applications on embedded hardware (Daisy platform), reducing the learning curve for embedded audio development.

---

### 2. Scope & Key Features (MVP Focus)

*   **Minimum Viable Product (MVP) Description:** A collection of functional audio effect examples (oscillators, filters, envelope generators, reverb, etc.) that can be built and flashed to Daisy hardware, along with a visual programming environment framework.
*   **Key Features (In Scope for MVP):**
    *   `Audio Effect Examples`: Working implementations of chorus, flanger, phaser, sampler, modal voice, string voice
    *   `Granular Processor (Nimbus)`: Advanced granular synthesis engine with sample playback
    *   `Hardware Examples`: Platform-specific examples for seed, patch, pod, field, petal, versio, cube
    *   `Build System`: Makefile-based build system for all examples
    *   `Documentation`: README files and inline code documentation
*   **Key Features (Explicitly OUT of Scope for MVP):**
    *   `Network API for Remote Control`: Not required for local embedded MVP
    *   `Cloud Sync for Presets`: Not required for embedded MVP
    *   `Automated CI Hardware Farm`: Deferred to post MVP

### 2.1 MVP Implementation Status

*   **MVP Completion:** 17% (4 of 23 components verified)
*   **Existing Components Identified:** 19
*   **Missing Components Required for MVP:** 4
*   **Missing MVP Components:** `DVPE_UI`, `DVPE_Compiler`, `DVPE_ParamControl`, `DVPE_PresetManager`
*   **Current Priority:** Complete DVPE components while preserving verified embedded audio foundation

---

### 3. Target Audience

*   **Primary User Group(s):** 
    *   Musicians and audio engineers wanting to create custom audio effects
    *   Embedded systems developers learning audio programming
    *   Electronic music enthusiasts building custom instruments
*   **Key User Needs Addressed:**
    *   Working code examples that can be studied and modified
    *   Easy-to-use build system for compiling examples
    *   Documentation for understanding Daisy platform capabilities

---

### 4. Technology Stack (Specific Versions Critical for Agent)

| Category             | Technology                                                                                                                                                        | Specific Version (or latest stable)      | Notes for AI Agent / Rationale                                         |
|:---------------------|:------------------------------------------------------------------------------------------------------------------------------------------------------------------|:-----------------------------------------|:----------------------------------------------------------------------------|
| Language(s)          | C++ (embedded)                                                                                                                                                   | C++14 (gnu++14)                         | Required for STM32H7 microcontroller                       |
| Backend Framework    | libDaisy (Hardware Abstraction Layer)                                                                                                                           | Latest from submodule                   | Hardware abstraction for Daisy devices                |
| Audio Library        | DaisySP (Audio Signal Processing)                                                                                                                                | Latest from submodule                   | DSP algorithms and audio processing                  |
| Build System         | GNU Make                                                                                                                                                         | Make 3.81+                              | Required for building embedded projects             |
| Toolchain            | ARM GCC Embedded Toolchain                                                                                                                                       | arm-none-eabi-gcc 10.x                  | Cross-compiler for STM32 ARM Cortex-M7              |
| Hardware Platform    | STM32H750IB                                                                                                                                                     | Rev 4                                    | Daisy Seed microcontroller                         |
| Documentation        | Doxygen                                                                                                                                                          |                                          | API documentation generation                        |
| Version Control      | Git                                                                                                                                                               |                                          | Repository hosted on GitHub                         |
| Deployment Target    | Daisy Hardware (Seed, Patch, Pod, Field, Petal, Versio, Cube)                                                                                                  |                                          | Various Daisy board form factors                   |
| Debugger             | OpenOCD / ST-Link                                                                                                                                                |                                          | For debugging with GDB                            |
| Programmer           | USB DFU (Device Firmware Update)                                                                                                                                |                                          | Primary flashing method                            |

* **Tech Stack Rationale:** C++ with libDaisy provides low-level hardware access while DaisySP offers pre-built DSP algorithms. The ARM GCC toolchain is the standard for STM32 embedded development. Using Makefile-based builds keeps the system simple and portable.

---

### 5. High-Level Architecture

* **Architectural Style:** Embedded Audio Processing Application with Hardware Abstraction
* **Key Components & Interactions (Brief Textual Description):** The project consists of multiple independent example applications. Each example uses libDaisy for hardware initialization (GPIO, ADC, DAC, I2C, SPI) and DaisySP for audio signal processing. The applications run on STM32H750 microcontroller and interface with audio codec (AK4552) for analog/digital audio conversion.
* **Diagram (Mermaid - Agent to Generate):**
    ```mermaid
    graph TD
        A[User] -->|Flashes Binary| B[Daisy Hardware];
        B -->|Audio In| C[AK4552 Codec];
        C -->|I2S| D[STM32H750 Processor];
        D -->|Processing| E[DaisySP Audio Algorithms];
        D -->|Control| F[Hardware (Pots, Buttons, LEDs)];
        E -->|Processed Audio| C;
        C -->|Audio Out| G[Output];
        
        subgraph "libDaisy"
            D --> H[GPIO Manager];
            D --> I[Audio Driver];
            D --> J[Hardware Init];
        end
        
        subgraph "DaisySP"
            E --> K[Oscillators];
            E --> L[Filters];
            E --> M[Effects];
            E --> N[Envelopes];
        end
    ```

---

### 6. Core Components/Modules (Logical Breakdown)

* `libDaisy Core`: Hardware abstraction layer providing initialization, GPIO, I2C, SPI, UART, USB, and audio driver interfaces
* `DaisySP Audio Library`: Collection of DSP algorithms including oscillators, filters, effects, envelope generators, and utilities
* `Field Platform Examples`: Audio effects (chorus, flanger, phaser, sampler) designed for the Field hardware
* `Nimbus Granular Processor`: Advanced granular synthesis engine with multiple playback modes
* `Patch/Seed/Pod Examples`: Platform-specific example applications demonstrating various features

---

### 7. Key UI/UX Considerations

* **Overall Feel:** N/A - This is embedded code with no GUI. User interface is through physical controls (potentiometers, buttons, LEDs).
* **Key Principles:**
    * `Clarity`: Well-commented code with clear variable names
    * `Efficiency`: Real-time audio processing requires efficient code
    * `Reliability`: Deterministic timing for audio callbacks

---

### 8. Coding Standards & Conventions

* **Primary Style Guide:** Custom coding style enforced by clang-format-10
* **Formatter:** clang-format-10 (as specified in README.md)
* **Linter:** clang-format for style checking
* **File Naming Conventions:** 
    * Source files: `lowercase.cpp`
    * Header files: `lowercase.h`
    * Makefiles: `Makefile` (no extension)
* **Commit Message Convention:** Conventional Commits (e.g., `feat: add chorus effect`, `fix: correct filter cutoff`)
* **Code Commenting Style:** Doxygen-style comments for public APIs, inline comments for complex logic
* **Other Key Standards:**
    * Use `float` for audio processing (32-bit floating point)
    * Maintain real-time constraints in audio callback
    * Avoid dynamic memory allocation in audio path

### 8.1 Component Classification Criteria

* **Standard**: Simple audio effects with minimal parameters
* **Complex**: Multi-voice processors, granular engines with multiple modes
* **Critical**: Hardware initialization, audio callback timing

---

### 9. Key Quality Criteria Focus (Priorities from `noderr/noderr_log.md`)
* This project will prioritize the following **Top 3-5 quality criteria** from the "Reference Quality Criteria" section of `noderr/noderr_log.md`. Agent, you should pay special attention to these during ARC-Based Verification.
    1.  **Performance (Real-time)**: Audio processing must complete within sample time (1/48000s)
    2.  **Reliability**: Deterministic behavior, no audio glitches
    3.  **Code Quality**: Clear, maintainable C++ code
    4.  **Documentation**: README files and inline comments
    5.  **Portability**: Code should work across different Daisy hardware variants

---

### 10. Testing Strategy

* **Required Test Types for MVP:**
    * `Build Testing`: All examples must compile without errors or warnings
    * `Static Analysis`: Code style verification with clang-format
    * `Hardware Testing`: Flash to device and verify audio output (manual)
* **Testing Framework(s) & Version(s):** 
    * clang-format-10 for code style
    * arm-none-eabi-size for memory usage analysis
* **Test File Location & Naming:** N/A - No automated test framework in examples
* **Minimum Code Coverage Target (Conceptual Goal):** N/A for embedded examples

---

### 11. Initial Setup Steps (Conceptual for a new developer/environment)

1.  **Clone Repository:** `git clone --recursive https://github.com/electro-smith/DaisyExamples`
2.  **Install Toolchain:** Download ARM GCC Embedded Toolchain from ARM's website
3.  **Install Build Tools:** Ensure make is available (MINGW64 on Windows)
4.  **Build Libraries:** `./ci/build_libs.sh`
5.  **Build Example:** `cd field/chorus && make`
6.  **Flash to Hardware:** Use Daisy Web Programmer or `make program-dfu`

---

### 12. Key Architectural Decisions & Rationale

* **Decision 1: C++ with libDaisy for Hardware Access**
    * **Rationale:** libDaisy provides clean C++ abstraction over STM32 HAL while maintaining performance
* **Decision 2: DaisySP for Audio Processing**
    * **Rationale:** Pre-built, optimized DSP algorithms that are well-tested
* **Decision 3: Makefile-based Build System**
    * **Rationale:** Simple, portable, no complex build system needed for embedded projects

---

### 13. Repository Link

* `https://github.com/electro-smith/DaisyExamples` (Main repository)
* Local development: `c:\Users\denko\Gemini\Antigravity\DVPE_Daisy-Visual-Programming-Environment\DaisyExamples`

---

### 14. Dependencies & Third-Party Services (Key Ones for MVP)

* **libDaisy (Hardware Abstraction):**
    * Purpose: Low-level hardware access for Daisy devices
    * Integration: Git submodule
* **DaisySP (Audio Library):**
    * Purpose: Audio signal processing algorithms
    * Integration: Git submodule
* *(No external services required)*

---

### 15. Security Considerations (Initial High-Level)

* **Authentication:** N/A - No network connectivity
* **Authorization:** N/A - Single-user embedded device
* **Input Validation:** Validate all ADC inputs, constrain parameter ranges
* **Data Protection:** No sensitive data stored
* **Dependency Management:** Keep submodules updated periodically

---

### 16. Performance Requirements (Initial Qualitative Goals)

* **Response Time:** Audio callback must complete in < 20.83Î¼s (1/48000s)
* **Load Capacity (Conceptual for MVP):** Single audio application running
* **Scalability Approach (Future Consideration):** Add more complex effects, multi-voice polyphony

---

### 17. Monitoring & Observability (Basic for MVP)

* **Logging Strategy (Application-level):**
    * Serial UART debug output at 115200 baud
    * LED status indicators for runtime feedback
* **Monitoring Tools:** Serial terminal (PuTTY, TeraTerm)
* **Key Metrics to Observe (Qualitative):** Audio quality, response to control changes
* **Alerting Criteria (Manual for MVP):** Visual/audio feedback from hardware

---

### 18. Links to Other Noderr v1.9 Artifacts
* **Agent Main Loop & Protocol:** `noderr/noderr_loop.md`
* **Operational Record & Quality Criteria:** `noderr/noderr_log.md`
* **Architectural Flowchart (This Project):** `noderr/noderr_architecture.md`
* **Status Map (This Project):** `noderr/noderr_tracker.md`
* **Component Specifications Directory:** `noderr/specs/`
* **Environment Protocol:** `noderr/environment_context.md` (completed)

---

*(This document describes the DaisyExamples project - a collection of embedded audio examples for the Daisy platform. The project provides working code that can be studied, modified, and flashed to hardware for audio applications.)*

