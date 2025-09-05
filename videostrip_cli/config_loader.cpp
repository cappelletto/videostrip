// config_loader.cpp
#include "config_loader.hpp"

#include <filesystem>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

namespace videostrip::cli {

static inline bool is_nonempty(const std::string& s) { return !s.empty(); }

bool load_yaml_config(const std::string& yaml_path,
                      ExtractorConfig& out,
                      std::string& err)
{
    try {
        if (!fs::exists(yaml_path)) {
            err = "Config file does not exist: " + yaml_path;
            return false;
        }

        YAML::Node root = YAML::LoadFile(yaml_path);

        // version (optional)
        if (auto v = root["version"]; v && v.IsScalar()) {
            // reserved for schema evolution; ignore for now
            (void)v.as<int>();
        }

        // input.video
        if (auto n = root["input"]; n && n.IsMap()) {
            if (auto v = n["video"]; v && v.IsScalar()) {
                out.input_video_path = v.as<std::string>();
            }
        }

        // output.*
        std::string base_dir = "output";
        if (auto n = root["output"]; n && n.IsMap()) {
            if (auto v = n["base_dir"]; v && v.IsScalar()) {
                base_dir = v.as<std::string>();
            }
            if (auto v = n["images_dir"]; v && v.IsScalar()) {
                out.output_images_dir = v.as<std::string>();
            }
            if (auto v = n["features_dir"]; v && v.IsScalar()) {
                out.output_features_dir = v.as<std::string>();
            }
            if (auto v = n["csv"]; v && v.IsScalar()) {
                out.output_metadata_csv = v.as<std::string>();
            }
            if (auto v = n["yaml"]; v && v.IsScalar()) {
                out.output_summary_yaml = v.as<std::string>();
            }
            if (auto v = n["log"]; v && v.IsScalar()) {
                out.output_log_file = v.as<std::string>();
            }
        }

        // processing.*
        if (auto n = root["processing"]; n && n.IsMap()) {
            if (auto v = n["feature_type"]; v && v.IsScalar()) {
                out.feature_type = v.as<std::string>();
            }
            if (auto v = n["image_format"]; v && v.IsScalar()) {
                out.image_format = v.as<std::string>();
            }
            if (auto v = n["overlap_threshold"]; v && v.IsScalar()) {
                out.overlap_threshold = v.as<double>();
            }
            if (auto v = n["max_skipped_frames"]; v && v.IsScalar()) {
                out.max_skipped_frames = v.as<int>();
            }
            if (auto v = n["apply_enhancement"]; v && v.IsScalar()) {
                out.apply_enhancement = v.as<bool>();
            }
            if (auto v = n["create_output_dirs"]; v && v.IsScalar()) {
                out.create_output_dirs = v.as<bool>();
            }
            if (auto v = n["enable_logging"]; v && v.IsScalar()) {
                out.enable_logging = v.as<bool>();
            }
            // Optional, harmless now: overlap_mode (string)
            if (auto v = n["overlap_mode"]; v && v.IsScalar()) {
                // Add this field to ExtractorConfig when you wire modes in core
                // e.g., out.overlap_mode = v.as<std::string>();
                // Ignored if not present in struct.
            }
        }

        // Normalize outputs (relative -> base_dir)
        // Determine base_dir: prefer output.base_dir, else inferred from any set path, else "output"
        std::string chosen_base = "output";
        if (root["output"] && root["output"]["base_dir"] && root["output"]["base_dir"].IsScalar()) {
            chosen_base = root["output"]["base_dir"].as<std::string>();
        }
        normalize_output_paths(out, chosen_base);

        err.clear();
        return true;
    }
    catch (const std::exception& e) {
        err = std::string("Failed to parse YAML: ") + e.what();
        return false;
    }
}

void normalize_output_paths(ExtractorConfig& cfg, const std::string& base_dir)
{
    auto norm = [&](std::string& p) {
        if (p.empty()) return;
        fs::path pp(p);
        if (pp.is_relative()) {
            p = (fs::path(base_dir) / pp).string();
        } // else absolute; leave as-is
    };

    // If nothing set at all, build a default layout
    if (!is_nonempty(cfg.output_images_dir) &&
        !is_nonempty(cfg.output_features_dir) &&
        !is_nonempty(cfg.output_metadata_csv) &&
        !is_nonempty(cfg.output_summary_yaml) &&
        !is_nonempty(cfg.output_log_file))
    {
        cfg.output_images_dir   = (fs::path(base_dir) / "images").string();
        cfg.output_features_dir = (fs::path(base_dir) / "features").string();
        cfg.output_metadata_csv = (fs::path(base_dir) / "frames.csv").string();
        cfg.output_summary_yaml = (fs::path(base_dir) / "summary.yaml").string();
        cfg.output_log_file     = (fs::path(base_dir) / "run.log").string();
        return;
    }

    // Otherwise normalize individually
    norm(cfg.output_images_dir);
    norm(cfg.output_features_dir);
    norm(cfg.output_metadata_csv);
    norm(cfg.output_summary_yaml);
    norm(cfg.output_log_file);
}

void merge_yaml_into(ExtractorConfig& dst, const ExtractorConfig& y)
{
    auto set_if = [](std::string& dstf, const std::string& src) {
        if (!src.empty()) dstf = src;
    };

    set_if(dst.input_video_path, y.input_video_path);

    set_if(dst.output_images_dir,   y.output_images_dir);
    set_if(dst.output_features_dir, y.output_features_dir);
    set_if(dst.output_metadata_csv, y.output_metadata_csv);
    set_if(dst.output_summary_yaml, y.output_summary_yaml);
    set_if(dst.output_log_file,     y.output_log_file);

    if (!y.feature_type.empty()) dst.feature_type = y.feature_type;
    if (!y.image_format.empty()) dst.image_format = y.image_format;

    // Numeric/bool fields: copy directly (assume src had meaningful values when set)
    // We can’t distinguish “unset” from “default” here; rely on loader side to only set when key exists.
    dst.overlap_threshold  = y.overlap_threshold;
    dst.max_skipped_frames = y.max_skipped_frames;
    dst.apply_enhancement  = y.apply_enhancement;
    dst.create_output_dirs = y.create_output_dirs;
    dst.enable_logging     = y.enable_logging;

    // Optional future field:
    // if (!y.overlap_mode.empty()) dst.overlap_mode = y.overlap_mode;
}

} // namespace videostrip::cli
