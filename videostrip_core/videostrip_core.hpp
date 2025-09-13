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
#pragma once

#include <cstdint>
#include <iostream>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <videostrip_core/feature/feature_extractor.hpp>
#include <videostrip_core/enhance/enhance_yaml.hpp> 
#include <videostrip_core/enhance/pipeline_enhance.hpp>

/**
 * @namespace videostrip
 * @brief Core types and API for video frame extraction and feature analysis.
 */

namespace videostrip
{

    // -------------------------------------------------------------
    // ExtractorConfig: user-facing config object (YAML/JSON ready)
    // -------------------------------------------------------------
    /**
     * @struct ExtractorConfig
     * @brief User-facing configuration object for video frame extraction.
     *
     * Contains input/output paths, frame selection parameters, feature extraction options,
     * output/robustness flags, logging options, and reserved extension fields.
     */
    struct ExtractorConfig
    {
        // Input video and output folders
        std::string input_video_path;
        std::string output_images_dir;
        std::string output_features_dir;
        std::string output_metadata_csv;
        std::string output_summary_yaml;
        std::string output_log_file;
        std::string overlap_mode; ///< Reserved for future use, from args, options are FEATURE|FLOW|ECC

        videostrip::EnhanceConfig enhance; //< Image enhancement config

        // Frame selection
        float overlap_threshold = 0.7f; ///< [0.0 - 1.0], min overlap/confidence for selecting frames
        int max_skipped_frames = 5;     ///< Max consecutive frames that may be ignored

        // Image writing
        std::string image_format = "png"; ///< Output format for images

        // Feature extraction
        std::string feature_type = "SIFT"; ///< e.g., SIFT, SURF, ORB, AKAZE
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
    /**
     * @struct FrameMetadata
     * @brief Metadata for each extracted video frame.
     *
     * Represents a row in the output CSV, including frame index, timestamp, output image name,
     * feature count, quality score, and optional georeferencing.
     */
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

    /**
     * @struct RunSummary
     * @brief Summary information for a video extraction run.
     *
     * Used for YAML sidecar output, includes input video basename, config used, total frames extracted,
     * list of extracted images, and run timestamp.
     */
    struct RunSummary
    {
        std::string input_video_basename;
        ExtractorConfig config_used;
        int total_frames;            ///< Total frames in input video
        int total_frames_extracted;  ///< Number of frames extracted
        std::vector<std::string> extracted_images;
        std::string run_datetime; ///< ISO8601 timestamp of execution
    };

    // -------------------------------------------------------------
    // Main API Class: VideoFrameExtractor
    // -------------------------------------------------------------
    /**
     * @class VideoFrameExtractor
     * @brief Main API class for extracting frames and features from video.
     *
     * Handles configuration, progress callbacks, logging, running extraction, and accessing results.
     */
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
        EnhanceStage m_enhance_stage;  // Image enhancement stage
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

} // namespace videostrip
