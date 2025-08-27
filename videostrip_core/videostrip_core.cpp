/**
 * @file videostrip_core.cpp
 * @brief Implementation of VideoFrameExtractor for the videostrip pipeline.
 */

#include <videostrip_core/videostrip_core.hpp>
// #include <videostrip_core/feature/feature_extractor.hpp>    // already included by core.hpp
#include <videostrip_core/logging/logger.hpp>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

namespace videostrip
{

    // =======================
    // VideoFrameExtractor
    // =======================

    VideoFrameExtractor::VideoFrameExtractor(const ExtractorConfig &config)
        : m_config(config),
          m_feature_extractor(make_default_extractor(config.feature_type)),
          m_logger(std::make_shared<videostrip::logger::ConsoleLogger>("VideoFrameExtractor"))
    {
    }

    VideoFrameExtractor::~VideoFrameExtractor() = default;

    void VideoFrameExtractor::setProgressCallback(ProgressCallback cb)
    {
        m_progress_cb = cb;
    }

    void VideoFrameExtractor::setLogger(std::shared_ptr<Logger> logger)
    {
        m_logger = logger;
    }

    void VideoFrameExtractor::setFeatureExtractor(std::unique_ptr<FeatureExtractor> extractor)
    {
        m_feature_extractor = std::move(extractor);
    }

    const std::vector<FrameMetadata> &VideoFrameExtractor::getExtractedMetadata() const
    {
        return m_metadata;
    }

    const RunSummary &VideoFrameExtractor::getRunSummary() const
    {
        return m_summary;
    }

    bool VideoFrameExtractor::run()
    {
        // Prepare outputs and summary
        ensureOutputFolders();

        m_metadata.clear();
        m_summary.extracted_images.clear();
        m_summary.input_video_basename = fs::path(m_config.input_video_path).filename().string();
        m_summary.config_used = m_config;

        // ISO8601-ish timestamp
        {
            auto now = std::chrono::system_clock::now();
            std::time_t tt = std::chrono::system_clock::to_time_t(now);
            std::tm tm{};
            #if defined(_WIN32)
                localtime_s(&tm, &tt);
            #else
                localtime_r(&tt, &tm);
            #endif
            std::ostringstream ts;
            ts << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
            m_summary.run_datetime = ts.str();
        }

        // Open video
        cv::VideoCapture cap(m_config.input_video_path);
        if (!cap.isOpened()) {
            writeLog("Failed to open video: " + m_config.input_video_path, "ERROR");
            throw std::runtime_error("Cannot open video: " + m_config.input_video_path);
        }

        const int total_frames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
        int frame_idx = 0;
        int extracted  = 0;
        int skip_count = 0;

        cv::Mat frame;
        // For homography based selection, we need to keep previous keyframe and keypoints
        // This imposes some statefulness and sequential read; consider refactoring later
        while (cap.read(frame)) {
            // crude stride: export when we've skipped >= max_skipped_frames
            bool select = (skip_count >= m_config.max_skipped_frames);
            skip_count++;

            if (select) {
                // Build image name
                std::ostringstream img_name;
                img_name << "frame_" << std::setw(6) << std::setfill('0') << frame_idx << "." << m_config.image_format;
                const std::string image_name = img_name.str();
                const std::string image_path = (fs::path(m_config.output_images_dir) / image_name).string();

                // Optional enhancement (placeholder hook)
                if (m_config.apply_enhancement) {
                    // TODO: enhancement step
                    writeLog("Enhancement not implemented; skipping", "WARN");
                }

                // Write image
                if (!cv::imwrite(image_path, frame)) {
                    writeLog("Failed to write image: " + image_path, "WARN");
                } else {
                    // Feature extraction
                    const std::string feature_path = (fs::path(m_config.output_features_dir) / (image_name + ".feat.txt")).string();
                    double quality_score = 0.0;
                    int feature_count = 0;

                    // TODO: default feature extractor can be created once in ctor
                    // We do not expect to change it per-frame
                    try {
                        if (!m_feature_extractor) {
                            // fall back to default ORB if not set
                            m_feature_extractor = make_default_extractor(m_config.feature_type);
                        }
                        // TODO: use cv::Mat input and in-memory output later for faster processing
                        feature_count = m_feature_extractor->extract(image_path, const_cast<std::string&>(feature_path), quality_score);
                    } catch (const std::exception& e) {
                        writeLog(std::string("Feature extraction failed for ") + image_name + ": " + e.what(), "WARN");
                        // continue; we still add image metadata with zeros
                    } catch (...) {
                        writeLog(std::string("Feature extraction failed for ") + image_name + ": unknown error", "WARN");
                    }

                    // Build metadata row
                    FrameMetadata md{};
                    md.frame_idx         = frame_idx;
                    md.timestamp_ms      = static_cast<uint64_t>(cap.get(cv::CAP_PROP_POS_MSEC));
                    md.output_image_name = image_name;
                    md.feature_count     = feature_count;
                    md.quality_score     = quality_score;
                    md.georef            = m_config.georef;

                    m_metadata.push_back(md);
                    m_summary.extracted_images.push_back(image_name);
                    extracted++;
                }

                // reset stride counter
                skip_count = 0;
            }

            // Progress callback
            if (m_progress_cb) {
                float prog = (total_frames > 0) ? (static_cast<float>(frame_idx) / static_cast<float>(total_frames)) : 0.0f;
                m_progress_cb(static_cast<size_t>(frame_idx), static_cast<size_t>(total_frames), prog, "Processing");
            }

            frame_idx++;
        }

        m_summary.total_frames_extracted = extracted;

        // Persist metadata
        writeMetadataCSV();
        writeSummaryYAML();
        writeLog("Run complete: extracted " + std::to_string(extracted) + " frames", "INFO");

        return true;
    }


