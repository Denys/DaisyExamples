#include "daisysp.h"
#include "daisy_patch.h"
#include "daisyhost/apps/MultiDelayCore.h"

#include <array>
#include <string>

using namespace daisy;
using namespace daisyhost;
using namespace daisyhost::apps;

namespace
{
DaisyPatch patch;
MultiDelayCore core("node0");
MultiDelayCore::DelayLineType DSY_SDRAM_BSS
    sdramDelays[MultiDelayCore::kDelayCount];

const std::array<std::string, 4> kKnobIds = {{
    MultiDelayCore::MakeKnobControlId("node0", 1),
    MultiDelayCore::MakeKnobControlId("node0", 2),
    MultiDelayCore::MakeKnobControlId("node0", 3),
    MultiDelayCore::MakeKnobControlId("node0", 4),
}};
} // namespace

void ProcessControls();
void UpdateOled();

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    ProcessControls();

    const std::array<const float*, 1> inputPtrs = {{in[0]}};
    const std::array<float*, 4> outputPtrs = {{out[0], out[1], out[2], out[3]}};

    core.Process({inputPtrs.data(), inputPtrs.size()},
                 {outputPtrs.data(), outputPtrs.size()},
                 size);
    core.TickUi((1000.0 * static_cast<double>(size)) / patch.AudioSampleRate());
}

int main(void)
{
    patch.Init();
    core.AttachDelayStorage(sdramDelays, MultiDelayCore::kDelayCount);
    core.Prepare(patch.AudioSampleRate(), patch.AudioBlockSize());

    patch.StartAdc();
    patch.StartAudio(AudioCallback);

    while(1)
    {
        UpdateOled();
    }
}

void UpdateOled()
{
    patch.display.Fill(false);

    const DisplayModel& model = core.GetDisplayModel();
    for(const auto& text : model.texts)
    {
        patch.display.SetCursor(text.x, text.y);
        patch.display.WriteString(text.text.c_str(), Font_7x10, !text.inverted);
    }

    for(const auto& bar : model.bars)
    {
        const int x2 = bar.x + bar.width;
        const int y2 = bar.y + bar.height;
        patch.display.DrawRect(bar.x, bar.y, x2, y2, true, false);

        const int fillWidth = static_cast<int>(bar.width * bar.normalized);
        if(fillWidth > 0)
        {
            patch.display.DrawRect(
                bar.x, bar.y, bar.x + fillWidth, y2, true, true);
        }
    }

    patch.display.Update();
}

void ProcessControls()
{
    patch.ProcessAnalogControls();
    patch.ProcessDigitalControls();

    for(int i = 0; i < 4; ++i)
    {
        core.SetControl(kKnobIds[static_cast<std::size_t>(i)],
                        patch.controls[i].Value());
    }

    core.SetEncoderDelta(patch.encoder.Increment());
    core.SetEncoderPress(patch.encoder.Pressed());
}
