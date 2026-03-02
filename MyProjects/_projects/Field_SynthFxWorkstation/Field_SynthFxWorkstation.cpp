/**
 * Field_SynthFxWorkstation
 *
 * Daisy Field adaptation of the Pod synth+fx workstation.
 * Signal path:
 * MIDI/Keys -> Grainlet+Particle+Dust voice -> FX bus -> Distortion -> Stereo out
 *
 * Interface:
 * - External MIDI in (DIN/UART via hw.midi)
 * - OLED status
 * - USB serial monitor logs
 * - Patch system (A1/A2/A3): Bass / Kick Synth / Snare Noise
 */

#include "daisy_field.h"
#include "daisysp.h"
#include <cmath>
#include <cstdio>

using namespace daisy;
using namespace daisysp;

DaisyField hw;

// Voice core
GrainletOscillator grainlet;
Particle           particle;
Dust               dust;
Adsr               amp_env;
AdEnv              pitch_env;
Svf                voice_filter;

// FX
Chorus chorus;

DelayLine<float, 32768> delay_l, delay_r;
DelayLine<float, 8192>  reverb_l, reverb_r;

Overdrive dist_l, dist_r;

enum PatchId
{
    PATCH_BASS  = 0,
    PATCH_KICK  = 1,
    PATCH_SNARE = 2,
    PATCH_COUNT
};

struct PatchSpec
{
    const char* name;
    int         default_note;
    float       blend;
    float       timbre;
    float       cutoff_norm;
    float       decay_s;
    float       release_s;
    float       fx_amount;
    float       delay_ms;
    float       drive;
    float       reverb_amount;
    float       pitch_env_amt;
    float       pitch_env_decay_s;
    float       particle_amount;
    float       dust_amount;
    float       sustain_level;
    float       output_level;
};

const PatchSpec kPatchSpecs[PATCH_COUNT] = {
    {"Bass", 36, 0.84f, 0.32f, 0.28f, 0.26f, 0.22f, 0.32f, 170.0f, 0.42f, 0.20f, 0.24f, 0.07f, 0.34f, 0.18f, 0.62f, 0.72f},
    {"Kick", 36, 0.94f, 0.42f, 0.22f, 0.09f, 0.05f, 0.15f, 85.0f, 0.74f, 0.08f, 1.45f, 0.05f, 0.16f, 0.10f, 0.00f, 0.84f},
    {"Snare", 38, 0.40f, 0.78f, 0.66f, 0.11f, 0.09f, 0.28f, 120.0f, 0.58f, 0.18f, 0.58f, 0.04f, 0.72f, 0.86f, 0.00f, 0.72f},
};

// Runtime voice state
volatile PatchId active_patch = PATCH_BASS;
volatile bool    midi_gate    = false;
volatile bool    drone_mode   = false;
volatile int     last_note    = -1;
volatile float   note_freq    = 55.0f;
volatile float   note_vel     = 0.0f;

// Sound parameters
volatile float voice_blend      = 0.8f;
volatile float timbre           = 0.3f;
volatile float cutoff_norm      = 0.3f;
volatile float env_decay_s      = 0.25f;
volatile float env_release_s    = 0.2f;
volatile float fx_amount        = 0.3f;
volatile float delay_time_ms    = 180.0f;
volatile float dist_drive       = 0.4f;
volatile float reverb_amount    = 0.2f;
volatile float pitch_env_amt    = 0.25f;
volatile float pitch_env_decay  = 0.06f;
volatile float particle_amount  = 0.3f;
volatile float dust_amount      = 0.2f;
volatile float sustain_level    = 0.6f;
volatile float output_level     = 0.72f;
volatile float delay_feedback   = 0.45f;
volatile float reverb_feedback  = 0.52f;

// Manual key triggers (short gates)
uint32_t manual_gate_off_ms = 0;

// Reverb and dust state
float rev_state_l = 0.0f;
float rev_state_r = 0.0f;
float dust_burst  = 0.0f;

// UI/telemetry
uint32_t last_status_log_ms = 0;
uint32_t last_oled_ms       = 0;
bool     serial_help_done   = false;

volatile float gate_meter  = 0.0f;
volatile float dust_meter  = 0.0f;
volatile bool  ui_needs_refresh = true;

