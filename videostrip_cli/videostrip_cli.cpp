/**
 * @file videostrip_cli.cpp
 * @brief Command-line entry for videostrip frame extraction pipeline.
 */

#include <algorithm>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <videostrip_cli/config_loader.hpp>
#include <videostrip_core/logging/logger.hpp>
#include <videostrip_core/videostrip_core.hpp>

// TODO: Replace with proper include when args is added to third_party
#include <../third_party/args.hxx> // provided from third_pary/ via include path

// --- Added for --version / --print-schema ---
#include <opencv2/core.hpp>

#include <videostrip_core/schema.hpp>

#include "version.hpp"

namespace fs = std::filesystem;
using namespace videostrip;

static inline std::string to_upper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
    return s;
}

// --- helpers for reporting-only actions ---
static int PrintVersion(const std::string& fmt)
{
    const std::string ocv = cv::getVersionString();
    if (fmt == "json")
    {
        std::cout << R"({JSON fmt string goes here})" << std::endl;
        std::cout << "{"
                  << "\"name\":\"videostrip_cli\","
                  << "\"version\":\"" << VIDEOSTRIP_VERSION << "\","
                  << "\"schema_version\":\"" << videostrip::kSchemaVersion << "\","
                  << "\"git\":{\"commit\":\"" << VIDEOSTRIP_GIT_COMMIT << "\",\"dirty\":false},"
                  << "\"build\":{\"type\":\"" << VIDEOSTRIP_BUILD_TYPE
                  << "\","
                     "\"compiler\":\""
                  << VIDEOSTRIP_COMPILER_ID
                  << "\","
                     "\"compiler_version\":\""
                  << VIDEOSTRIP_COMPILER_VERSION
                  << "\","
                     "\"platform\":\""
                  << VIDEOSTRIP_PLATFORM << "\"},"
                  << "\"opencv\":{\"version\":\"" << ocv << "\",\"vendored\":true}"
                  << "}";
    }
    else
    {
        // std::cout << R"({VERSION string goes here})" << std::endl;
        std::cout << "videostrip_cli " << VIDEOSTRIP_VERSION << " (schema v"
                  << videostrip::kSchemaVersion << ")\n"
                  << "git: " << VIDEOSTRIP_GIT_COMMIT << " (dirty: no)\n"
                  << "build: " << VIDEOSTRIP_BUILD_TYPE << ", " << VIDEOSTRIP_COMPILER_ID << " "
                  << VIDEOSTRIP_COMPILER_VERSION << ", " << VIDEOSTRIP_PLATFORM << "\n"
                  << "opencv: " << ocv << " (vendored: yes)" << std::endl;
    }
    return 0;
}

static int PrintSchema(const std::string& which, const std::string& format, bool /*brief*/)
{
    const std::string WHICH = to_upper(which);
    const std::string FMT = to_upper(format);

    if (WHICH == "SUMMARY")
    {
        if (FMT == "YAML")
        {
            std::cout << videostrip::SummarySchemaYaml();
            return 0;
        }
        std::cerr << "JSON summary schema not implemented; use --schema-format=yaml";
        return 2;
    }
    else if (WHICH == "FRAMES")
    {
        if (FMT == "JSON")
        {
            std::cout << videostrip::FramesCsvSchemaJson();
            return 0;
        }
        std::cout << "schema_version: \"" << videostrip::kSchemaVersion << "\"\n"
                  << "frames_csv:\n"
                  << "  header: [frame_idx, timestamp_ms, output_image, feature_count, "
                     "quality_score, georef]"
                  << std::endl;
        return 0;
    }
    else
    { // ALL
        if (FMT == "YAML")
        {
            std::cout << videostrip::SummarySchemaYaml() << "\n---\n\n"
                      << "schema_version: \"" << videostrip::kSchemaVersion << "\"\n"
                      << "frames_csv:\n"
                      << "  header: [frame_idx, timestamp_ms, output_image, feature_count, "
                         "quality_score, georef]"
                      << std::endl;
            return 0;
        }
        std::cout << "{\"summary\": null,\"frames\": " << videostrip::FramesCsvSchemaJson() << "}";
        return 0;
    }
}

