#pragma once
/**
 * @file metadata_writer.hpp
 * @brief Handles CSV + YAML output for videostrip_core (schema v1).
 */

#include <string>
#include <vector>
#include <filesystem>

#include <videostrip_core/videostrip_core.hpp>  // ExtractorConfig, FrameMetadata, RunSummary

namespace videostrip::io {

/**
 * @brief MetadataWriter is responsible for persisting pipeline outputs:
 *        - frames.csv (per-frame metadata)
 *        - summary.yaml (run summary)
 *
 *        Contract: schema_version = 1
 */
class MetadataWriter {
public:
    explicit MetadataWriter(const ExtractorConfig& cfg);

    /// Write CSV header (frame_idx,timestamp_ms,...). Overwrites if exists.
    void initCSV();

    /// Append a single row of per-frame metadata to CSV.
    void appendFrame(const FrameMetadata& md);

    /// Write final summary.yaml (always overwrites).
    void writeSummary(const RunSummary& summary);

    /// @return schema version number (currently 1).
    static int schemaVersion() { return 1; }

private:
    ExtractorConfig m_cfg;
    std::filesystem::path m_csvPath;
    std::filesystem::path m_yamlPath;
};

} // namespace videostrip::io