// Knob takeover so patch defaults remain active until knob catches up.
float knob_target[8]    = {0};
float knob_effective[8] = {0};
bool  knob_picked[8]    = {false};

static inline float Clamp01(float v)
{
    return fclamp(v, 0.0f, 1.0f);
}

template <size_t N>
static inline size_t MsToSamplesClamped(float ms, float sr)
{
    const float s = (ms * 0.001f) * sr;
    return static_cast<size_t>(fclamp(s, 1.0f, static_cast<float>(N - 1)));
}

const char* PatchName(PatchId id)
{
    if(id < PATCH_COUNT)
        return kPatchSpecs[id].name;
    return "Unknown";
}

void PrintSerialHelp()
{
    if(serial_help_done)
        return;
    serial_help_done = true;

    hw.seed.PrintLine("=== Field_SynthFxWorkstation ===");
    hw.seed.PrintLine("External MIDI: NoteOn/NoteOff drive synth.");
    hw.seed.PrintLine("Keys: A1=Bass A2=Kick A3=Snare A4=Drone");
    hw.seed.PrintLine("Keys: B1/B2/B3 trigger test notes 36/38/43");
    hw.seed.PrintLine("Switch1: Drone toggle, Switch2: Panic (gate off)");
    hw.seed.PrintLine("Knobs: K1 Blend K2 Timbre K3 Cutoff K4 Decay");
    hw.seed.PrintLine("       K5 FX K6 Delay K7 Drive K8 Reverb");
}

void SetKnobTargetsFromCurrentParams()
{
    knob_target[0] = Clamp01(voice_blend);
    knob_target[1] = Clamp01(timbre);
    knob_target[2] = Clamp01(cutoff_norm);
    knob_target[3] = Clamp01((env_decay_s - 0.02f) / 1.20f);
    knob_target[4] = Clamp01(fx_amount);
    knob_target[5] = Clamp01((delay_time_ms - 25.0f) / 575.0f);
    knob_target[6] = Clamp01((dist_drive - 0.05f) / 0.95f);
    knob_target[7] = Clamp01(reverb_amount);

    for(int i = 0; i < 8; i++)
    {
        knob_effective[i] = knob_target[i];
        knob_picked[i]    = false;
    }
}

void ApplyPatch(PatchId id)
{
    const PatchSpec& p = kPatchSpecs[id];

    active_patch      = id;
    note_freq         = mtof(static_cast<float>(p.default_note));
    note_vel          = 0.85f;
    voice_blend       = p.blend;
    timbre            = p.timbre;
    cutoff_norm       = p.cutoff_norm;
    env_decay_s       = p.decay_s;
    env_release_s     = p.release_s;
    fx_amount         = p.fx_amount;
    delay_time_ms     = p.delay_ms;
    dist_drive        = p.drive;
    reverb_amount     = p.reverb_amount;
    pitch_env_amt     = p.pitch_env_amt;
    pitch_env_decay   = p.pitch_env_decay_s;
    particle_amount   = p.particle_amount;
    dust_amount       = p.dust_amount;
    sustain_level     = p.sustain_level;
    output_level      = p.output_level;
    delay_feedback    = 0.25f + 0.60f * fx_amount;
    reverb_feedback   = 0.30f + 0.62f * reverb_amount;

    amp_env.SetAttackTime(0.002f);
    amp_env.SetDecayTime(env_decay_s);
    amp_env.SetSustainLevel(sustain_level);
    amp_env.SetReleaseTime(env_release_s);

    pitch_env.SetTime(ADENV_SEG_ATTACK, 0.001f);
    pitch_env.SetTime(ADENV_SEG_DECAY, pitch_env_decay);

    SetKnobTargetsFromCurrentParams();

    ui_needs_refresh = true;
    hw.seed.PrintLine("[PATCH] %s note=%d", p.name, p.default_note);
}

void TriggerManualNote(int midi_note, int gate_ms)
{
    note_freq           = mtof(static_cast<float>(midi_note));
    note_vel            = 1.0f;
    midi_gate           = true;
    last_note           = midi_note;
    manual_gate_off_ms  = System::GetNow() + static_cast<uint32_t>(gate_ms);
    pitch_env.Trigger();
    dust_burst = 0.8f;
    ui_needs_refresh = true;
}

