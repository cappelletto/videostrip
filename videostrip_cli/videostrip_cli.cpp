/**
 * @file videostrip_cli.cpp
 * @brief Command-line entry for videostrip frame extraction pipeline.
 */

#include <iostream>
#include <filesystem>
#include <sstream>
#include <exception>
#include <iomanip>
#include <algorithm>

#include <videostrip_core/videostrip_core.hpp>
#include <videostrip_core/logging/logger.hpp>

#include <args.hxx>  // provided from external/ via include path

namespace fs = std::filesystem;
using namespace videostrip;

static inline std::string to_upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::toupper(c); });
    return s;
}

int main(int argc, char *argv[])
{
    // --- Argument parsing ---
    args::ArgumentParser parser("videostrip - Video frame extractor for mapping pipelines", "");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<std::string> input_video(parser, "video", "Input video file", {'i', "input"});
    args::ValueFlag<std::string> out_dir(parser, "dir", "Output base directory", {'o', "output"});
    args::ValueFlag<std::string> feature_type(parser, "feature", "Feature type (SIFT/ORB/AKAZE/SURF)", {'f', "feature"});
    args::ValueFlag<std::string> image_format(parser, "fmt", "Output image format (png, jpg, ...)", {'t', "format"});
    args::ValueFlag<float>       overlap(parser, "overlap", "Overlap/confidence threshold [0.0–1.0]", {'c', "conf"});
    args::ValueFlag<int>         max_skip(parser, "max-skip", "Max consecutive images to skip", {'s', "skip"});
    args::Flag                   enhance(parser, "enhance", "Apply image enhancement", {'e', "enhance"});
    args::ValueFlag<std::string> log_file(parser, "log", "Log file path", {'l', "log"});

    try {
        parser.ParseCLI(argc, argv);
    } catch (const args::Help&) {
        std::cout << parser;
        return 0;
    } catch (const args::ParseError& e) {
        std::cerr << e.what() << "\n" << parser;
        return 1;
    } catch (const args::ValidationError& e) {
        std::cerr << e.what() << "\n" << parser;
        return 2;
    }

    // --- Config construction ---
    ExtractorConfig config;

    if (!input_video) {
        std::cerr << "Error: --input is required\n";
        return 1;
    }
    config.input_video_path = args::get(input_video);

    // Basic validation: input file exists
    if (!fs::exists(config.input_video_path)) {
        std::cerr << "Error: input video does not exist: " << config.input_video_path << "\n";
        return 1;
    }

    // Output base directory (default: ./output)
    const fs::path base_out = out_dir ? fs::path(args::get(out_dir)) : fs::path("output");
    config.output_images_dir   = (base_out / "images").string();
    config.output_features_dir = (base_out / "features").string();
    config.output_metadata_csv = (base_out / "frames.csv").string();
    config.output_summary_yaml = (base_out / "summary.yaml").string();
    config.output_log_file     = log_file ? args::get(log_file) : (base_out / "run.log").string();

    // Feature type: normalize and fallback to ORB
    if (feature_type) {
        config.feature_type = to_upper(args::get(feature_type));
    }
    if (config.feature_type != "ORB" && config.feature_type != "SIFT"
        && config.feature_type != "AKAZE" && config.feature_type != "SURF") {
        std::cerr << "Warning: unsupported feature type '" << config.feature_type
                  << "' → falling back to ORB\n";
        config.feature_type = "ORB";
    }

    if (image_format) config.image_format = args::get(image_format);

    if (overlap) {
        const float v = args::get(overlap);
        if (v < 0.0f || v > 1.0f) {
            std::cerr << "Warning: --conf out of range [0,1]; clamping\n";
        }
        config.overlap_threshold = std::clamp(v, 0.0f, 1.0f);
    }

    if (max_skip) {
        const int ms = args::get(max_skip);
        if (ms < 0) {
            std::cerr << "Warning: --skip negative; setting to 0\n";
            config.max_skipped_frames = 0;
        } else {
            config.max_skipped_frames = ms;
        }
    }

    config.apply_enhancement = enhance ? true : false;
    config.create_output_dirs = true;
    config.enable_logging = true;

    // --- Logger setup ---
    auto logger = std::make_shared<videostrip::logger::ConsoleLogger>("videostrip_cli");

    // --- Print a concise run summary ---
    {
        std::ostringstream ss;
        ss << "Input: "  << config.input_video_path << " | Out: " << base_out
           << " | Feature: " << config.feature_type
           << " | Format: "  << config.image_format
           << " | Conf: "    << config.overlap_threshold
           << " | Skip: "    << config.max_skipped_frames
           << " | Enhance: " << (config.apply_enhancement ? "on" : "off");
        logger->info(ss.str());
    }

    try {
        // Extraction object
        VideoFrameExtractor extractor(config);
        extractor.setLogger(logger);

        // Progress callback
        extractor.setProgressCallback(
            [](size_t idx, size_t total, float progress, const std::string& msg) {
                std::cout << "\r["
                          << std::setw(3) << static_cast<int>(progress * 100.0f) << "%] "
                          << msg << " (frame " << idx << "/" << total << ")   " << std::flush;
            });

        // Run
        const bool ok = extractor.run();
        std::cout << std::endl;
        if (!ok) {
            logger->error("Extraction failed (non-fatal). Check logs for details.");
            return 3;
        }

        // Final reporting
        const auto& meta    = extractor.getExtractedMetadata();
        const auto& summary = extractor.getRunSummary();
        logger->info("Extracted " + std::to_string(meta.size()) + " frames");
        logger->info("CSV: " + config.output_metadata_csv);
        logger->info("YAML: " + config.output_summary_yaml);
        (void)summary; // summary already reflected by paths above
    }
    catch (const std::exception& ex) {
        logger->error(std::string("Fatal error: ") + ex.what());
        return 10;
    }

    return 0;
}
