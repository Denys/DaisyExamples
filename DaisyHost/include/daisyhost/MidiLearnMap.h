#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace daisyhost
{
struct MidiBinding
{
    int         cc = 0;
    std::string controlId;
};

class MidiLearnMap
{
  public:
    void Assign(int cc, const std::string& controlId);
    void Clear(int cc);
    bool TryLookup(int cc, std::string* controlId) const;
    std::vector<MidiBinding> Bindings() const;

    std::string Serialize() const;
    static MidiLearnMap Deserialize(const std::string& text);

  private:
    std::unordered_map<int, std::string> bindings_;
};
} // namespace daisyhost