void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:
        {
            NoteOnEvent n = m.AsNoteOn();
            if(n.velocity > 0)
            {
                note_freq = mtof(static_cast<float>(n.note));
                note_vel  = n.velocity / 127.0f;
                last_note = n.note;
                midi_gate = true;
                pitch_env.Trigger();
                dust_burst = fmaxf(dust_burst, 0.25f + 0.55f * note_vel);
                ui_needs_refresh = true;

                hw.seed.PrintLine("[MIDI] NoteOn note=%d vel=%d freq=%.1f",
                                  n.note,
                                  n.velocity,
                                  note_freq);
            }
            else if(n.note == last_note)
            {
                midi_gate = false;
                ui_needs_refresh = true;
                hw.seed.PrintLine("[MIDI] NoteOff(v0) note=%d", n.note);
            }
        }
        break;

        case NoteOff:
        {
            NoteOffEvent n = m.AsNoteOff();
            if(n.note == last_note)
            {
                midi_gate = false;
                ui_needs_refresh = true;
                hw.seed.PrintLine("[MIDI] NoteOff note=%d", n.note);
            }
        }
        break;

        default: break;
    }
}

void UpdateLeds()
{
    // Patch select LEDs (A1..A3)
    for(int i = 0; i < 8; i++)
    {
        float v = 0.04f;
        if(i < 3 && i == static_cast<int>(active_patch))
            v = 1.0f;
        hw.led_driver.SetLed(DaisyField::LED_KEY_A1 + i, v);
    }

    // B row: gate meter pulse on B1, activity on B2/B3.
    hw.led_driver.SetLed(DaisyField::LED_KEY_B1, Clamp01(0.10f + 0.90f * gate_meter));
    hw.led_driver.SetLed(DaisyField::LED_KEY_B2, Clamp01(0.08f + 0.85f * dust_meter));
    hw.led_driver.SetLed(DaisyField::LED_KEY_B3, midi_gate ? 0.9f : 0.08f);

    // Switch LEDs
    hw.led_driver.SetLed(DaisyField::LED_SW_1, drone_mode ? 1.0f : 0.08f);
    hw.led_driver.SetLed(DaisyField::LED_SW_2, midi_gate ? 0.6f : 0.08f);

    // Knob ring LEDs follow effective control values.
    for(int i = 0; i < 8; i++)
    {
        hw.led_driver.SetLed(DaisyField::LED_KNOB_1 + i, Clamp01(knob_effective[i]));
    }

    hw.led_driver.SwapBuffersAndTransmit();
}

void LogStatus(bool force)
{
    const uint32_t now = System::GetNow();
    if(!force && (now - last_status_log_ms) < 1000)
        return;
    last_status_log_ms = now;

    hw.seed.PrintLine("[STATUS] patch=%s gate=%d drone=%d note=%d f=%.1f "
                      "K1=%.2f K2=%.2f K3=%.2f K4=%.2f K5=%.2f K6=%.2f K7=%.2f K8=%.2f",
                      PatchName(active_patch),
                      midi_gate ? 1 : 0,
                      drone_mode ? 1 : 0,
                      last_note,
                      note_freq,
                      knob_effective[0],
                      knob_effective[1],
                      knob_effective[2],
                      knob_effective[3],
                      knob_effective[4],
                      knob_effective[5],
                      knob_effective[6],
                      knob_effective[7]);
}

