#include <algorithm>
#include <videostrip_core/enhance_yaml.hpp>

namespace videostrip_core {

static inline std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

std::optional<EnhanceConfig> parseEnhanceConfig(const YAML::Node& root, std::string& err) {
    EnhanceConfig cfg{};
    const auto n = root["enhance"];
    if (!n) {
        // Not present → disabled
        return cfg;
    }

    if (auto e = n["enable"]) cfg.enable = e.as<bool>(false);
    if (!cfg.enable) return cfg;

    const auto seq = n["sequence"];
    if (!seq || !seq.IsSequence()) {
        err = "enhance.sequence missing or not a sequence";
        return std::nullopt;
    }

    for (const auto& item : seq) {
        if (!item || !item.IsMap()) { err = "enhance.sequence entry must be a map"; return std::nullopt; }
        auto tnode = item["type"];
        if (!tnode) { err = "enhance.sequence entry missing 'type'"; return std::nullopt; }

        const std::string t = toLower(tnode.as<std::string>());
        if (t == "contrast" || t == "contrastoffset") {
            ContrastOffsetParams p{};
            if (auto a = item["alpha"]) p.alpha = a.as<double>(1.0);
            if (auto b = item["beta"])  p.beta  = b.as<double>(0.0);
            cfg.sequence.push_back({EnhanceType::ContrastOffset, p});
        }
        else if (t == "grayworld" || t == "gray" || t == "gw") {
            cfg.sequence.push_back({EnhanceType::GrayWorldWB, GrayWorldParams{}});
        }
        else if (t == "gamma") {
            GammaParams p{};
            // accept "value" or single scalar
            if (item["value"]) p.gamma = item["value"].as<double>(1.0);
            else if (item.size() == 2 && item.begin()->first.as<std::string>() == "type") {
                // nothing else
            }
            cfg.sequence.push_back({EnhanceType::Gamma, p});
        }
        else if (t == "clahe") {
            ClaheParams p{};
            if (auto c = item["clip_limit"]) p.clipLimit = c.as<double>(2.0);
            if (auto g = item["tile_grid"]) {
                if (g.IsSequence() && g.size() == 2) {
                    p.tileGrid.width  = g[0].as<int>(8);
                    p.tileGrid.height = g[1].as<int>(8);
                }
            }
            if (auto s = item["space"]) {
                auto ls = toLower(s.as<std::string>());
                if (ls == "ycrcb" || ls == "ycbcr") p.space = ClaheSpace::YCrCb;
                else if (ls == "hsv") p.space = ClaheSpace::HSV;
                else if (ls == "lab") p.space = ClaheSpace::Lab;
                else if (ls == "bgr") p.space = ClaheSpace::BGR;
            }
            cfg.sequence.push_back({EnhanceType::CLAHE, p});
        }
        else {
            err = "Unknown enhance.type: " + t;
            return std::nullopt;
        }
    }

    return cfg;
}

} // namespace videostrip_core
