
#include "daisy_field.h"
#include <cstdio>

using namespace daisy;
DaisyField hw;

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    printf("callback log");
    for(size_t i = 0; i < size; ++i)
    {
        out[0][i] = in[0][i] * hw.GetKnobValue(0);
    }
}

int main()
{
    hw.Init();
    hw.StartAudio(AudioCallback);
    while(true) { hw.ProcessAllControls(); }
}
