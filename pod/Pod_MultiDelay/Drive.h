#pragma once
#include "DigiMono.h"
#include <cstdint>
namespace poddemo
{
// v0.5 PRE drive: OVERDRIVE / DISTORTION / FUZZ (PROPOSED laws, not DAFX authority).
// Chain per type: pre high-pass -> gain -> first-order ADAA clipper -> tone low-pass
// -> DC blocker -> (FUZZ gate) -> LEVEL -> BLEND with clean -> enable crossfade.
// OFF is the exact clean input with no added latency, as in v0.4.
enum class DriveType : int
{
    Overdrive,
    Distortion,
    Fuzz
};
constexpr int kDriveTypes = 3;

struct DriveParams
{
    int   type  = 1;   // DriveType
    float gain  = .4f; // 0..1 -> per-type dB range
    float tone  = .5f; // 0..1 -> 1..12 kHz low-pass
    float level = .8f; // 0..1 -> -24..+6 dB
    float blend = 1;   // 0 clean .. 1 driven
    bool  on    = false;
};

struct DriveLaw
{
    float minDb, maxDb, preHpHz;
};
// Gain ranges: v0.4 DIST reached +29.5 dB; v0.5 DISTORTION reaches +50 dB, FUZZ +66 dB.
constexpr DriveLaw kDriveLaws[kDriveTypes]
    = {{0, 30, 720}, {6, 50, 150}, {20, 66, 80}};
constexpr float kFuzzLow = .6f; // asymmetric FUZZ clip: [-0.6, +1]

inline int DriveTypeIndex(int t)
{
    return std::clamp(t, 0, kDriveTypes - 1);
}
inline float DriveGainDb(int type, float g)
{
    const auto &l = kDriveLaws[DriveTypeIndex(type)];
    return l.minDb + (l.maxDb - l.minDb) * std::clamp(Finite(g), 0.0f, 1.0f);
}
inline float DriveLevelDb(float v)
{
    return -24 + 30 * std::clamp(Finite(v), 0.0f, 1.0f);
}
inline float DriveToneHz(float t)
{
    return 1000 * std::pow(12.0f, std::clamp(Finite(t), 0.0f, 1.0f));
}

// Static clipping laws f(u) and antiderivatives F(u), in double so the ADAA quotient
// keeps precision at FUZZ gains (u up to ~2000).
struct Shaper
{
    static double f(int type, double u)
    {
        switch(DriveTypeIndex(type))
        {
            case 0: return std::tanh(u);
            case 1:
            {
                if(u > 1.5)
                    return 1;
                if(u < -1.5)
                    return -1;
                return u * (1 - u * u / 6.75);
            }
            default: return std::clamp(u, -double(kFuzzLow), 1.0);
        }
    }
    static double F(int type, double u)
    {
        switch(DriveTypeIndex(type))
        {
            case 0:
            {
                // log(cosh(u)) without overflow.
                const double a = std::abs(u);
                return a + std::log1p(std::exp(-2 * a)) - 0.69314718055994531;
            }
            case 1:
            {
                const double a = std::abs(u);
                if(a > 1.5)
                    return 0.9375 + (a - 1.5);
                return u * u / 2 - u * u * u * u / 27;
            }
            default:
            {
                const double lo = kFuzzLow;
                if(u > 1)
                    return 0.5 + (u - 1);
                if(u < -lo)
                    return lo * lo / 2 - lo * (u + lo);
                return u * u / 2;
            }
        }
    }
};

// First-order antiderivative anti-aliasing (Parker, Zavalishin, Le Bivic, DAFx 2016).
class Adaa
{
    double u1_ = 0, F1_ = 0;
    int    type_ = 1;

  public:
    void Reset(int type, double u = 0)
    {
        type_ = DriveTypeIndex(type);
        u1_   = u;
        F1_   = Shaper::F(type_, u);
    }
    float Process(double u)
    {
        const double F = Shaper::F(type_, u), d = u - u1_;
        const double y = std::abs(d) > 1e-6 ? (F - F1_) / d
                                            : Shaper::f(type_, .5 * (u + u1_));
        u1_ = u;
        F1_ = F;
        return static_cast<float>(y);
    }
    // Plain sampling of the same law, kept for the alias comparison test.
    float Naive(double u) const { return float(Shaper::f(type_, u)); }
};

class Drive
{
    static constexpr float kTwoPi = 6.28318530718f;
    static float OnePole(float hz) { return 1 - std::exp(-kTwoPi * hz / kFs); }
    Adaa         adaa_;
    int          type_ = 1, request_ = 1;
    float        gain_ = 1, gainTarget_ = 1, level_ = 1, levelTarget_ = 1;
    float        tone_ = 0, toneTarget_ = 0, blendMix_ = 1, blendMixTarget_ = 1;
    float        blend_ = 0, blendTarget_ = 0; // enable crossfade, 240 samples
    float        hpCoeff_ = 0, hpLow_ = 0, low_ = 0, dcIn_ = 0, dcOut_ = 0;
    float        env_ = 0, gate_ = 1;
    bool         on_ = false, gateOpen_ = true;

