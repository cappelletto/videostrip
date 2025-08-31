#pragma once
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

class FeatureExtractor {
public:
    virtual ~FeatureExtractor() = default;

    /**
     * @brief Extract features for a single image.
     * @param image_path         Path to input image file.
     * @param feature_file_out   Output file path to write features (text).
     * @param quality_score_out  Output: quality score (e.g., sharpness).
     * @return Number of features detected (>=0). Return 0 on failure or no features.
     */
    virtual int extract(const std::string& image_path,
                        std::string& feature_file_out,
                        double& quality_score_out) = 0;

    /// @return An identifier for the extractor (e.g., "ORB", "AKAZE", "SURF").
    virtual std::string type() const = 0;
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
