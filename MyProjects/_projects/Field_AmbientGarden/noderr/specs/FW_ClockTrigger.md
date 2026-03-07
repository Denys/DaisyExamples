# Node Specification: FW_ClockTrigger - Voice Distribution on Clock Tick

**Version:** 1.0
**Date:** 2026-03-06
**Author:** AI-Agent (Draft — retrofit)
**Classification:** Critical

## 1. Purpose
* **Goal:** On each clock tick from GEN_RandomClock, advance the Turing Machine and ScaleQuantizer to get a base frequency, then probabilistically trigger 1–4 ModalVoices with harmonic spread applied. Called from within AudioCallback per-sample.

## 2. Dependencies & Triggers
* **Prerequisite NodeIDs:** `GEN_TuringMachine`, `GEN_ScaleQuantizer`, `DSP_ModalVoices`, `FW_ParamBridge` (reads `params.spread`), `CFG_VoicePresets` (reads `kPresets[current_preset]`)
* **Input Data/State:** `params.spread` (0–1), `current_preset` (0–7), `kPresets[]` array
* **Trigger:** Called from `FW_AudioCallback` when `GEN_RandomClock.Process()` returns true

## 3. Interfaces
* **Function:** `void OnClockTrigger()`
* **Outputs (side effects):**
  - `modal[0-3].SetFreq/SetAccent/SetBrightness/Trig()` — triggers voices
  - `last_voice_freq[0-3]` — updated for OLED display
  - `voice_active[0-3]` — set to true on trigger
  - `voice_pulse[0-3]` — set to 6 (~100ms pulse at 60Hz) for LED animation

## 4. Core Logic & Processing Steps

1. Call `tm.Process()` → get 8-bit shift register value
2. Call `quantizer.Process(tm_output)` → get quantized frequency in Hz
3. Voice 0 (melody): always triggered at base frequency, preset accent
4. Voice 1 (harmony low): triggered at 60% probability, freq = base × 2^(-spread×12/12)
5. Voice 2 (harmony high): triggered at 40% probability, freq = base × 2^(spread×7/12)
6. Voice 3 (accent chime): triggered at 20% probability, freq = base × 2.0 (octave up)
7. Update `last_voice_freq[]`, `voice_active[]`, `voice_pulse[]` for each triggered voice

## 5. Data Structures

```cpp
// Voice probability thresholds (constants in implementation)
constexpr float kVoice1Prob = 0.60f; // harmony low
constexpr float kVoice2Prob = 0.40f; // harmony high
constexpr float kVoice3Prob = 0.20f; // accent chime

// Shared state (written here, read in main loop for display/LEDs)
float last_voice_freq[4]; // Last triggered frequency per voice
bool  voice_active[4];    // Whether voice was last triggered
int   voice_pulse[4];     // LED pulse counter (decremented in main loop)
```

## 6. Error Handling & Edge Cases
* Voice 1 frequency clipped to minimum 30.0 Hz (below Daisy Field speaker range)
* Voice 2/3 frequency clipped to maximum 8000.0 Hz (prevent aliasing in ModalVoice)
* If spread = 0: all voices at same frequency (unison cluster)
* If spread = 1: voices span up to 3 octaves

## 7. ARC Verification Criteria

* **Functional:**
  * ARC_FUNC_01: At K1=max, clock fires frequently; all 4 voices trigger within ~2 seconds of observation
  * ARC_FUNC_02: Voice 0 fires on every clock tick (no missed melody notes)
  * ARC_FUNC_03: K4=0 → all voices near same pitch; K4=1 → voices spread across 2-3 octaves
  * ARC_FUNC_04: voice_pulse[] increments trigger matching LED flash on B-row
* **Real-time Safety:**
  * ARC_RT_01: No `rand()` call — uses xorshift32 PRNG only (lock-free, callback-safe)
  * ARC_RT_02: Function completes in < 5μs on Cortex-M7 (no blocking, no heap)
* **Error Handling:**
  * ARC_ERR_01: freq2 < 30.0 Hz → clamped, no crash
  * ARC_ERR_02: freq3/4 > 8000 Hz → clamped, no crash

## 8. Notes & Considerations

* **BUG-1 (RESOLVED in fix-20260306-BUG1):** Original implementation used `rand()` via `RandomFloat()`. `rand()` is NOT real-time safe — it acquires an internal mutex on most C runtimes, risking audio dropouts. Fixed by replacing with xorshift32 PRNG (static uint32_t, 3 XOR-shift operations, returns float 0–1).

* `voice_pulse[]` and `last_voice_freq[]` are written here (AudioCallback context) and read in main loop (LED/OLED). On Cortex-M7, 32-bit writes to aligned addresses are atomic, so no mutex needed in practice. Technically undefined behavior per C++ standard.

* `voice_active[]` is currently set to true but never set to false — mild logic gap, but harmless since only used for OLED display direction.

* **xorshift32 replacement pattern:**
  ```cpp
  static uint32_t xr_state = 12345;
  inline float RandomFloat() {
      xr_state ^= xr_state << 13;
      xr_state ^= xr_state >> 17;
      xr_state ^= xr_state << 5;
      return static_cast<float>(xr_state) * 2.3283064e-10f;
  }
  ```
  Seeded in `main()` using `xr_state = System::GetNow() | 1u;` (never 0).