void UpdateOled()
{
    const uint32_t now = System::GetNow();
    if(!ui_needs_refresh && (now - last_oled_ms) < 100)
        return;
    last_oled_ms = now;
    ui_needs_refresh = false;

    char line[48];
    hw.display.Fill(false);

    hw.display.SetCursor(0, 0);
    hw.display.WriteString("Field Synth+FX", Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Patch:%s G:%d D:%d",
             PatchName(active_patch),
             midi_gate ? 1 : 0,
             drone_mode ? 1 : 0);
    hw.display.SetCursor(0, 10);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line, sizeof(line), "N:%d F:%3.0fHz", last_note, note_freq);
    hw.display.SetCursor(0, 20);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "B:%2d T:%2d C:%2d D:%2d",
             static_cast<int>(knob_effective[0] * 99.0f),
             static_cast<int>(knob_effective[1] * 99.0f),
             static_cast<int>(knob_effective[2] * 99.0f),
             static_cast<int>(knob_effective[3] * 99.0f));
    hw.display.SetCursor(0, 30);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Fx:%2d Dly:%3d Drv:%2d Rv:%2d",
             static_cast<int>(knob_effective[4] * 99.0f),
             static_cast<int>(25.0f + 575.0f * knob_effective[5]),
             static_cast<int>(knob_effective[6] * 99.0f),
             static_cast<int>(knob_effective[7] * 99.0f));
    hw.display.SetCursor(0, 40);
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.SetCursor(0, 54);
    hw.display.WriteString("A1 Bass A2 Kick A3 Snr", Font_6x8, true);

    hw.display.Update();
}

