#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <opencv2/opencv.hpp>

#include <videostrip_core/videostrip_core.hpp>                // the public API
#include <videostrip_core/feature/feature_extractor.hpp>      // used indirectly
#include <videostrip_core/logging/logger.hpp>                 // ConsoleLogger

namespace fs = std::filesystem;
using namespace videostrip;

#if defined(_WIN32)
  #include <process.h>
  static inline long my_pid() { return _getpid(); }
#else
  #include <unistd.h>
  static inline long my_pid() { return getpid(); }
#endif

static fs::path make_tmp_dir(const std::string& name) {
    auto base = fs::temp_directory_path() / ("vs_" + name + "_" + std::to_string(my_pid()));
    fs::create_directories(base);
    return base;
}


// static fs::path make_tmp_dir(const std::string& name) {
//     auto base = fs::temp_directory_path() / ("vs_" + name + "_" + std::to_string(::getpid()));
//     fs::create_directories(base);
//     return base;
// }

TEST_CASE("core: synthetic video to images + CSV + YAML", "[core][smoke]") {
    // --- 1) Make synthetic video (20 frames @ 10 fps) ---
    const int W=320, H=240, FPS=10, N=20;
    auto tmp = make_tmp_dir("smoke");
    auto videoPath = (tmp / "synthetic.avi").string();
    auto fourcc = cv::VideoWriter::fourcc('M','J','P','G'); // widely available on CI
    cv::VideoWriter writer(videoPath, fourcc, FPS, cv::Size(W,H));
    REQUIRE(writer.isOpened());

    for (int i=0; i<N; ++i) {
        cv::Mat frame(H, W, CV_8UC3, cv::Scalar(10*i % 255, 50, 200));
        cv::putText(frame, std::to_string(i), {20,120}, cv::FONT_HERSHEY_SIMPLEX, 2.0, {255,255,255}, 2);
        writer.write(frame);
    }
    writer.release();

    // --- 2) Configure extractor to pick ~every 5th frame ---
    ExtractorConfig cfg;
    cfg.input_video_path    = videoPath;
    cfg.output_images_dir   = (tmp / "images").string();
    cfg.output_features_dir = (tmp / "features").string();
    cfg.output_metadata_csv = (tmp / "frames.csv").string();
    cfg.output_summary_yaml = (tmp / "summary.yaml").string();
    cfg.output_log_file     = (tmp / "run.log").string();

    cfg.feature_type = "ORB";
    cfg.image_format = "png";
    cfg.apply_enhancement = false;
    cfg.create_output_dirs = true;
    cfg.enable_logging = true;
    cfg.max_skipped_frames = 4;     // crude stride: extract roughly every 5th

    // --- 3) Run extraction ---
    VideoFrameExtractor extractor(cfg);
    extractor.setLogger(std::make_shared<videostrip::logger::ConsoleLogger>("test"));

    size_t lastIdx = 0, total = 0;
    extractor.setProgressCallback([&](size_t idx, size_t tot, float, const std::string&) {
        lastIdx = idx; total = tot;
    });

    REQUIRE_NOTHROW(extractor.run());
    const auto& rows = extractor.getExtractedMetadata();
    const auto& summary = extractor.getRunSummary();

    // --- 4) Validate outputs exist & are plausible ---
    // Expect ~4 frames extracted (20 / 5 stride)
    REQUIRE(rows.size() >= 3);
    REQUIRE(rows.size() <= 5);

    // Each image file should exist, feature_count/quality_score >= 0
    for (const auto& r : rows) {
        REQUIRE(r.frame_idx >= 0);
        REQUIRE(r.timestamp_ms >= 0);
        REQUIRE(r.feature_count >= 0);
        REQUIRE(r.quality_score >= 0.0);
        fs::path img = fs::path(cfg.output_images_dir) / r.output_image_name;
        REQUIRE(fs::exists(img));
    }

    // CSV and YAML should exist
    REQUIRE(fs::exists(cfg.output_metadata_csv));
    REQUIRE(fs::exists(cfg.output_summary_yaml));

    // Progress invoked
    REQUIRE(total >= static_cast<size_t>(N));
}
