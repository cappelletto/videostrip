// config_loader.hpp
#pragma once
/**
 * @file config_loader.hpp
 * @brief YAML loader & merge helpers for videostrip CLI.
 */

#include <string>
#include <optional>

#include <videostrip_core/videostrip_core.hpp>  // for ExtractorConfig

namespace videostrip::cli {

/**
 * @brief Load a YAML config file (optional), return a partially-filled ExtractorConfig.
 *        Paths under "output" are NOT normalized against base_dir here.
 * @param yaml_path   Path to YAML file
 * @param out         Filled on success (partial config; only keys present are set)
 * @param err         Error text on failure (file not found / parse error)
 * @return true if loaded successfully; false otherwise (out is undefined on failure)
 */
bool load_yaml_config(const std::string& yaml_path,
                      ExtractorConfig& out,
                      std::string& err);

/**
 * @brief Apply CLI-style normalization for output paths using base_dir.
 *        If a path is relative, it is joined to base_dir.
 *        No-op for empty paths.
 */
void normalize_output_paths(ExtractorConfig& cfg, const std::string& base_dir);

/**
 * @brief Merge precedence helper: dst = YAML ; then apply CLI overrides.
 *        Only overwrites fields when src has non-empty values / set flags.
 *        (Still need to clamp/validate after merging.)
 */
void merge_yaml_into(ExtractorConfig& dst, const ExtractorConfig& yamlCfg);

} // namespace videostrip::cli