void UpdateControls()
{
    hw.ProcessAllControls();

    if(hw.sw[0].RisingEdge())
    {
        drone_mode = !drone_mode;
        ui_needs_refresh = true;
        hw.seed.PrintLine("[SW1] drone=%d", drone_mode ? 1 : 0);
    }

    if(hw.sw[1].RisingEdge())
    {
        midi_gate = false;
        manual_gate_off_ms = 0;
        ui_needs_refresh = true;
        hw.seed.PrintLine("[SW2] panic: gate off");
    }

    // Patch selection keys
    if(hw.KeyboardRisingEdge(0))
        ApplyPatch(PATCH_BASS);
    if(hw.KeyboardRisingEdge(1))
        ApplyPatch(PATCH_KICK);
    if(hw.KeyboardRisingEdge(2))
        ApplyPatch(PATCH_SNARE);

    // Convenience key: toggle drone
    if(hw.KeyboardRisingEdge(3))
    {
        drone_mode = !drone_mode;
        ui_needs_refresh = true;
        hw.seed.PrintLine("[KEY A4] drone=%d", drone_mode ? 1 : 0);
    }

    // Test triggers for quick audition without external MIDI.
    if(hw.KeyboardRisingEdge(8))
        TriggerManualNote(36, 140);
    if(hw.KeyboardRisingEdge(9))
        TriggerManualNote(38, 110);
    if(hw.KeyboardRisingEdge(10))
        TriggerManualNote(43, 180);

    // Manual gate timeout
    if(manual_gate_off_ms != 0 && System::GetNow() >= manual_gate_off_ms && !drone_mode)
    {
        midi_gate = false;
        manual_gate_off_ms = 0;
        ui_needs_refresh = true;
    }

    // Knob takeover + mapping
    const float raw_knobs[8] = {
        hw.knob[0].Process(),
        hw.knob[1].Process(),
        hw.knob[2].Process(),
        hw.knob[3].Process(),
        hw.knob[4].Process(),
        hw.knob[5].Process(),
        hw.knob[6].Process(),
        hw.knob[7].Process(),
    };

    bool changed = false;
    for(int i = 0; i < 8; i++)
    {
        if(!knob_picked[i])
        {
            if(fabsf(raw_knobs[i] - knob_target[i]) < 0.04f)
            {
                knob_picked[i] = true;
                hw.seed.PrintLine("[KNOB] K%d picked up", i + 1);
            }
            else
            {
                knob_effective[i] = knob_target[i];
                continue;
            }
        }

        if(fabsf(knob_effective[i] - raw_knobs[i]) > 0.002f)
        {
            knob_effective[i] = raw_knobs[i];
            knob_target[i]    = raw_knobs[i];
            changed           = true;
        }
    }

    if(changed)
    {
        voice_blend   = knob_effective[0];
        timbre        = knob_effective[1];
        cutoff_norm   = knob_effective[2];
        env_decay_s   = 0.02f + 1.20f * knob_effective[3];
        fx_amount     = knob_effective[4];
        delay_time_ms = 25.0f + 575.0f * knob_effective[5];
        dist_drive    = 0.05f + 0.95f * knob_effective[6];
        reverb_amount = knob_effective[7];

        delay_feedback  = 0.25f + 0.60f * fx_amount;
        reverb_feedback = 0.30f + 0.62f * reverb_amount;

        // Keep patch flavor while still reacting to controls.
        switch(active_patch)
        {
            case PATCH_BASS:
                env_release_s   = 0.08f + 0.45f * (1.0f - cutoff_norm);
                sustain_level   = 0.45f + 0.45f * (1.0f - env_decay_s * 0.7f);
                pitch_env_amt   = 0.18f + 0.25f * (1.0f - voice_blend);
                particle_amount = 0.20f + 0.45f * (1.0f - voice_blend);
                dust_amount     = 0.10f + 0.35f * timbre;
                output_level    = 0.68f;
                break;

            case PATCH_KICK:
                env_release_s   = 0.025f + 0.16f * (1.0f - env_decay_s);
                sustain_level   = 0.0f;
                pitch_env_amt   = 0.85f + 1.25f * (1.0f - cutoff_norm);
                particle_amount = 0.07f + 0.18f * timbre;
                dust_amount     = 0.05f + 0.20f * timbre;
                output_level    = 0.84f;
                break;

            case PATCH_SNARE:
                env_release_s   = 0.05f + 0.22f * env_decay_s;
                sustain_level   = 0.0f;
                pitch_env_amt   = 0.30f + 0.45f * (1.0f - voice_blend);
                particle_amount = 0.45f + 0.50f * timbre;
                dust_amount     = 0.55f + 0.45f * timbre;
                output_level    = 0.72f;
                break;

            default: break;
        }

        amp_env.SetDecayTime(env_decay_s);
        amp_env.SetSustainLevel(sustain_level);
        amp_env.SetReleaseTime(env_release_s);

        pitch_env.SetTime(ADENV_SEG_DECAY, 0.02f + 0.12f * (1.0f - cutoff_norm));
        ui_needs_refresh = true;
    }
}

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    (void)in;

    const float sr = hw.AudioSampleRate();

    chorus.SetLfoFreq(0.06f + 2.4f * timbre);
    chorus.SetLfoDepth(0.04f + 0.52f * timbre);
    chorus.SetDelay(0.22f + 0.30f * timbre);
    chorus.SetFeedback(0.08f + 0.22f * timbre);

    delay_l.SetDelay(MsToSamplesClamped<32768>(delay_time_ms, sr));
    delay_r.SetDelay(MsToSamplesClamped<32768>(delay_time_ms * 1.20f, sr));

    reverb_l.SetDelay(MsToSamplesClamped<8192>(30.0f + 55.0f * reverb_amount, sr));
    reverb_r.SetDelay(MsToSamplesClamped<8192>(38.0f + 68.0f * reverb_amount, sr));

    dist_l.SetDrive(dist_drive);
    dist_r.SetDrive(dist_drive);

    for(size_t i = 0; i < size; i++)
    {
        const bool gate = midi_gate || drone_mode;
        const float env = amp_env.Process(gate);

        if(gate)
            gate_meter = 1.0f;
        gate_meter *= 0.992f;

        const float pitch_mod = pitch_env.Process();
        const float cur_freq  = fclamp(note_freq * (1.0f + pitch_env_amt * pitch_mod), 20.0f, 4000.0f);

        grainlet.SetFreq(cur_freq);
        grainlet.SetFormantFreq(cur_freq * (1.2f + 4.2f * timbre));
        grainlet.SetShape(0.12f + 2.25f * timbre);
        grainlet.SetBleed(0.10f + 0.88f * timbre);

        particle.SetDensity(0.02f + particle_amount * (1.0f - voice_blend));
        particle.SetFreq(90.0f + cur_freq * (0.50f + 1.35f * timbre));
        particle.SetResonance(0.08f + 0.82f * timbre);
        particle.SetSpread(0.25f + 2.75f * timbre);
        particle.SetGain(0.08f + 0.90f * particle_amount * (1.0f - voice_blend));
        particle.SetRandomFreq(0.6f + 6.8f * timbre);

        dust.SetDensity(0.02f + 0.95f * dust_amount);

        const float grain = grainlet.Process();
        const float part  = particle.Process();
        const float d_imp = dust.Process();

        dust_meter = fmaxf(dust_meter * 0.985f, d_imp);

        dust_burst = fmaxf(dust_burst * 0.990f, d_imp * (0.15f + 0.85f * timbre));

        const float grain_mix = 0.58f + 0.42f * voice_blend;
        const float part_mix  = (1.0f - voice_blend) * (0.25f + 0.75f * particle_amount);
        const float dust_mix  = (1.0f - voice_blend) * 0.55f * dust_amount;

        float voice = grain_mix * grain + part_mix * part + dust_mix * dust_burst;

        voice_filter.SetFreq(80.0f + cutoff_norm * 7600.0f + cur_freq * (0.30f + 0.80f * timbre));
        voice_filter.SetRes(0.08f + 0.86f * timbre);
        voice_filter.Process(voice);
        voice = voice_filter.Low();

        const float vel = drone_mode ? 0.75f : (0.12f + 0.88f * note_vel);
        const float dry = voice * env * vel * 0.82f;

        (void)chorus.Process(dry);
        const float ch_l = chorus.GetLeft();
        const float ch_r = chorus.GetRight();

        const float dly_l = delay_l.Read();
        const float dly_r = delay_r.Read();
        delay_l.Write(ch_l * fx_amount + dly_r * delay_feedback);
        delay_r.Write(ch_r * fx_amount + dly_l * delay_feedback);

        const float rv_in_l = (ch_l * reverb_amount) + (dly_l * 0.45f) + (rev_state_r * reverb_feedback);
        const float rv_in_r = (ch_r * reverb_amount) + (dly_r * 0.45f) + (rev_state_l * reverb_feedback);
        const float rv_l    = reverb_l.Allpass(rv_in_l, 1540, 0.63f);
        const float rv_r    = reverb_r.Allpass(rv_in_r, 1910, 0.61f);
        rev_state_l         = rv_l;
        rev_state_r         = rv_r;

        const float fx_l = (ch_l * fx_amount * 0.55f) + (dly_l * fx_amount) + (rv_l * reverb_amount);
        const float fx_r = (ch_r * fx_amount * 0.55f) + (dly_r * fx_amount) + (rv_r * reverb_amount);

        const float pre_l = dry * (1.0f - 0.65f * fx_amount) + fx_l;
        const float pre_r = dry * (1.0f - 0.65f * fx_amount) + fx_r;

        const float drv_l = dist_l.Process(pre_l);
        const float drv_r = dist_r.Process(pre_r);

        const float out_l = fclamp(((0.26f * pre_l) + (0.74f * drv_l)) * output_level, -0.96f, 0.96f);
        const float out_r = fclamp(((0.26f * pre_r) + (0.74f * drv_r)) * output_level, -0.96f, 0.96f);

        out[0][i] = out_l;
        out[1][i] = out_r;
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);

    hw.seed.StartLog(true);
    hw.seed.PrintLine("=== Field_SynthFxWorkstation Boot ===");
    PrintSerialHelp();

    const float sr = hw.AudioSampleRate();

    grainlet.Init(sr);
    particle.Init(sr);
    dust.Init();

    amp_env.Init(sr);
    amp_env.SetAttackTime(0.002f);
    amp_env.SetDecayTime(0.25f);
    amp_env.SetSustainLevel(0.62f);
    amp_env.SetReleaseTime(0.22f);

    pitch_env.Init(sr);
    pitch_env.SetMin(0.0f);
    pitch_env.SetMax(1.0f);
    pitch_env.SetCurve(0.0f);
    pitch_env.SetTime(ADENV_SEG_ATTACK, 0.001f);
    pitch_env.SetTime(ADENV_SEG_DECAY, 0.06f);

    voice_filter.Init(sr);
    voice_filter.SetDrive(0.35f);

    chorus.Init(sr);
    chorus.SetPan(0.25f, 0.75f);

    delay_l.Init();
    delay_r.Init();
    reverb_l.Init();
    reverb_r.Init();

    dist_l.Init();
    dist_r.Init();

    ApplyPatch(PATCH_BASS);
    LogStatus(true);
    UpdateOled();
    UpdateLeds();

    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    hw.midi.StartReceive();

    while(1)
    {
        UpdateControls();

        hw.midi.Listen();
        while(hw.midi.HasEvents())
        {
            HandleMidiMessage(hw.midi.PopEvent());
        }

        UpdateOled();
        UpdateLeds();
        LogStatus(false);
        System::Delay(1);
    }
}
