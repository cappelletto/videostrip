/**
 * @file videostrip_core.cpp
 * @brief Implementation of VideoFrameExtractor for the videostrip pipeline.
 */

#include <chrono>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <sstream>

#include <opencv2/opencv.hpp>

#include <videostrip_core/videostrip_core.hpp>
#include <videostrip_core/keyframe/keyframe_selector.hpp>
#include <videostrip_core/logging/logger.hpp>

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

        // Timestamp
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
        if (!cap.isOpened())
        {
            writeLog("Failed to open video: " + m_config.input_video_path, "ERROR");
            throw std::runtime_error("Cannot open video: " + m_config.input_video_path);
        }

        // Create default feature extractor once per run (if caller didn't provide one)
        if (!m_feature_extractor)
        {
            m_feature_extractor = make_default_extractor(m_config.feature_type);
        }

        // Note: this gives the number of expected frames, according to the header
        // The effective number of frames read may be lower (e.g. error decoding frames)
        const int total_frames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
        int frame_idx = 0;
        int extracted = 0;

        // Selection state
        cv::Mat keyframe_analyze; // resized/gray/bgr as passed to overlap (we keep original color frame separately on export)
        bool have_keyframe = false;

        // Helper lambda: write out one selected frame (image + features + CSV row)
        //------------------------------------------------------------------------------
        // Note: captures by reference; make sure to call only within run()
        auto export_frame = [&](const cv::Mat &bgr_frame, int source_frame_idx, double quality_score)
        {
            std::ostringstream img_name;
            img_name << "frame_" << std::setw(6) << std::setfill('0') << source_frame_idx << "." << m_config.image_format;
            const std::string image_name = img_name.str();
            const std::string image_path = (fs::path(m_config.output_images_dir) / image_name).string();

            // Optional enhancement hook
            if (m_config.apply_enhancement)
            {
                // TODO: enhancement step
                writeLog("Enhancement not implemented; skipping", "WARN");
            }

            // TODO: Once profiled, consider parallelizing image write, via deferred batch or thread pool
            // Most likely will require to promote from lambda to a proper member function
            // Save image
            if (!cv::imwrite(image_path, bgr_frame))
            {
                writeLog("Failed to write image: " + image_path, "WARN");
            }

            // Feature extraction (text keypoints; quality_score already computed from blur window)
            const std::string feature_path =
                (fs::path(m_config.output_features_dir) / (image_name + ".feat.txt")).string();
            int feature_count = 0;
            try
            {
                feature_count = m_feature_extractor->extract(image_path,
                                                             const_cast<std::string &>(feature_path),
                                                             quality_score /* overwrite with extractor’s quality if it sets it */);
            }
            catch (const std::exception &e)
            {
                writeLog(std::string("Feature extraction failed for ") + image_name + ": " + e.what(), "WARN");
                // continue; metadata row still written with zeros
            }
            catch (...)
            {
                writeLog(std::string("Feature extraction failed for ") + image_name + ": unknown error", "WARN");
            }

            FrameMetadata md{};
            md.frame_idx = source_frame_idx;
            md.timestamp_ms = static_cast<uint64_t>(cap.get(cv::CAP_PROP_POS_MSEC));
            md.output_image_name = image_name;
            md.feature_count = feature_count;
            md.quality_score = quality_score;
            md.georef = m_config.georef;

            m_metadata.push_back(md);
            m_summary.extracted_images.push_back(image_name);
            ++extracted;
        }; // end of lambda
        //------------------------------------------------------------------------------

        // Read loop
        cv::Mat frame_bgr;
        while (cap.read(frame_bgr))
        {
            // Prepare analysis image (you can downscale here if needed for speed)
            const cv::Mat analyze = frame_bgr; // currently full-res; consider cv::pyrDown for speed

            // If no keyframe yet, initialize with the very first readable frame
            if (!have_keyframe)
            {
                double q0 = calcBlur(analyze);
                export_frame(frame_bgr, frame_idx, q0);
                keyframe_analyze = analyze.clone();
                have_keyframe = true;

                if (m_progress_cb)
                {
                    float prog = (total_frames > 0) ? (static_cast<float>(frame_idx) / static_cast<float>(total_frames)) : 0.0f;
                    m_progress_cb(static_cast<size_t>(frame_idx), static_cast<size_t>(total_frames), prog, "Init keyframe");
                }
                ++frame_idx;
                continue;
            }

            // Compute overlap against current keyframe
            float overlap = calcOverlap(keyframe_analyze, analyze);
            // Legacy behavior: if overlap can't be computed, force a new keyframe search
            if (overlap == -2.0f)
            {
                overlap = m_config.overlap_threshold - 1e-3f;
            }

            // If overlap below threshold, search best (sharpest) in a small window
            if (overlap <= static_cast<float>(m_config.overlap_threshold))
            {
                const int kWindow = std::max(1, m_config.max_skipped_frames + 1);

                // Start with current frame as best
                cv::Mat best_bgr = frame_bgr.clone();
                double best_blur = calcBlur(analyze);
                int best_index = frame_idx;

                // Look ahead kWindow-1 frames to refine best
                for (int n = 1; n < kWindow; ++n)
                {
                    cv::Mat cand_bgr;
                    if (!cap.read(cand_bgr))
                    {
                        // Reached end; stop refining
                        break;
                    }
                    ++frame_idx;

                    const cv::Mat cand = cand_bgr; // optionally downscale
                    const double curr_blur = calcBlur(cand);
                    if (curr_blur > best_blur)
                    {
                        best_blur = curr_blur;
                        best_bgr = cand_bgr.clone();
                        best_index = frame_idx;
                    }

                    if (m_progress_cb)
                    {
                        float prog = (total_frames > 0) ? (static_cast<float>(frame_idx) / static_cast<float>(total_frames)) : 0.0f;
                        std::ostringstream msg;
                        msg << "Refining (" << n + 1 << "/" << kWindow << ")";
                        m_progress_cb(static_cast<size_t>(frame_idx), static_cast<size_t>(total_frames), prog, msg.str());
                    }
                }

                // Export winner and update keyframe
                export_frame(best_bgr, best_index, best_blur);
                keyframe_analyze = best_bgr.clone(); // keep same scale/colorspace as analyze path

                // Advance outer loop bookkeeping and continue
                if (m_progress_cb)
                {
                    float prog = (total_frames > 0) ? (static_cast<float>(frame_idx) / static_cast<float>(total_frames)) : 0.0f;
                    m_progress_cb(static_cast<size_t>(frame_idx), static_cast<size_t>(total_frames), prog, "New keyframe committed");
                }

                ++frame_idx;
                continue;
            }

            // Otherwise, overlap above threshold → keep scanning
            if (m_progress_cb)
            {
                float prog = (total_frames > 0) ? (static_cast<float>(frame_idx) / static_cast<float>(total_frames)) : 0.0f;
                std::ostringstream msg;
                msg << "Overlap " << std::fixed << std::setprecision(2) << overlap;
                m_progress_cb(static_cast<size_t>(frame_idx), static_cast<size_t>(total_frames), prog, msg.str());
            }
            ++frame_idx;
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
        if (!m_config.create_output_dirs)
            return;

        try
        {
            if (!m_config.output_images_dir.empty())
                fs::create_directories(m_config.output_images_dir);
            if (!m_config.output_features_dir.empty())
                fs::create_directories(m_config.output_features_dir);
            if (!m_config.output_log_file.empty())
            {
                auto p = fs::path(m_config.output_log_file).parent_path();
                if (!p.empty())
                    fs::create_directories(p);
            }
            // CSV/YAML parent dirs
            if (!m_config.output_metadata_csv.empty())
                fs::create_directories(fs::path(m_config.output_metadata_csv).parent_path());
            if (!m_config.output_summary_yaml.empty())
                fs::create_directories(fs::path(m_config.output_summary_yaml).parent_path());
        }
        catch (...)
        {
            // Best-effort: let subsequent I/O report failures
        }
    }

    void VideoFrameExtractor::writeMetadataCSV() const
    {
        if (m_config.output_metadata_csv.empty())
            return;
        std::ofstream ofs(m_config.output_metadata_csv);
        if (!ofs)
            return;

        ofs << "frame_idx,timestamp_ms,output_image_name,feature_count,quality_score,georef\n";
        for (const auto &md : m_metadata)
        {
            ofs << md.frame_idx << ","
                << md.timestamp_ms << ","
                << md.output_image_name << ","
                << md.feature_count << ","
                << md.quality_score << ",";
            if (md.georef.has_value())
                ofs << md.georef.value();
            ofs << "\n";
        }
    }

    void VideoFrameExtractor::writeSummaryYAML() const
    {
        if (m_config.output_summary_yaml.empty())
            return;
        std::ofstream ofs(m_config.output_summary_yaml);
        if (!ofs)
            return;

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
        for (const auto &img : m_summary.extracted_images)
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
