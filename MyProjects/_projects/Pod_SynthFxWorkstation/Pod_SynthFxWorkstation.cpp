/**
 * Pod_SynthFxWorkstation
 *
 * Synth + FX workstation (no audio input path):
 * MIDI -> Grainlet + Particle + Dust voice -> FX bus -> Distortion -> Stereo Out
 */

#include "daisy_pod.h"
#include "daisysp.h"
#include <cmath>

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

// Voice core
GrainletOscillator grainlet;
Particle           particle;
Dust               dust;
Adsr               amp_env;
Svf                voice_filter;

// FX bus
Chorus chorus;

// Delay and simple diffusion reverb network.
DelayLine<float, 22050> delay_l, delay_r;
DelayLine<float, 8192>  reverb_l, reverb_r;

// Post-FX distortion
Overdrive dist_l, dist_r;

enum ParamPage
{
    PAGE_VOICE = 0,
    PAGE_FX    = 1,
    PAGE_DRIVE = 2,
    PAGE_COUNT
};

volatile ParamPage page = PAGE_VOICE;

// MIDI / voice state
volatile float note_freq     = 110.0f;
volatile float note_velocity = 0.0f;
volatile bool  midi_gate     = false;
volatile bool  drone_mode    = false;
volatile int   last_note     = -1;

// Parameters
volatile float voice_blend = 0.65f; // 1.0 = Grainlet dominant
volatile float timbre      = 0.55f;

volatile float fx_amount      = 0.45f;
volatile float delay_time_ms  = 260.0f;
volatile float delay_feedback = 0.42f;

volatile float dist_drive     = 0.55f;
volatile float reverb_amount  = 0.35f;
volatile float reverb_feedback = 0.62f;

volatile int fx_preset = 0; // 0..3

// Visuals
volatile float beat_pulse  = 0.0f;
volatile float dust_pulse  = 0.0f;
volatile float gate_pulse  = 0.0f;
volatile float step_hz_acc = 0.0f;

// Reverb state
float rev_state_l = 0.0f;
float rev_state_r = 0.0f;

// Dust transient accumulator
float dust_burst = 0.0f;

// Serial monitor telemetry
uint32_t last_status_log_ms = 0;
bool     serial_help_printed = false;

static inline float Clamp01(float x)
{
    return fclamp(x, 0.0f, 1.0f);
}

template <size_t N>
static inline size_t MsToSamplesClamped(float ms, float sr)
{
    const float s = (ms * 0.001f) * sr;
    return static_cast<size_t>(fclamp(s, 1.0f, static_cast<float>(N - 1)));
}

void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:
        {
            const NoteOnEvent n = m.AsNoteOn();
            if(n.velocity > 0)
            {
                note_freq     = mtof(static_cast<float>(n.note));
                note_velocity = n.velocity / 127.0f;
                midi_gate     = true;
                last_note     = n.note;
                hw.seed.PrintLine("[MIDI] NoteOn note=%d vel=%d freq=%.2f",
                                  n.note,
                                  n.velocity,
                                  note_freq);
            }
            else if(n.note == last_note)
            {
                midi_gate = false;
                hw.seed.PrintLine("[MIDI] NoteOff(via vel0) note=%d", n.note);
            }
        }
        break;

        case NoteOff:
        {
            const NoteOffEvent n = m.AsNoteOff();
            if(n.note == last_note)
            {
                midi_gate = false;
                hw.seed.PrintLine("[MIDI] NoteOff note=%d", n.note);
            }
        }
        break;

        default: break;
    }
}

const char* GetPageName(ParamPage p)
{
    switch(p)
    {
        case PAGE_VOICE: return "VOICE";
        case PAGE_FX: return "FX";
        case PAGE_DRIVE: return "DRIVE";
        default: return "UNKNOWN";
    }
}

void PrintSerialHelp()
{
    if(serial_help_printed)
        return;
    serial_help_printed = true;

    hw.seed.PrintLine("=== SERIAL MONITOR INTERFACE: Pod_SynthFxWorkstation ===");
    hw.seed.PrintLine("Controls:");
    hw.seed.PrintLine("  Encoder click: Page (VOICE -> FX -> DRIVE)");
    hw.seed.PrintLine("  B1: Drone mode toggle");
    hw.seed.PrintLine("  B2: FX preset cycle");
    hw.seed.PrintLine("  VOICE page: K1 blend, K2 timbre");
    hw.seed.PrintLine("  FX page:    K1 amount, K2 delay-time");
    hw.seed.PrintLine("  DRIVE page: K1 drive, K2 reverb amount");
    hw.seed.PrintLine("Telemetry:");
    hw.seed.PrintLine("  [STATUS] periodic system snapshot");
    hw.seed.PrintLine("  [MIDI] incoming note events");
}

