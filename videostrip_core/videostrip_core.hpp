#pragma once
/**
 * @file videostrip_core.hpp
 * @author J. Cappelletto
 * @brief Core API for the video frame extraction and feature metadata for underwater mapping pipelines.
 * @version 0.3.0
 * @date 2024-08-05
 *
 * Public API for the core library of the videostrip pipeline.
 * All public API is within the `videostrip` namespace.
 */

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <memory>
#include <cstdint>
#include <optional>
#include <mutex>

namespace videostrip
{

    // -------------------------------------------------------------
    // ExtractorConfig: user-facing config object (YAML/JSON ready)
    // -------------------------------------------------------------
    struct ExtractorConfig
    {
        // Input video and output folders
        std::string input_video_path;
        std::string output_images_dir;
        std::string output_features_dir;
        std::string output_metadata_csv;
        std::string output_summary_yaml;
        std::string output_log_file;

        // Frame selection
        float overlap_threshold = 0.7f; ///< [0.0 - 1.0], min overlap/confidence for selecting frames
        int max_skipped_frames = 5;     ///< Max consecutive frames that may be ignored

        // Image writing
        std::string image_format = "png"; ///< Output format for images

        // Feature extraction
        std::string feature_type = "SIFT"; ///< e.g., SIFT, SURF, ORB, KAZE
        bool apply_enhancement = false;    ///< Optional image enhancement

        // Output/robustness
        bool create_output_dirs = true; ///< If true, will create folders if missing

        // Logging
        bool enable_logging = true; ///< If true, log to output_log_file

        // Reserved for extension (georef, etc.)
        std::optional<std::string> georef; ///< Georef tag for session (optional)
    };

    // -------------------------------------------------------------
    // FrameMetadata: One row per extracted frame (CSV)
    // -------------------------------------------------------------
    struct FrameMetadata
    {
        int frame_idx;                     ///< Sequential index in video
        uint64_t timestamp_ms;             ///< Timestamp of frame in ms
        std::string output_image_name;     ///< Name of image file for this frame
        int feature_count;                 ///< Keypoints/features found
        double quality_score;              ///< E.g., sharpness, entropy, or similar
        std::optional<std::string> georef; ///< Optional geotag, if present
    };

    // -------------------------------------------------------------
    // RunSummary: Info for YAML sidecar
    // -------------------------------------------------------------
    struct RunSummary
    {
        std::string input_video_basename;
        ExtractorConfig config_used;
        int total_frames_extracted;
        std::vector<std::string> extracted_images;
        std::string run_datetime; ///< ISO8601 timestamp of execution
    };

    // -------------------------------------------------------------
    // Abstract Base for Feature Extractors
    // -------------------------------------------------------------
    class FeatureExtractor
    {
    public:
        virtual ~FeatureExtractor() = default;
        /// Compute keypoints/features for the given image file, return number found
        virtual int extract(const std::string &image_path,
                            std::string &feature_file_out,
                            double &quality_score_out) = 0;
        /// Return feature type name (e.g., "SIFT")
        virtual std::string type() const = 0;
    };

    // -------------------------------------------------------------
    // Main API Class: VideoFrameExtractor
    // -------------------------------------------------------------
    class VideoFrameExtractor
    {
    public:
        /// Progress callback: (current_frame_idx, total_frames, progress_0_1, message)
        using ProgressCallback = std::function<void(size_t, size_t, float, const std::string &)>;

        /// Construct with config and optional logger
        explicit VideoFrameExtractor(const ExtractorConfig &config);

        /// Set an optional progress callback (may be called from multiple threads)
        void setProgressCallback(ProgressCallback cb);

        /// (Optional) Set an external logger or log level
        void setLogger(std::shared_ptr<class Logger> logger);

        /// Run extraction on the input video. Returns true on success, false on recoverable error.
        /// Throws std::runtime_error on critical/fatal errors (bad config, can't open video, etc).
        bool run();

        /// Get vector of all per-frame metadata (populated after run)
        const std::vector<FrameMetadata> &getExtractedMetadata() const;

        /// Get run summary (populated after run)
        const RunSummary &getRunSummary() const;

        /// Set a different feature extractor (advanced/extensibility)
        void setFeatureExtractor(std::unique_ptr<FeatureExtractor> extractor);

        /// Destructor
        ~VideoFrameExtractor();

    private:
        ExtractorConfig m_config;
        std::vector<FrameMetadata> m_metadata;
        RunSummary m_summary;
        ProgressCallback m_progress_cb;
        std::unique_ptr<FeatureExtractor> m_feature_extractor;
        std::shared_ptr<class Logger> m_logger;

        // Internal helpers (not public API)
        void writeMetadataCSV() const;
        void writeSummaryYAML() const;
        void writeLog(const std::string &msg, const std::string &level = "INFO") const;
        void ensureOutputFolders() const;
    };

    /// Provide a default FeatureExtractor for a given type (factory helper)
    std::unique_ptr<FeatureExtractor> make_default_extractor(const std::string &type);

    /// Minimal logger interface for file logging and in-memory debugging
    class Logger
    {
    public:
        virtual ~Logger() = default;
        virtual void info(const std::string &msg) = 0;
        virtual void warn(const std::string &msg) = 0;
        virtual void error(const std::string &msg) = 0;
        virtual void debug(const std::string &msg) = 0;
    };

    class ConsoleLogger : public Logger
    {
    public:
        ConsoleLogger(const std::string &publisher = "core")
            : m_publisher(publisher) {}

        void info(const std::string &msg) override
        {
            publish("INFO", msg);
        }
        void warn(const std::string &msg) override
        {
            publish("WARN", msg);
        }
        void error(const std::string &msg) override
        {
            publish("ERROR", msg);
        }
        void debug(const std::string &msg) override
        {
            publish("DEBUG", msg);
        }

        // Optionally, overloads with publisher tag
        void info(const std::string &publisher, const std::string &msg)
        {
            publish("INFO", msg, publisher);
        }
        void warn(const std::string &publisher, const std::string &msg)
        {
            publish("WARN", msg, publisher);
        }
        void error(const std::string &publisher, const std::string &msg)
        {
            publish("ERROR", msg, publisher);
        }
        void debug(const std::string &publisher, const std::string &msg)
        {
            publish("DEBUG", msg, publisher);
        }

    private:
        std::string m_publisher;
        std::mutex mtx;

        void publish(const std::string &level, const std::string &msg, const std::string &publisher = "")
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::string tag = publisher.empty() ? m_publisher : publisher;
            std::cout << "[" << level << "] <" << tag << "> " << msg << std::endl;
        }
    };

} // namespace videostrip
