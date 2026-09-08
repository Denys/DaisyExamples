#include "src/app/Engine.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
namespace fs = std::filesystem;
void U16(std::ostream &o, std::uint16_t x)
{
    o.put(char(x & 255u));
    o.put(char(x >> 8u));
}
void U32(std::ostream &o, std::uint32_t x)
{
    U16(o, std::uint16_t(x & 65535u));
    U16(o, std::uint16_t(x >> 16u));
}
int main(int argc, char **argv)
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "usage: render_scenarios OUTPUT_DIR\n";
            return 2;
        }
        fs::path output = argv[1];
        fs::create_directories(output);
        std::ofstream metrics(output / "metrics.csv");
        if (!metrics)
            throw std::runtime_error("cannot create metrics");
        metrics << "preset,frames,peak,rms,dc,max_delta,tail_peak,faults\n";
        constexpr std::uint32_t sr = 48000, frames = sr * 10;
        for (std::uint8_t p = 0; p < 8; ++p)
        {
            hydrapulse::Engine e;
            e.Init();
            e.RequestPreset(p);
            for (int i = 0; i < 600; ++i)
                (void)e.Process();
            e.SetMaster(.7f);
            e.Start();
            const auto path = output / ("preset_" + std::to_string(p) + ".wav");
            std::ofstream wav(path, std::ios::binary);
            if (!wav)
                throw std::runtime_error("cannot create WAV");
            wav.write("RIFF", 4);
            U32(wav, 36u + frames * 4u);
            wav.write("WAVEfmt ", 8);
            U32(wav, 16);
            U16(wav, 1);
            U16(wav, 2);
            U32(wav, sr);
            U32(wav, sr * 4);
            U16(wav, 4);
            U16(wav, 16);
            wav.write("data", 4);
            U32(wav, frames * 4);
            double energy = 0, mean = 0, peak = 0, delta = 0, tail = 0;
            float prev_l = 0, prev_r = 0;
            for (std::uint32_t n = 0; n < frames; ++n)
            {
                if (n == sr * 3)
                    e.QueueBank(hydrapulse::Bank::B);
                if (n == sr * 5)
                    e.SetFill(true);
                if (n == sr * 6)
                    e.SetFill(false);
                if (n == sr * 7)
                    e.Panic();
                auto s = e.Process();
                for (float x : {s.left, s.right})
                {
                    if (!std::isfinite(x))
                        throw std::runtime_error("nonfinite output");
                    peak = std::max(peak, double(std::fabs(x)));
                    energy += double(x) * x;
                    mean += x;
                    if (n > sr * 9)
                        tail = std::max(tail, double(std::fabs(x)));
                    const auto pcm =
                        static_cast<std::int16_t>(std::lround(std::clamp(x, -1.0f, 1.0f) * 32767.0f));
                    U16(wav, static_cast<std::uint16_t>(pcm));
                }
                delta = std::max(delta,
                                 double(std::max(std::fabs(s.left - prev_l), std::fabs(s.right - prev_r))));
                prev_l = s.left;
                prev_r = s.right;
            }
            if (!wav)
                throw std::runtime_error("WAV write failed");
            metrics << unsigned(p) << ',' << frames << ',' << peak << ',' << std::sqrt(energy / (frames * 2))
                    << ',' << mean / (frames * 2) << ',' << delta << ',' << tail << ',' << e.Faults() << '\n';
        }
        return metrics ? 0 : 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
