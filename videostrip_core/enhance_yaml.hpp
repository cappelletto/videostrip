#pragma once
#include <yaml-cpp/yaml.h>
#include <optional>
#include <string>

#include <videostrip_core/image_enhancers.hpp>

namespace videostrip_core {

struct EnhanceConfig {
    bool enable{false};
    std::vector<EnhanceStep> sequence;
};

/// Parse EnhanceConfig from a YAML node (root or subnode). Returns error string on failure.
std::optional<EnhanceConfig> parseEnhanceConfig(const YAML::Node& root, std::string& err);

} // namespace videostrip_core