void LogStatus(bool force)
{
    const uint32_t now = System::GetNow();
    if(!force && (now - last_status_log_ms) < 500)
        return;
    last_status_log_ms = now;

    hw.seed.PrintLine("[STATUS] page=%s gate=%d drone=%d note=%d freq=%.1f "
                      "blend=%.2f timbre=%.2f fx=%.2f dly=%.0fms fb=%.2f drv=%.2f rvb=%.2f preset=%d",
                      GetPageName(page),
                      midi_gate ? 1 : 0,
                      drone_mode ? 1 : 0,
                      last_note,
                      note_freq,
                      voice_blend,
                      timbre,
                      fx_amount,
                      delay_time_ms,
                      delay_feedback,
                      dist_drive,
                      reverb_amount,
                      fx_preset);
}

void UpdateVoiceParams()
{
    const float f = fclamp(note_freq, 20.0f, 2000.0f);

    grainlet.SetFreq(f);
    grainlet.SetFormantFreq(f * (1.6f + 4.0f * timbre));
    grainlet.SetShape(0.15f + 2.20f * timbre);
    grainlet.SetBleed(0.18f + 0.82f * timbre);

    particle.SetDensity(0.05f + (1.0f - voice_blend) * 0.75f);
    particle.SetFreq(140.0f + f * (0.6f + 1.4f * timbre));
    particle.SetResonance(0.20f + 0.70f * timbre);
    particle.SetSpread(0.20f + 2.80f * timbre);
    particle.SetGain(0.20f + 0.70f * (1.0f - voice_blend));
    particle.SetRandomFreq(0.8f + 7.0f * timbre);

    dust.SetDensity(0.10f + 0.80f * timbre);

    voice_filter.SetFreq(180.0f + (f * (1.8f + 3.2f * timbre)));
    voice_filter.SetRes(0.12f + 0.78f * timbre);
}

void UpdateFxParams(float sr)
{
    chorus.SetLfoFreq(0.06f + 2.2f * timbre);
    chorus.SetLfoDepth(0.08f + 0.55f * timbre);
    chorus.SetDelay(0.25f + 0.35f * timbre);
    chorus.SetFeedback(0.10f + 0.25f * timbre);

    delay_l.SetDelay(MsToSamplesClamped<22050>(delay_time_ms, sr));
    delay_r.SetDelay(MsToSamplesClamped<22050>(delay_time_ms * 1.22f, sr));

    // Two diffusion delays for a compact "pod reverb" tail.
    reverb_l.SetDelay(MsToSamplesClamped<8192>(35.0f + 55.0f * reverb_amount, sr));
    reverb_r.SetDelay(MsToSamplesClamped<8192>(43.0f + 70.0f * reverb_amount, sr));

    dist_l.SetDrive(dist_drive);
    dist_r.SetDrive(dist_drive);
}

void ProcessControls()
{
    hw.ProcessDigitalControls();
    hw.ProcessAnalogControls();

    if(hw.encoder.RisingEdge())
    {
        page = static_cast<ParamPage>((page + 1) % PAGE_COUNT);
        hw.seed.PrintLine("[PAGE] %d", static_cast<int>(page) + 1);
    }

    if(hw.button1.RisingEdge())
    {
        drone_mode = !drone_mode;
        hw.seed.PrintLine("[DRONE] %s", drone_mode ? "ON" : "OFF");
    }

    if(hw.button2.RisingEdge())
    {
        fx_preset = (fx_preset + 1) & 0x3;
        hw.seed.PrintLine("[FX] preset=%d", fx_preset);
    }

    const float k1 = hw.knob1.Process();
    const float k2 = hw.knob2.Process();

    switch(page)
    {
        case PAGE_VOICE:
            voice_blend = k1;
            timbre      = k2;
            break;

        case PAGE_FX:
            fx_amount      = k1;
            delay_time_ms  = 35.0f + 420.0f * k2;
            delay_feedback = 0.20f + 0.74f * k2;
            break;

        case PAGE_DRIVE:
            dist_drive      = 0.05f + 0.95f * k1;
            reverb_amount   = k2;
            reverb_feedback = 0.35f + 0.60f * k2;
            break;

        default: break;
    }
}

