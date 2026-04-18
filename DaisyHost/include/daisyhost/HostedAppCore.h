#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "daisyhost/DisplayModel.h"
#include "daisyhost/VirtualPort.h"

namespace daisyhost
{
struct AudioBufferView
{
    const float* const* channels     = nullptr;
    std::size_t         channelCount = 0;
};

struct AudioBufferWriteView
{
    float* const* channels     = nullptr;
    std::size_t   channelCount = 0;
};

struct MidiMessageEvent
{
    std::uint8_t status = 0;
    std::uint8_t data1  = 0;
    std::uint8_t data2  = 0;
};

struct PortValue
{
    VirtualPortType               type   = VirtualPortType::kAudio;
    float                         scalar = 0.0f;
    bool                          gate   = false;
    std::vector<MidiMessageEvent> midiEvents;
};

class HostedAppCore
{
  public:
    virtual ~HostedAppCore() {}

    virtual void Prepare(double sampleRate, std::size_t maxBlockSize) = 0;
    virtual void Process(const AudioBufferView& input,
                         const AudioBufferWriteView& output,
                         std::size_t frameCount)
        = 0;
    virtual void SetControl(const std::string& controlId, float normalizedValue)
        = 0;
    virtual void SetEncoderDelta(int delta)                = 0;
    virtual void SetEncoderPress(bool pressed)             = 0;
    virtual void SetPortInput(const std::string& portId, const PortValue& value)
        = 0;
    virtual PortValue GetPortOutput(const std::string& portId) const = 0;
    virtual void TickUi(double deltaMs)                             = 0;
    virtual const DisplayModel& GetDisplayModel() const             = 0;
};
} // namespace daisyhost
