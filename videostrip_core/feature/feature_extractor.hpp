#pragma once
/**
 * @file feature_extractor.hpp
 * @brief Feature extraction interfaces and factory for videostrip_core.
 * @note Public surface keeps implementation hidden; concrete extractors live in the .cpp.
 */

#include <memory>
#include <string>

namespace videostrip
{

/**
 * @brief Abstract base for feature extractors.
 *
 * Implementations should:
 *  - Read the image from @p image_path
 *  - Compute keypoints/features and a quality score
 *  - Write features to @p feature_file_out (text for now)
 *  - Return the number of features; set @p quality_score_out
 */
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

    /// @return An identifier for the extractor (e.g., "ORB", "SIFT", "KAZE", "SURF").
    virtual std::string type() const = 0;
};

/**
 * @brief Factory for default extractors.
 *
 * Supported names (case-sensitive): "ORB" (always available).
 * Other names ("SIFT", "KAZE", "SURF") currently fall back to ORB to avoid nonfree/contrib deps.
 *
 * @param type Feature type requested.
 * @return std::unique_ptr<FeatureExtractor> A ready-to-use extractor.
 */
std::unique_ptr<FeatureExtractor> make_default_extractor(const std::string& type);

} // namespace videostrip
