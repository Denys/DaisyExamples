#include "daisyhost/HostSessionState.h"

#include <algorithm>
#include <sstream>
#include <vector>

namespace daisyhost
{
namespace
{
template <typename MapType>
std::vector<typename MapType::key_type> SortedKeys(const MapType& map)
{
    std::vector<typename MapType::key_type> keys;
    keys.reserve(map.size());
    for(const auto& item : map)
    {
        keys.push_back(item.first);
    }
    std::sort(keys.begin(), keys.end());
    return keys;
}
} // namespace

std::string HostSessionState::Serialize() const
{
    std::ostringstream stream;
    stream << "version 1\n";

    const auto controlKeys = SortedKeys(controlValues);
    for(const auto& key : controlKeys)
    {
        stream << "control " << key << ' ' << controlValues.at(key) << '\n';
    }

    const auto cvKeys = SortedKeys(cvValues);
    for(const auto& key : cvKeys)
    {
        stream << "cv " << key << ' ' << cvValues.at(key) << '\n';
    }

    const auto gateKeys = SortedKeys(gateValues);
    for(const auto& key : gateKeys)
    {
        stream << "gate " << key << ' ' << (gateValues.at(key) ? 1 : 0) << '\n';
    }

    stream << midiLearn.Serialize();
    return stream.str();
}

HostSessionState HostSessionState::Deserialize(const std::string& text)
{
    HostSessionState   state;
    std::istringstream stream(text);
    std::string        tag;

    while(stream >> tag)
    {
        if(tag == "version")
        {
            std::string ignored;
            std::getline(stream, ignored);
            continue;
        }

        if(tag == "control")
        {
            std::string id;
            float       value = 0.0f;
            if(stream >> id >> value)
            {
                state.controlValues[id] = value;
            }
            continue;
        }

        if(tag == "cv")
        {
            std::string id;
            float       value = 0.0f;
            if(stream >> id >> value)
            {
                state.cvValues[id] = value;
            }
            continue;
        }

        if(tag == "gate")
        {
            std::string id;
            int         value = 0;
            if(stream >> id >> value)
            {
                state.gateValues[id] = value != 0;
            }
            continue;
        }

        if(tag == "midi")
        {
            int         cc = 0;
            std::string controlId;
            if(stream >> cc >> controlId)
            {
                state.midiLearn.Assign(cc, controlId);
            }
            continue;
        }

        std::string ignored;
        std::getline(stream, ignored);
    }

    return state;
}
} // namespace daisyhost
