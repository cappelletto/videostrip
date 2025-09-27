/**
 * @file feature_extractor.hpp
 * @brief Feature extraction interfaces and factory for videostrip_core.
 *
 * Supported types (case-insensitive):
 *  - "ORB"   : always available (default)
 *  - "AKAZE" : available in core OpenCV
 *  - "SURF"  : available only if OpenCV xfeatures2d (nonfree) is present
 *
 * Notes:
 *  - The extractor writes a simple TEXT keypoint file (one per line).
 *  - The quality score is a simple sharpness proxy (variance of Laplacian).
 */
#pragma once

#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>

#include <memory>
#include <string>

// TODO: We can convert this into a CMake stage xfeatures2d check
// Conditionally enable SURF if headers exist
#if __has_include(<opencv2/xfeatures2d.hpp>)
#include <opencv2/xfeatures2d.hpp>
#define VS_HAVE_XFEATURES2D 1
#else
#define VS_HAVE_XFEATURES2D 0
#endif

namespace videostrip
{
// #23: Normalization controls
enum class FeatureNormalizationMode
{
    None = 0,
    Grid // enforce per-cell cap using a score (e.g., response)
};

// #23: Parameters for grid-based normalization
struct GridNormalizationParams
{
    // Either define by cell size in pixels (recommended for robustness),
    // or by grid rows/cols (mutually exclusive; prefer cell_w/h)
    int cell_w{32};       // pixels
    int cell_h{32};       // pixels
    int max_per_cell{50}; // cap per cell
    // Score key to pick "best" per cell. For now: response or size
    enum class Score
    {
        Response,
        Size
    } score{Score::Response};
};

// #23: Overall normalization config
struct FeatureNormalizationConfig
{
    FeatureNormalizationMode mode{FeatureNormalizationMode::None};
    GridNormalizationParams grid{};
};

class FeatureExtractor
{
public:
    virtual ~FeatureExtractor() = default;

    /**
     * @brief Extract features for a single image.
     * @param image_path         Path to input image file.
     * @param feature_file_out   Output file path to write features (text).
     * @param quality_score_out  Output: quality score (e.g., sharpness).
     * @return Number of features detected (>=0). Return 0 on failure or no features.
     */
    virtual int extract(const std::string& image_path, std::string& feature_file_out,
                        double& quality_score_out) = 0;

    /// @return An identifier for the extractor (e.g., "ORB", "AKAZE", "SURF").
    virtual std::string type() const = 0;
    // #23: Configure normalization (optional; keep defaults if not called)
    void set_normalization(const FeatureNormalizationConfig& cfg)
    {
        norm_cfg_ = cfg;
    }

protected:
    // Helpers for subclasses (post-detect normalization)
    void apply_normalization(const cv::Size& img_size, std::vector<cv::KeyPoint>& keypoints) const;

    // Normalization config (default: none)
    FeatureNormalizationConfig norm_cfg_{};
};

/**
 * @brief Factory for default extractors.
 *
 * Recognized @p type strings (case-insensitive): "ORB", "AKAZE", "SURF".
 * If a requested type is unavailable (e.g., SURF without xfeatures2d), the
 * factory will fall back to ORB.
 */
std::unique_ptr<FeatureExtractor> make_default_extractor(const std::string& type);

} // namespace videostrip
