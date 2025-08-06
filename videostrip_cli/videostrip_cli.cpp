/**
 * @file videostrip_cli.cpp
 * @brief Command-line entry for videostrip frame extraction pipeline.
 */

#include <iostream>
#include <filesystem>
#include <sstream>
#include <exception>
#include <iomanip>
#include "videostrip_core.hpp"
// #include "logger.hpp"
#include <args.hxx>  // adjust as needed if in external/

namespace fs = std::filesystem;
using namespace videostrip;

int main(int argc, char* argv[])
{
    // --- Argument parsing ---
    args::ArgumentParser parser("videostrip - Video frame extractor for mapping pipelines", "");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<std::string> input_video(parser, "video", "Input video file", {'i', "input"});
    args::ValueFlag<std::string> out_dir(parser, "dir", "Output base directory", {'o', "output"});
    args::ValueFlag<std::string> feature_type(parser, "feature", "Feature type (SIFT/ORB/KAZE/SURF)", {'f', "feature"});
    args::ValueFlag<std::string> image_format(parser, "fmt", "Output image format (png, jpg, ...)", {'t', "format"});
    args::ValueFlag<float> overlap(parser, "overlap", "Overlap/confidence threshold [0.0–1.0]", {'c', "conf"});
    args::ValueFlag<int> max_skip(parser, "max-skip", "Max consecutive images to skip", {'s', "skip"});
    args::Flag enhance(parser, "enhance", "Apply image enhancement", {'e', "enhance"});
    args::ValueFlag<std::string> log_file(parser, "log", "Log file path", {'l', "log"});

    try {
        parser.ParseCLI(argc, argv);
    } catch (const args::Help&) {
        std::cout << parser;
        return 0;
    } catch (const args::ParseError& e) {
        std::cerr << e.what() << std::endl << parser;
        return 1;
    } catch (const args::ValidationError& e) {
        std::cerr << e.what() << std::endl << parser;
        return 2;
    }

    // --- Config construction ---
    ExtractorConfig config;
    if (input_video) config.input_video_path = args::get(input_video);
    else {
        std::cerr << "Error: --input required\n";
        return 1;
    }

    // Output dir base (default: ./output)
    std::string base_out = out_dir ? args::get(out_dir) : "output";
    config.output_images_dir = fs::path(base_out) / "images";
    config.output_features_dir = fs::path(base_out) / "features";
    config.output_metadata_csv = fs::path(base_out) / "frames.csv";
    config.output_summary_yaml = fs::path(base_out) / "summary.yaml";
    config.output_log_file = log_file ? args::get(log_file) : std::string((fs::path(base_out) / "run.log"));

    if (feature_type) config.feature_type = args::get(feature_type);
    if (image_format) config.image_format = args::get(image_format);
    if (overlap) config.overlap_threshold = args::get(overlap);
    if (max_skip) config.max_skipped_frames = args::get(max_skip);
    config.apply_enhancement = enhance ? true : false;

    config.create_output_dirs = true;
    config.enable_logging = true;

    // --- Logger setup ---
    auto logger = std::make_shared<ConsoleLogger>("videostrip_cli");

    try {
        logger->info("Starting videostrip run");

        // --- Extraction object ---
        VideoFrameExtractor extractor(config);
        extractor.setLogger(logger);

        // --- Progress callback ---
        extractor.setProgressCallback(
            [](size_t idx, size_t total, float progress, const std::string& msg) {
                std::cout << "\r["
                          << std::setw(3) << int(progress * 100.0f) << "%] "
                          << msg << " (frame " << idx << "/" << total << ")   " << std::flush;
            });

        // --- Main extraction ---
        bool ok = extractor.run();
        std::cout << std::endl;
        if (!ok) {
            logger->error("Extraction failed (non-fatal). Check logs for details.");
            return 3;
        }

        // --- Final reporting ---
        const auto& meta = extractor.getExtractedMetadata();
        const auto& summary = extractor.getRunSummary();
        logger->info("Extracted " + std::to_string(meta.size()) + " frames");
        logger->info("Metadata written to: " + config.output_metadata_csv);
        logger->info("Summary written to: " + config.output_summary_yaml);

    } catch (const std::exception& ex) {
        logger->error(std::string("Fatal error: ") + ex.what());
        return 10;
    }

    return 0;
}