    void VideoFrameExtractor::ensureOutputFolders() const
    {
        if (!m_config.create_output_dirs) return;

        try {
            if (!m_config.output_images_dir.empty())
                fs::create_directories(m_config.output_images_dir);
            if (!m_config.output_features_dir.empty())
                fs::create_directories(m_config.output_features_dir);
            if (!m_config.output_log_file.empty()) {
                auto p = fs::path(m_config.output_log_file).parent_path();
                if (!p.empty()) fs::create_directories(p);
            }
            // CSV/YAML parent dirs
            if (!m_config.output_metadata_csv.empty())
                fs::create_directories(fs::path(m_config.output_metadata_csv).parent_path());
            if (!m_config.output_summary_yaml.empty())
                fs::create_directories(fs::path(m_config.output_summary_yaml).parent_path());
        } catch (...) {
            // Best-effort: let subsequent I/O report failures
        }
    }

    void VideoFrameExtractor::writeMetadataCSV() const
    {
        if (m_config.output_metadata_csv.empty()) return;
        std::ofstream ofs(m_config.output_metadata_csv);
        if (!ofs) return;

        ofs << "frame_idx,timestamp_ms,output_image_name,feature_count,quality_score,georef\n";
        for (const auto& md : m_metadata) {
            ofs << md.frame_idx << ","
                << md.timestamp_ms << ","
                << md.output_image_name << ","
                << md.feature_count << ","
                << md.quality_score << ",";
            if (md.georef.has_value()) ofs << md.georef.value();
            ofs << "\n";
        }
    }

    void VideoFrameExtractor::writeSummaryYAML() const
    {
        if (m_config.output_summary_yaml.empty()) return;
        std::ofstream ofs(m_config.output_summary_yaml);
        if (!ofs) return;

        // Minimal YAML summary (no external deps)
        ofs << "input_video_basename: " << m_summary.input_video_basename << "\n";
        ofs << "total_frames_extracted: " << m_summary.total_frames_extracted << "\n";
        ofs << "run_datetime: " << m_summary.run_datetime << "\n";
        ofs << "config:\n";
        ofs << "  feature_type: " << m_config.feature_type << "\n";
        ofs << "  image_format: " << m_config.image_format << "\n";
        ofs << "  overlap_threshold: " << m_config.overlap_threshold << "\n";
        ofs << "  max_skipped_frames: " << m_config.max_skipped_frames << "\n";
        ofs << "  apply_enhancement: " << (m_config.apply_enhancement ? "true" : "false") << "\n";
        if (m_config.georef.has_value())
            ofs << "  georef: " << *m_config.georef << "\n";
        ofs << "images:\n";
        for (const auto& img : m_summary.extracted_images)
            ofs << "  - " << img << "\n";
    }


    void VideoFrameExtractor::writeLog(const std::string &msg, const std::string &level) const
    {
        if (!m_config.enable_logging)
            return;
        std::ofstream ofs(m_config.output_log_file, std::ios::app);
        ofs << "[" << level << "] " << msg << "\n";
    }

} // namespace videostrip