void UpdateLeds()
{
    beat_pulse *= 0.94f;
    dust_pulse *= 0.92f;
    gate_pulse *= 0.95f;

    static const float page_color[PAGE_COUNT][3] = {
        {0.05f, 0.55f, 0.95f}, // Voice
        {0.10f, 0.85f, 0.30f}, // FX
        {0.90f, 0.18f, 0.12f}, // Drive
    };

    const float pulse = 0.22f + 0.78f * beat_pulse;
    hw.led1.Set(page_color[page][0] * pulse,
                page_color[page][1] * pulse,
                page_color[page][2] * pulse);

    if(drone_mode || midi_gate)
    {
        hw.led2.Set(0.08f + 0.75f * gate_pulse, 0.04f + 0.65f * dust_pulse, 0.10f);
    }
    else
    {
        hw.led2.Set(0.08f, 0.02f, 0.02f);
    }

    hw.UpdateLeds();
}

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    (void)in;

    const float sr = hw.AudioSampleRate();
    UpdateVoiceParams();
    UpdateFxParams(sr);

    // Preset scalars
    static const float preset_chorus[4] = {0.00f, 0.75f, 0.30f, 0.55f};
    static const float preset_delay[4]  = {0.00f, 0.20f, 0.85f, 0.45f};
    static const float preset_reverb[4] = {0.00f, 0.15f, 0.35f, 0.95f};

    const float ch_send = fx_amount * preset_chorus[fx_preset];
    const float dl_send = fx_amount * preset_delay[fx_preset];
    const float rv_send = fx_amount * preset_reverb[fx_preset];

    for(size_t i = 0; i < size; i += 2)
    {
        const bool gate = midi_gate || drone_mode;
        const float env = amp_env.Process(gate);

        // Beat pulse proxy from estimated pitch motion.
        step_hz_acc += note_freq / sr;
        if(step_hz_acc >= 1.0f)
        {
            step_hz_acc -= 1.0f;
            beat_pulse = 1.0f;
        }
        if(gate)
            gate_pulse = 1.0f;

        const float grain = grainlet.Process();
        const float part  = particle.Process();

        const float dust_imp = dust.Process();
        if(dust_imp > 0.22f)
            dust_pulse = 1.0f;
        dust_burst = fmaxf(dust_burst * 0.992f, dust_imp * (0.15f + 0.85f * timbre));
        const float dust_sig = dust_burst;

        const float grain_mix = 0.55f + 0.45f * voice_blend;
        const float part_mix  = (1.0f - voice_blend) * 0.70f;
        const float dust_mix  = (1.0f - voice_blend) * 0.45f;

        float voice = (grain_mix * grain) + (part_mix * part) + (dust_mix * dust_sig);
        voice_filter.Process(voice);
        voice = voice_filter.Low();

        const float vel = drone_mode ? 0.6f : (0.12f + 0.88f * note_velocity);
        const float dry = voice * env * vel * 0.70f;

        // Chorus (stereo)
        (void)chorus.Process(dry);
        const float ch_l = chorus.GetLeft();
        const float ch_r = chorus.GetRight();

        // Delay
        const float dly_l = delay_l.Read();
        const float dly_r = delay_r.Read();
        delay_l.Write(ch_l * dl_send + dly_r * delay_feedback);
        delay_r.Write(ch_r * dl_send + dly_l * delay_feedback);

        // Simple diffused reverb
        const float rv_in_l = (ch_l * rv_send) + (dly_l * 0.5f) + (rev_state_r * reverb_feedback);
        const float rv_in_r = (ch_r * rv_send) + (dly_r * 0.5f) + (rev_state_l * reverb_feedback);
        const float rv_l    = reverb_l.Allpass(rv_in_l, 1900, 0.63f);
        const float rv_r    = reverb_r.Allpass(rv_in_r, 2300, 0.61f);
        rev_state_l         = rv_l;
        rev_state_r         = rv_r;

        const float fx_l = (ch_l * ch_send) + (dly_l * dl_send) + (rv_l * rv_send);
        const float fx_r = (ch_r * ch_send) + (dly_r * dl_send) + (rv_r * rv_send);

        // FX bus -> distortion -> output
        const float pre_l = dry * (1.0f - 0.70f * fx_amount) + fx_l;
        const float pre_r = dry * (1.0f - 0.70f * fx_amount) + fx_r;
        const float drv_l = dist_l.Process(pre_l);
        const float drv_r = dist_r.Process(pre_r);

        const float out_l = fclamp((0.25f * pre_l) + (0.75f * drv_l), -0.95f, 0.95f);
        const float out_r = fclamp((0.25f * pre_r) + (0.75f * drv_r), -0.95f, 0.95f);

        out[i]     = out_l;
        out[i + 1] = out_r;
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);

    hw.seed.StartLog(true);
    hw.seed.PrintLine("=== Pod_SynthFxWorkstation Boot ===");
    PrintSerialHelp();

    hw.seed.usb_handle.Init(UsbHandle::FS_INTERNAL);
    System::Delay(250);

    const float sr = hw.AudioSampleRate();

    grainlet.Init(sr);
    particle.Init(sr);
    dust.Init();

    amp_env.Init(sr);
    amp_env.SetAttackTime(0.003f);
    amp_env.SetDecayTime(0.12f);
    amp_env.SetSustainLevel(0.72f);
    amp_env.SetReleaseTime(0.30f);

    voice_filter.Init(sr);
    voice_filter.SetDrive(0.25f);

    chorus.Init(sr);
    chorus.SetPan(0.20f, 0.80f);

    delay_l.Init();
    delay_r.Init();
    reverb_l.Init();
    reverb_r.Init();

    dist_l.Init();
    dist_r.Init();

    UpdateVoiceParams();
    UpdateFxParams(sr);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    hw.midi.StartReceive();
    LogStatus(true);

    while(1)
    {
        ProcessControls();

        hw.midi.Listen();
        while(hw.midi.HasEvents())
        {
            HandleMidiMessage(hw.midi.PopEvent());
        }

        UpdateLeds();
        LogStatus(false);
        System::Delay(1);
    }
}
