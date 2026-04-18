#include "daisyhost/MidiLearnMap.h"

#include <algorithm>
#include <sstream>

namespace daisyhost
{
void MidiLearnMap::Assign(int cc, const std::string& controlId)
{
    bindings_[cc] = controlId;
}

void MidiLearnMap::Clear(int cc)
{
    bindings_.erase(cc);
}

bool MidiLearnMap::TryLookup(int cc, std::string* controlId) const
{
    const auto it = bindings_.find(cc);
    if(it == bindings_.end())
    {
        return false;
    }

    if(controlId != nullptr)
    {
        *controlId = it->second;
    }

    return true;
}

std::vector<MidiBinding> MidiLearnMap::Bindings() const
{
    std::vector<MidiBinding> bindings;
    bindings.reserve(bindings_.size());
    for(const auto& item : bindings_)
    {
        bindings.push_back({item.first, item.second});
    }

    std::sort(bindings.begin(),
              bindings.end(),
              [](const MidiBinding& lhs, const MidiBinding& rhs) {
                  return lhs.cc < rhs.cc;
              });
    return bindings;
}

std::string MidiLearnMap::Serialize() const
{
    std::ostringstream stream;
    const auto         bindings = Bindings();
    for(const auto& binding : bindings)
    {
        stream << "midi " << binding.cc << ' ' << binding.controlId << '\n';
    }
    return stream.str();
}

MidiLearnMap MidiLearnMap::Deserialize(const std::string& text)
{
    MidiLearnMap       map;
    std::istringstream stream(text);
    std::string        tag;
    while(stream >> tag)
    {
        if(tag != "midi")
        {
            std::string ignored;
            std::getline(stream, ignored);
            continue;
        }

        int         cc = 0;
        std::string controlId;
        if(stream >> cc >> controlId)
        {
            map.Assign(cc, controlId);
        }
    }

    return map;
}
} // namespace daisyhost