int main(int argc, char* argv[])
{
    // --- Argument parsing ---
    args::ArgumentParser parser("videostrip - Video frame extractor for mapping pipelines", "");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<std::string> input_video(parser, "video", "Input video file", {'i', "input"});
    args::ValueFlag<std::string> out_dir(parser, "dir", "Output base directory", {'o', "output"});
    args::ValueFlag<std::string> feature_type(
        parser, "feature", "Feature type (SIFT/ORB/AKAZE/SURF)", {'f', "feature"});
    args::ValueFlag<std::string> image_format(parser, "fmt", "Output image format (png, jpg, ...)",
                                              {'t', "format"});
    args::ValueFlag<float> overlap(parser, "overlap", "Overlap/confidence threshold [0.0–1.0]",
                                   {'c', "conf"});
    args::ValueFlag<int> max_skip(parser, "max-skip", "Max consecutive images to skip",
                                  {'s', "skip"});
    args::Flag enhance(parser, "enhance", "Apply image enhancement", {'e', "enhance"});
    args::ValueFlag<std::string> log_file(parser, "log", "Log file path", {'l', "log"});
    args::ValueFlag<std::string> config_file(parser, "file", "YAML config file", {"config"});
    args::ValueFlag<std::string> overlap_mode(parser, "mode", "Overlap mode (FEATURE|FLOW|ECC)",
                                              {"overlap-mode"});

    // --- Version / Schema reporting ---
    args::Flag version(parser, "version", "Print version and exit", {'V', "version"});
    args::ValueFlag<std::string> version_format(parser, "fmt", "Version format: text|json",
                                                {"version-format"});

    args::Flag print_schema(parser, "print-schema", "Print output schema and exit",
                            {"print-schema"});
    args::ValueFlag<std::string> schema_which(parser, "which", "Which schema: summary|frames|all",
                                              {"schema-which"});
    args::ValueFlag<std::string> schema_format(parser, "format", "Schema format: yaml|json",
                                               {"schema-format"});
    args::Flag schema_brief(parser, "brief", "Brief schema info", {"brief"});

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Help&)
    {
        std::cout << parser;
        return 0;
    }
    catch (const args::ParseError& e)
    {
        std::cerr << e.what() << "\n" << parser;
        return 1;
    }
    catch (const args::ValidationError& e)
    {
        std::cerr << e.what() << "\n" << parser;
        return 2;
    }

    // Handle reporting-only actions and exit cleanly
    if (version)
    {
        const std::string fmt = version_format ? args::get(version_format) : std::string("text");
        return PrintVersion(to_upper(fmt) == "JSON" ? std::string("json") : std::string("text"));
    }
    if (print_schema)
    {
        const std::string which = schema_which ? args::get(schema_which) : std::string("all");
        const std::string format = schema_format ? args::get(schema_format) : std::string("yaml");
        const bool brief = static_cast<bool>(schema_brief);
        return PrintSchema(which, format, brief);
    }

    // --- Config construction (YAML optional + CLI overrides) ---
    ExtractorConfig config; // compiled defaults

    // Load YAML first (optional)
    if (config_file)
    {
        std::string err;
        ExtractorConfig y;
        // Show in console we are loading a config
        std::cout << "Loading config from " << args::get(config_file) << "\n";
        if (!videostrip::cli::load_yaml_config(args::get(config_file), y, err))
        {
            std::cerr << "Config error: " << err << "\n";
            return 2;
        }
        // TODO: Reevaluate either setting flags as optional (otherwise we can't tell if user set
        // them or end using defaults) Or just avoid merging and always let CLI override YAML if
        // present
        videostrip::cli::merge_yaml_into(
            config, y); // Ensure 'config' is passed by reference in the function definition
    }

    // Input (CLI > YAML)
    if (input_video)
        config.input_video_path = args::get(input_video);
    if (config.input_video_path.empty())
    {
        std::cerr << "Error: --input (or input.video in YAML) is required\n";
        return 1;
    }

    // ===== Preserve explicit base_out logic =====
    fs::path base_out;
    if (out_dir)
    {
        // CLI overrides everything for outputs
        base_out = fs::path(args::get(out_dir));
        config.output_images_dir = (base_out / "images").string();
        config.output_features_dir = (base_out / "features").string();
        config.output_metadata_csv = (base_out / "frames.csv").string();
        config.output_summary_yaml = (base_out / "summary.yaml").string();
        // log can still be overridden by --log below
        if (log_file)
        {
            config.output_log_file = args::get(log_file);
        }
        else
        {
            config.output_log_file = (base_out / "run.log").string();
        }
    }
    else
    {
        // No --output: use YAML outputs if provided; otherwise default to ./output/*
        base_out = fs::path("output");

        const bool yaml_set_any =
            !config.output_images_dir.empty() || !config.output_features_dir.empty() ||
            !config.output_metadata_csv.empty() || !config.output_summary_yaml.empty() ||
            !config.output_log_file.empty();

        if (!yaml_set_any)
        {
            // Build canonical layout under ./output (original behavior)
            config.output_images_dir = (base_out / "images").string();
            config.output_features_dir = (base_out / "features").string();
            config.output_metadata_csv = (base_out / "frames.csv").string();
            config.output_summary_yaml = (base_out / "summary.yaml").string();
            config.output_log_file = (base_out / "run.log").string();
        }
        else
        {
            // YAML provided paths: if any are relative, normalize under YAML base (handled by
            // loader) Honor --log override if present
            if (log_file)
                config.output_log_file = args::get(log_file);
        }
    }
    // ============================================

    // Feature type: normalize & validate (fallback to ORB)
    if (feature_type)
        config.feature_type = to_upper(args::get(feature_type));
    {
        const std::string& ft = config.feature_type;
        const bool ok = (ft == "ORB" || ft == "AKAZE" || ft == "KAZE" || ft == "SURF" ||
                         ft == "GRID_ORB" || ft == "GRID_AKAZE");
        if (!ok)
        {
            std::cerr << "Warning: unsupported feature type '" << ft
                      << "' -> falling back to ORB\n";
            config.feature_type = "ORB";
        }
    }

    // Image format
    if (image_format)
        config.image_format = args::get(image_format);

    // Overlap/conf threshold
    if (overlap)
    {
        const float v = args::get(overlap);
        if (v < 0.0f || v > 1.0f)
            std::cerr << "Warning: --conf out of range [0,1]; clamping\n";
        config.overlap_threshold = std::clamp(v, 0.0f, 1.0f);
    }

    // Max skipped frames
    if (max_skip)
    {
        const int ms = args::get(max_skip);
        config.max_skipped_frames = ms < 0 ? 0 : ms;
    }

    // Enhancement
    if (enhance)
    {
        // CLI flag always enables enhancement
        config.apply_enhancement = true;

        // If no enhancement sequence has been configured (no YAML or no enhance block),
        // install the default pipeline used by the YAML loader’s legacy path.
        if (!config.enhance.enable || config.enhance.sequence.empty())
        {
            config.enhance.enable = true;
            config.enhance.sequence.clear();
            config.enhance.sequence.push_back({EnhanceType::GrayWorldWB, GrayWorldParams{}});
            config.enhance.sequence.push_back(
                {EnhanceType::CLAHE, ClaheParams{2.0, cv::Size(8, 8), ClaheSpace::YCrCb}});
        }
    }

    // Optional: overlap-mode passthrough (harmless if core ignores it now)
    // if (overlap_mode) config.overlap_mode = to_upper(args::get(overlap_mode));

    // Always allow creating dirs and logging by default (YAML can override earlier)
    config.create_output_dirs = true;
    config.enable_logging = true;

    // Final validation
    if (!fs::exists(config.input_video_path))
    {
        std::cerr << "Error: input video does not exist: " << config.input_video_path << "\n";
        return 1;
    }

    // --- Logger setup ---
    auto logger = std::make_shared<videostrip::logger::ConsoleLogger>("videostrip_cli");

    // --- Print a concise run summary ---
    {
        std::ostringstream ss;
        ss << "Input: " << config.input_video_path << " | Out: " << base_out
           << " | Feature: " << config.feature_type << " | Format: " << config.image_format
           << " | Conf: " << config.overlap_threshold << " | Skip: " << config.max_skipped_frames
           << " | Enhance: " << (config.apply_enhancement ? "on" : "off");
        logger->info(ss.str());
    }

    try
    {
        // Extraction object
        VideoFrameExtractor extractor(config);
        extractor.setLogger(logger);

        // Progress callback
        extractor.setProgressCallback(
            [](size_t idx, size_t total, float progress, const std::string& msg)
            {
                std::cout << "\r[" << std::setw(3) << static_cast<int>(progress * 100.0f) << "%] "
                          << msg << " (frame " << idx << "/" << total << ")   " << std::flush;
            });

        // Run
        const bool ok =
            extractor
                .run(); ///< this will run main videostrip_core loop: VideoFrameExtractor::run()
        std::cout << "\n";
        if (!ok)
        {
            logger->error("Extraction failed (non-fatal). Check logs for details.");
            return 3;
        }

        // Final reporting
        const auto& meta = extractor.getExtractedMetadata();
        const auto& summary = extractor.getRunSummary();
        logger->info("Extracted " + std::to_string(meta.size()) + " frames");
        logger->info("CSV: " + config.output_metadata_csv);
        logger->info("YAML: " + config.output_summary_yaml);
        (void)summary; // summary already reflected by paths above
    }
    catch (const std::exception& ex)
    {
        logger->error(std::string("Fatal error: ") + ex.what());
        return 10;
    }

    return 0;
}
