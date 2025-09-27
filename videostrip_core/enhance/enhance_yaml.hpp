#pragma once
#include <optional>
#include <string>
#include <videostrip_core/enhance/image_enhancers.hpp>
#include <yaml-cpp/yaml.h>

namespace videostrip
{

struct EnhanceConfig
{
    bool enable{false};
    std::vector<EnhanceStep> sequence;
};

/// Parse EnhanceConfig from a YAML node (root or subnode). Returns error string on failure.
std::optional<EnhanceConfig> parseEnhanceConfig(const YAML::Node& root, std::string& err);

} // namespace videostrip
