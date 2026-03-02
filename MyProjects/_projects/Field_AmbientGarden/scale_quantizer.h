#pragma once
#include <cstdint>
#include <cmath>

/**
 * ScaleQuantizer - Maps raw 0-255 values to musical scale frequencies.
 *
 * Supports 8 scales selectable at runtime. Converts a raw byte value
 * to a MIDI note number within the selected scale and octave range,
 * then outputs frequency in Hz.
 */
class ScaleQuantizer
{
  public:
    static constexpr int kMaxScaleNotes = 12;
    static constexpr int kNumScales     = 8;

    void Init()
    {
        current_scale_ = 0;
        root_note_     = 48; // C3
        octave_range_  = 3;
    }

    /** Convert raw TM output (0-255) to frequency in Hz. */
    float Process(uint8_t raw_value)
    {
        int scale_size = GetScaleSize(current_scale_);
        if(scale_size <= 0)
            scale_size = 1;

        int total_notes = scale_size * octave_range_;
        int note_index  = (raw_value * total_notes) / 256;

        if(note_index < 0)
            note_index = 0;
        if(note_index >= total_notes)
            note_index = total_notes - 1;

        int octave = note_index / scale_size;
        int degree = note_index % scale_size;

        int midi_note = root_note_ + (octave * 12) + kScales[current_scale_][degree];

        return MidiToHz(midi_note);
    }

    /** Set active scale (0-7). */
    void SetScale(int idx)
    {
        if(idx >= 0 && idx < kNumScales)
            current_scale_ = idx;
    }

    /** Set root MIDI note (typically 36-72). */
    void SetRoot(int midi_root)
    {
        root_note_ = (midi_root < 24) ? 24 : (midi_root > 84) ? 84 : midi_root;
    }

    /** Set octave range (1-4). */
    void SetOctaveRange(int range)
    {
        octave_range_ = (range < 1) ? 1 : (range > 4) ? 4 : range;
    }

    int GetCurrentScale() const { return current_scale_; }
    int GetRoot() const { return root_note_; }

    /** Get scale name for display. */
    const char* GetScaleName() const { return kScaleNames[current_scale_]; }

    /** Get note name for a MIDI note number. */
    static const char* GetNoteName(int root_offset)
    {
        static const char* names[12] = {
            "C", "C#", "D", "D#", "E", "F",
            "F#", "G", "G#", "A", "A#", "B"};
        int idx = root_offset % 12;
        if(idx < 0)
            idx += 12;
        return names[idx];
    }

  private:
    int GetScaleSize(int scale_idx) const
    {
        for(int i = 0; i < kMaxScaleNotes; i++)
        {
            if(kScales[scale_idx][i] < 0)
                return i;
        }
        return kMaxScaleNotes;
    }

    static float MidiToHz(int note)
    {
        return 440.0f * powf(2.0f, (note - 69) / 12.0f);
    }

    int current_scale_;
    int root_note_;
    int octave_range_;

    // Scale definitions: semitone offsets from root. -1 = end marker.
    static constexpr int kScales[kNumScales][kMaxScaleNotes] = {
        {0, 2, 4, 7, 9, -1, -1, -1, -1, -1, -1, -1},       // Pentatonic Major
        {0, 2, 4, 5, 7, 9, 11, -1, -1, -1, -1, -1},         // Major (Ionian)
        {0, 2, 3, 5, 7, 8, 10, -1, -1, -1, -1, -1},         // Minor (Aeolian)
        {0, 2, 3, 5, 7, 9, 10, -1, -1, -1, -1, -1},         // Dorian
        {0, 2, 4, 5, 7, 9, 10, -1, -1, -1, -1, -1},         // Mixolydian
        {0, 3, 5, 7, 10, -1, -1, -1, -1, -1, -1, -1},       // Pentatonic Minor
        {0, 2, 4, 6, 8, 10, -1, -1, -1, -1, -1, -1},        // Whole Tone
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},             // Chromatic
    };

    static constexpr const char* kScaleNames[kNumScales] = {
        "PentMaj", "Major", "Minor", "Dorian",
        "Mixoly", "PentMin", "WhlTone", "Chromat"};
};

// C++14 requires out-of-class definitions for ODR-used static constexpr members
constexpr int         ScaleQuantizer::kScales[ScaleQuantizer::kNumScales][ScaleQuantizer::kMaxScaleNotes];
constexpr const char* ScaleQuantizer::kScaleNames[ScaleQuantizer::kNumScales];
