#include "metadata_writer.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace videostrip::io
{

MetadataWriter::MetadataWriter(const ExtractorConfig& cfg)
    : m_cfg(cfg), m_csvPath(cfg.output_metadata_csv), m_yamlPath(cfg.output_summary_yaml)
{
    if (m_csvPath.empty() || m_yamlPath.empty())
    {
        throw std::runtime_error("MetadataWriter: output CSV/YAML paths not set in config");
    }
}

void MetadataWriter::initCSV()
{
    fs::create_directories(m_csvPath.parent_path());
    std::ofstream ofs(m_csvPath, std::ios::trunc);
    if (!ofs)
    {
        throw std::runtime_error("Cannot open CSV for writing: " + m_csvPath.string());
    }
    ofs << "frame_idx,timestamp_ms,output_image,feature_count,quality_score,georef\n";
}

void MetadataWriter::appendFrame(const FrameMetadata& md)
{
    fs::create_directories(m_csvPath.parent_path());
    std::ofstream ofs(m_csvPath, std::ios::app);
    if (!ofs)
    {
        throw std::runtime_error("Cannot open CSV for append: " + m_csvPath.string());
    }

    ofs << md.frame_idx << "," << md.timestamp_ms << "," << md.output_image_name << ","
        << md.feature_count << "," << std::fixed << std::setprecision(4) << md.quality_score << ","
        << (md.georef ? *md.georef : "") << "\n";
}

void MetadataWriter::writeSummary(const RunSummary& summary)
{
    fs::create_directories(m_yamlPath.parent_path());
    std::ofstream ofs(m_yamlPath, std::ios::trunc);
    if (!ofs)
    {
        throw std::runtime_error("Cannot open YAML for writing: " + m_yamlPath.string());
    }

    ofs << "schema_version: " << schemaVersion() << "\n\n";

    ofs << "run:\n";
    ofs << "  datetime: \"" << summary.run_datetime << "\"\n";
    ofs << "  input_video: \"" << summary.input_video_basename << "\"\n";
    ofs << "  total_frames: " << summary.total_frames << "\n";
    ofs << "  total_extracted: " << summary.total_frames_extracted << "\n\n";

    ofs << "config:\n";
    ofs << "  feature_type: \"" << summary.config_used.feature_type << "\"\n";
    ofs << "  image_format: \"" << summary.config_used.image_format << "\"\n";
    ofs << "  overlap_threshold: " << summary.config_used.overlap_threshold << "\n";
    ofs << "  max_skipped_frames: " << summary.config_used.max_skipped_frames << "\n";
    ofs << "  apply_enhancement: " << (summary.config_used.apply_enhancement ? "true" : "false")
        << "\n";
    ofs << "  create_output_dirs: " << (summary.config_used.create_output_dirs ? "true" : "false")
        << "\n";
    ofs << "  enable_logging: " << (summary.config_used.enable_logging ? "true" : "false") << "\n";
    if (!summary.config_used.overlap_mode.empty())
    {
        ofs << "  overlap_mode: \"" << summary.config_used.overlap_mode << "\"\n";
    }

    ofs << "  output_dirs:\n";
    ofs << "    images: \"" << summary.config_used.output_images_dir << "\"\n";
    ofs << "    features: \"" << summary.config_used.output_features_dir << "\"\n";
    ofs << "    csv: \"" << summary.config_used.output_metadata_csv << "\"\n";
    ofs << "    yaml: \"" << summary.config_used.output_summary_yaml << "\"\n";
    ofs << "    log: \"" << summary.config_used.output_log_file << "\"\n\n";

    ofs << "extracted_images:\n";
    for (const auto& img : summary.extracted_images)
    {
        ofs << "  - \"" << img << "\"\n";
    }
}

} // namespace videostrip::io