    void Targets(const DriveParams &p)
    {
        gainTarget_     = std::pow(10.0f, DriveGainDb(type_, p.gain) / 20);
        levelTarget_    = std::pow(10.0f, DriveLevelDb(p.level) / 20);
        toneTarget_     = OnePole(DriveToneHz(p.tone));
        blendMixTarget_ = std::clamp(Finite(p.blend), 0.0f, 1.0f);
        hpCoeff_        = OnePole(kDriveLaws[type_].preHpHz);
    }
    void Switch()
    {
        type_ = request_;
        adaa_.Reset(type_);
        hpLow_ = low_ = dcIn_ = dcOut_ = 0;
    }
    DriveParams last_;

  public:
    int  Type() const { return type_; }
    bool Switching() const { return request_ != type_; }
    void Configure(const DriveParams &p)
    {
        last_    = p;
        request_ = DriveTypeIndex(p.type);
        on_      = p.on;
        // A type change mutes to clean first (5 ms), switches, then fades back in.
        // While fading out, the old type keeps its own targets.
        if(request_ != type_)
        {
            if(blend_ == 0)
            {
                Switch();
                Targets(p);
                gain_  = gainTarget_;
                level_ = levelTarget_;
            }
        }
        else
            Targets(p);
        blendTarget_ = on_ && request_ == type_ ? 1.0f : 0.0f;
    }
    void Init(const DriveParams &p)
    {
        request_ = type_ = DriveTypeIndex(p.type);
        adaa_.Reset(type_);
        Configure(p);
        gain_     = gainTarget_;
        level_    = levelTarget_;
        tone_     = toneTarget_;
        blendMix_ = blendMixTarget_;
        blend_    = 0;
    }
    float Process(float input)
    {
        const float clean = std::clamp(Finite(input), -1.0f, 1.0f);
        gain_ += (gainTarget_ - gain_) * .002f;
        level_ += (levelTarget_ - level_) * .002f;
        tone_ += (toneTarget_ - tone_) * .002f;
        blendMix_ += (blendMixTarget_ - blendMix_) * .002f;
        hpLow_ += (clean - hpLow_) * hpCoeff_;
        const float pre = (clean - hpLow_) * gain_;
        float       wet = adaa_.Process(pre);
        low_ += (wet - low_) * tone_;
        // DC blocker (~10 Hz): the asymmetric FUZZ law produces DC.
        const float dc = low_ - dcIn_ + .99869f * dcOut_;
        dcIn_          = low_;
        dcOut_         = Finite(dc);
        wet            = dcOut_;
        if(type_ == static_cast<int>(DriveType::Fuzz))
        {
            // PROPOSED gate: open above -54 dBFS, close below -60 dBFS at the input.
            const float a = std::abs(clean);
            env_ += (a - env_) * (a > env_ ? .02f : .0004f);
            gateOpen_ = gateOpen_ ? env_ > .001f : env_ > .002f;
            gate_ += ((gateOpen_ ? 1.0f : 0.0f) - gate_)
                     * (gateOpen_ ? .01f : .0005f);
            wet *= gate_;
        }
        const float mixed
            = clean
              + blendMix_ * (std::clamp(wet * level_, -1.0f, 1.0f) - clean);
        const float step = 1.0f / 240;
        if(blend_ < blendTarget_)
            blend_ = std::min(blendTarget_, blend_ + step);
        else if(blend_ > blendTarget_)
            blend_ = std::max(blendTarget_, blend_ - step);
        if(blend_ == 0 && request_ != type_)
        {
            Switch();
            Targets(last_);
            gain_        = gainTarget_;
            level_       = levelTarget_;
            blendTarget_ = on_ ? 1.0f : 0.0f;
        }
        // Exact clean identity once OFF, including at startup; keep filters warm.
        return blend_ == 0
                   ? clean
                   : std::clamp(
                       Finite(clean + blend_ * (mixed - clean)), -1.0f, 1.0f);
    }
};
} // namespace poddemo
