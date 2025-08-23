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
    // FeatureExtractor stub
    // =======================
    class SIFTFeatureExtractor : public FeatureExtractor
    {
    public:
        std::string type() const override { return "SIFT"; }

        int extract(const std::string &image_path,
                    std::string &feature_file_out,
                    double &quality_score_out) override
        {
            // Placeholder: load image, use OpenCV SIFT, write features to file
            cv::Mat img = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
            if (img.empty())
                return 0;

            // Compute sharpness as quality score (variance of Laplacian)
            cv::Mat lap;
            cv::Laplacian(img, lap, CV_64F);
            quality_score_out = cv::mean(lap.mul(lap))[0];

            // TODO: Call OpenCV SIFT or use another method if license/availability
            // For now, pretend we found 42 features
            std::ofstream feats(feature_file_out);
            feats << "# Features for " << image_path << "\n";
            feats << "keypoint1 ...\n";
            feats << "keypoint2 ...\n";
            feats.close();

            return 42; // Placeholder
        }
    };

    // =======================
    // Factory for extractors
    // =======================
    std::unique_ptr<FeatureExtractor> make_default_extractor(const std::string &type)
    {
        // TODO: support more types as needed
        if (type == "SIFT")
            return std::make_unique<SIFTFeatureExtractor>();
        // Add: ORBFeatureExtractor, KAZEFeatureExtractor, etc.
        return std::make_unique<SIFTFeatureExtractor>();
    }

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
        // 1. Ensure outputs
        ensureOutputFolders();

        // 2. Open video
        cv::VideoCapture cap(m_config.input_video_path);
        if (!cap.isOpened())
        {
            writeLog("Failed to open video: " + m_config.input_video_path, "ERROR");
            throw std::runtime_error("Cannot open video: " + m_config.input_video_path);
        }
        int total_frames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
        int frame_idx = 0;
        int frames_extracted = 0;
        int skipped = 0;

        m_metadata.clear();
        m_summary.extracted_images.clear();
        m_summary.input_video_basename = fs::path(m_config.input_video_path).filename().string();
        m_summary.config_used = m_config;
        m_summary.run_datetime = "<timestamp>";
        m_summary.total_frames_extracted = 0;

        cv::Mat frame;
        while (cap.read(frame))
        {
            // Frame selection logic (overlap/quality threshold)
            // For now, extract every Nth frame for simplicity
            bool select = (skipped >= m_config.max_skipped_frames);
            skipped++;

            if (select)
            {
                std::ostringstream img_name;
                img_name << "frame_" << std::setw(6) << std::setfill('0') << frame_idx << "." << m_config.image_format;
                std::string image_path = fs::path(m_config.output_images_dir) / img_name.str();

                // Optionally enhance image
                if (m_config.apply_enhancement)
                {
                    // TODO: implement enhancement
                }

                cv::imwrite(image_path, frame);

                // Feature extraction
                std::string feature_file = fs::path(m_config.output_features_dir) / (img_name.str() + ".feat.txt");
                double quality_score = 0.0;
                int feature_count = 0;
                try
                {
                    feature_count = m_feature_extractor->extract(image_path, feature_file, quality_score);
                }
                catch (...)
                {
                    writeLog("Feature extraction failed for frame " + std::to_string(frame_idx), "WARN");
                    continue; // skip frame, log and move on
                }

                // Metadata
                FrameMetadata md;
                md.frame_idx = frame_idx;
                md.timestamp_ms = static_cast<uint64_t>(cap.get(cv::CAP_PROP_POS_MSEC));
                md.output_image_name = img_name.str();
                md.feature_count = feature_count;
                md.quality_score = quality_score;
                md.georef = m_config.georef;
                m_metadata.push_back(md);

                m_summary.extracted_images.push_back(img_name.str());
                frames_extracted++;
                skipped = 0;
            }

            // Progress callback (every frame)
            if (m_progress_cb)
            {
                float prog = total_frames > 0 ? float(frame_idx) / float(total_frames) : 0.0f;
                m_progress_cb(frame_idx, total_frames, prog, "Processing frame " + std::to_string(frame_idx));
            }

            frame_idx++;
        }

        m_summary.total_frames_extracted = frames_extracted;
        writeMetadataCSV();
        writeSummaryYAML();
        writeLog("Run complete", "INFO");
        return true;
    }

    void VideoFrameExtractor::ensureOutputFolders() const
    {
        if (m_config.create_output_dirs)
        {
            fs::create_directories(m_config.output_images_dir);
            fs::create_directories(m_config.output_features_dir);
            fs::create_directories(fs::path(m_config.output_log_file).parent_path());
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
        std::ofstream ofs(m_config.output_summary_yaml);
        // Minimal YAML summary
        ofs << "input_video_basename: " << m_summary.input_video_basename << "\n";
        ofs << "total_frames_extracted: " << m_summary.total_frames_extracted << "\n";
        ofs << "run_datetime: " << m_summary.run_datetime << "\n";
        ofs << "images:\n";
        for (const auto &img : m_summary.extracted_images)
            ofs << "  - " << img << "\n";
        // TODO: Write out config fields
    }

    void VideoFrameExtractor::writeLog(const std::string &msg, const std::string &level) const
    {
        if (!m_config.enable_logging)
            return;
        std::ofstream ofs(m_config.output_log_file, std::ios::app);
        ofs << "[" << level << "] " << msg << "\n";
    }

} // namespace videostrip
