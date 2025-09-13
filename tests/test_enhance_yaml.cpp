#include <catch2/catch_test_macros.hpp>
#include <yaml-cpp/yaml.h>
#include <string>

#include <videostrip_core/enhance/enhance_yaml.hpp>

using namespace videostrip;

TEST_CASE("enhance yaml: valid sequence parses", "[enhance][yaml]") {
    const char* y = R"YAML(
enhance:
  enable: true
  sequence:
    - type: contrast
      alpha: 1.1
      beta: -5
    - type: grayworld
    - type: gamma
      value: 1.05
    - type: clahe
      clip_limit: 2.0
      tile_grid: [8, 8]
      space: YCrCb
)YAML";
    YAML::Node root = YAML::Load(y);
    std::string err;
    auto cfg = parseEnhanceConfig(root, err);
    REQUIRE(cfg.has_value());
    CHECK(cfg->enable == true);
    REQUIRE(cfg->sequence.size() == 4);
    CHECK(cfg->sequence[0].type == EnhanceType::ContrastOffset);
    CHECK(cfg->sequence[1].type == EnhanceType::GrayWorldWB);
    CHECK(cfg->sequence[2].type == EnhanceType::Gamma);
    CHECK(cfg->sequence[3].type == EnhanceType::CLAHE);
}

TEST_CASE("enhance yaml: malformed op fails", "[enhance][yaml]") {
    const char* y = R"YAML(
enhance:
  enable: true
  sequence:
    - type: i_do_not_exist
)YAML";
    YAML::Node root = YAML::Load(y);
    std::string err;
    auto cfg = parseEnhanceConfig(root, err);
    CHECK_FALSE(cfg.has_value());
    CHECK(err.find("Unknown") != std::string::npos);
}
