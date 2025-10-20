#include "metadata_writer.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace fs = std::filesystem;

namespace videostrip::io
{

namespace
{

std::string indent(int level)
{
    return std::string(static_cast<size_t>(level) * 2, ' ');
}

std::string toString(FeatureNormalizationMode mode)
{
    switch (mode)
    {
    case FeatureNormalizationMode::Grid:
        return "grid";
    case FeatureNormalizationMode::None:
    default:
        return "none";
    }
}

std::string toString(GridNormalizationParams::Score score)
{
    return (score == GridNormalizationParams::Score::Size) ? "size" : "response";
}

std::string toString(EnhanceType type)
{
    switch (type)
    {
    case EnhanceType::ContrastOffset:
        return "contrast";
    case EnhanceType::GrayWorldWB:
        return "grayworld";
    case EnhanceType::Gamma:
        return "gamma";
    case EnhanceType::CLAHE:
        return "clahe";
    }
    return "unknown";
}

std::string toString(ClaheSpace space)
{
    switch (space)
    {
    case ClaheSpace::YCrCb:
        return "YCrCb";
    case ClaheSpace::HSV:
        return "HSV";
    case ClaheSpace::Lab:
        return "Lab";
    case ClaheSpace::BGR:
        return "BGR";
    }
    return "YCrCb";
}

std::string formatDouble(double value)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4) << value;
    return oss.str();
}

void writeFeatureNormalization(std::ofstream& ofs, const FeatureNormalizationConfig& norm_cfg)
{
    ofs << indent(1) << "feature_normalization:\n";
    ofs << indent(2) << "mode: \"" << toString(norm_cfg.mode) << "\"\n";
    if (norm_cfg.mode == FeatureNormalizationMode::Grid)
    {
        ofs << indent(2) << "grid:\n";
        ofs << indent(3) << "cell: [" << norm_cfg.grid.cell_w << ", " << norm_cfg.grid.cell_h
            << "]\n";
        ofs << indent(3) << "max_per_cell: " << norm_cfg.grid.max_per_cell << "\n";
        ofs << indent(3) << "score: \"" << toString(norm_cfg.grid.score) << "\"\n";
    }
    ofs << "\n";
}

void writeEnhanceConfig(std::ofstream& ofs, const EnhanceConfig& enhance_cfg)
{
    ofs << indent(1) << "enhance:\n";
    ofs << indent(2) << "enable: " << (enhance_cfg.enable ? "true" : "false") << "\n";

    if (enhance_cfg.sequence.empty())
    {
        ofs << indent(2) << "sequence: []\n\n";
        return;
    }

    ofs << indent(2) << "sequence:\n";
    for (const auto& step : enhance_cfg.sequence)
    {
        ofs << indent(3) << "- type: \"" << toString(step.type) << "\"\n";
        std::visit(
            [&](auto&& params)
            {
                using ParamT = std::decay_t<decltype(params)>;
                if constexpr (std::is_same_v<ParamT, GrayWorldParams>)
                {
                    ofs << indent(3) << "  params: {}\n";
                }
                else if constexpr (std::is_same_v<ParamT, ContrastOffsetParams>)
                {
                    ofs << indent(3) << "  params:\n";
                    ofs << indent(5) << "alpha: " << formatDouble(params.alpha) << "\n";
                    ofs << indent(5) << "beta: " << formatDouble(params.beta) << "\n";
                }
                else if constexpr (std::is_same_v<ParamT, GammaParams>)
                {
                    ofs << indent(3) << "  params:\n";
                    ofs << indent(5) << "gamma: " << formatDouble(params.gamma) << "\n";
                }
                else if constexpr (std::is_same_v<ParamT, ClaheParams>)
                {
                    ofs << indent(3) << "  params:\n";
                    ofs << indent(5) << "clip_limit: " << formatDouble(params.clipLimit) << "\n";
                    ofs << indent(5) << "tile_grid: [" << params.tileGrid.width << ", "
                        << params.tileGrid.height << "]\n";
                    ofs << indent(5) << "space: \"" << toString(params.space) << "\"\n";
                }
            },
            step.params);
    }
    ofs << "\n";
}

} // namespace

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

    writeFeatureNormalization(ofs, summary.config_used.feature_normalization);
    writeEnhanceConfig(ofs, summary.config_used.enhance);

    ofs << "extracted_images:\n";
    for (const auto& img : summary.extracted_images)
    {
        ofs << "  - \"" << img << "\"\n";
    }
}

} // namespace videostrip::io
