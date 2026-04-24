#include "daisyhost/HostSessionState.h"

#include <algorithm>
#include <sstream>
#include <vector>

#include "daisyhost/AppRegistry.h"

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

void EnsurePrimaryNodeMetadata(HostSessionState* state)
{
    if(state == nullptr || !state->nodes.empty())
    {
        return;
    }

    if(state->appId.empty() && state->controlValues.empty()
       && state->parameterValues.empty() && state->cvValues.empty()
       && state->gateValues.empty())
    {
        return;
    }

    HostSessionNodeState node;
    node.nodeId     = "node0";
    node.appId      = state->appId.empty() ? GetDefaultHostedAppId() : state->appId;
    node.randomSeed = state->randomSeed;
    state->nodes.push_back(std::move(node));
}

void EnsureRackDefaults(HostSessionState* state)
{
    if(state == nullptr)
    {
        return;
    }

    if(state->boardId.empty())
    {
        state->boardId = "daisy_patch";
    }
    if(state->selectedNodeId.empty())
    {
        state->selectedNodeId = "node0";
    }
    if(state->entryNodeId.empty())
    {
        state->entryNodeId = "node0";
    }
    if(state->outputNodeId.empty())
    {
        state->outputNodeId = "node0";
    }
}
} // namespace

std::string HostSessionState::Serialize() const
{
    std::ostringstream stream;
    stream << "version 5\n";
    stream << "board " << (boardId.empty() ? "daisy_patch" : boardId) << '\n';
    stream << "selected_node "
           << (selectedNodeId.empty() ? "node0" : selectedNodeId) << '\n';
    stream << "entry_node " << (entryNodeId.empty() ? "node0" : entryNodeId)
           << '\n';
    stream << "output_node "
           << (outputNodeId.empty() ? "node0" : outputNodeId) << '\n';

    std::vector<HostSessionNodeState> nodesToWrite = nodes;
    if(nodesToWrite.empty())
    {
        HostSessionNodeState node;
        node.nodeId     = "node0";
        node.appId      = appId.empty() ? GetDefaultHostedAppId() : appId;
        node.randomSeed = randomSeed;
        nodesToWrite.push_back(std::move(node));
    }
    for(const auto& node : nodesToWrite)
    {
        stream << "node " << node.nodeId << ' ' << node.appId << ' '
               << node.randomSeed << '\n';
    }

    const auto controlKeys = SortedKeys(controlValues);
    for(const auto& key : controlKeys)
    {
        stream << "control " << key << ' ' << controlValues.at(key) << '\n';
    }

    const auto parameterKeys = SortedKeys(parameterValues);
    for(const auto& key : parameterKeys)
    {
        stream << "param " << key << ' ' << parameterValues.at(key) << '\n';
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

    for(const auto& route : routes)
    {
        stream << "route " << route.sourcePortId << ' ' << route.destPortId << '\n';
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

        if(tag == "app")
        {
            std::string id;
            if(stream >> id)
            {
                state.appId = id;
            }
            continue;
        }

        if(tag == "board")
        {
            stream >> state.boardId;
            continue;
        }

        if(tag == "selected_node")
        {
            stream >> state.selectedNodeId;
            continue;
        }

        if(tag == "entry_node")
        {
            stream >> state.entryNodeId;
            continue;
        }

        if(tag == "output_node")
        {
            stream >> state.outputNodeId;
            continue;
        }

        if(tag == "node")
        {
            HostSessionNodeState node;
            if(stream >> node.nodeId >> node.appId >> node.randomSeed)
            {
                state.nodes.push_back(node);
            }
            continue;
        }

        if(tag == "param")
        {
            std::string id;
            float       value = 0.0f;
            if(stream >> id >> value)
            {
                state.parameterValues[id] = value;
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

        if(tag == "route")
        {
            HostSessionRoute route;
            if(stream >> route.sourcePortId >> route.destPortId)
            {
                state.routes.push_back(route);
            }
            continue;
        }

        if(tag == "seed")
        {
            std::uint32_t value = 0;
            if(stream >> value)
            {
                state.randomSeed = value;
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

    if(!state.nodes.empty())
    {
        const auto primaryIt
            = std::find_if(state.nodes.begin(),
                           state.nodes.end(),
                           [](const HostSessionNodeState& node) {
                               return node.nodeId == "node0";
                           });
        const auto& primary = primaryIt != state.nodes.end() ? *primaryIt : state.nodes.front();
        state.appId         = primary.appId;
        state.randomSeed    = primary.randomSeed;
    }
    else
    {
        EnsurePrimaryNodeMetadata(&state);
    }

    EnsureRackDefaults(&state);

    return state;
}
} // namespace daisyhost
