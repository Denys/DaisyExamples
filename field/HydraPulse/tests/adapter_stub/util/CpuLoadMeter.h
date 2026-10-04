#pragma once
namespace daisy
{
class CpuLoadMeter
{
  public:
    void Init(float, int) {}
    void OnBlockStart() {}
    void OnBlockEnd() {}
    float GetMaxCpuLoad() const
    {
        return 0;
    }
};
} // namespace daisy
