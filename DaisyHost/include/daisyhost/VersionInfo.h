#pragma once

#include <string>
#include <vector>

namespace daisyhost
{
struct VersionInfo
{
    std::string              version;
    std::string              buildIdentity;
    std::vector<std::string> releaseHighlights;
};

const VersionInfo& GetVersionInfo();
} // namespace daisyhost
