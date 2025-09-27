#pragma once
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <string>
#include <variant>
#include <vector>

namespace videostrip
{

/// Supported Operations - extensible list.
enum class EnhanceType
{
    ContrastOffset, ///< alpha * I + beta (per-pixel, per-channel)
    GrayWorldWB,    ///< scale channels so their means match the global mean
    Gamma,          ///< apply LUT with gamma correction
    CLAHE ///< contrast-limited adaptive histogram equalization //TODO: bring implementation of
          ///< ACLAHE
};

/// Colorspace selection for CLAHE. Channel is implicit:
/// - YCrCb -> Y
/// - HSV   -> V
/// - Lab   -> L
/// - BGR   -> per-channel (3x CLAHE; slower; optional)
enum class ClaheSpace
{
    YCrCb,
    HSV,
    Lab,
    BGR
};

struct ContrastOffsetParams
{
    double alpha{1.0}; ///< multiplicative gain
    double beta{0.0};  ///< additive offset
};

struct GrayWorldParams
{
    // Future: percentile clipping / robust mean. For now basic mean.
};

struct GammaParams
{
    double gamma{1.0}; ///< >0; 1.0 = identity
};

struct ClaheParams
{
    double clipLimit{2.0};   ///< OpenCV CLAHE clipLimit
    cv::Size tileGrid{8, 8}; ///< tile grid size
    ClaheSpace space{ClaheSpace::YCrCb};
};

using EnhanceParams = std::variant<ContrastOffsetParams, GrayWorldParams, GammaParams, ClaheParams>;

/// One step in the sequence
struct EnhanceStep
{
    EnhanceType type;
    EnhanceParams params;
};

/// Enhancer: apply a sequence of operations reusing internal scratch buffers.
/// Not thread-safe across calls; use one instance per worker thread.
class Enhancer
{
public:
    Enhancer() = default;

    /// Replace the entire sequence.
    Enhancer& setSequence(std::vector<EnhanceStep> seq);

    /// Append a step.
    Enhancer& addStep(const EnhanceStep& step);

    /// Clear steps.
    void clear();

    /// Apply sequence in-place to a BGR 8-bit image. Returns false if input invalid.
    /// Notes:
    /// - Input must be CV_8UC3 (BGR). Convert beforehand otherwise.
    /// - Uses internal scratch buffers (no heap churn).
    bool apply(cv::Mat& bgr);

    /// Convenience: parse a simple CSV sequence string like:
    ///   "contrast(alpha=1.2,beta=-5); grayworld; gamma(1.1); clahe(clip=2.0,grid=8x8,space=YCrCb)"
    /// Minimal parser for CLI glue; ignores unknown tokens silently.
    static std::vector<EnhanceStep> parseSequence(const std::string& spec);

private:
    std::vector<EnhanceStep> sequence_;

    // Scratch buffers (reused).
    cv::Mat scratch1_;
    cv::Mat scratch2_;
    cv::Mat lut_; // 256x1 for gamma. We always expect 8-bit input.

    // CLAHE cache to avoid reallocation when params unchanged.
    cv::Ptr<cv::CLAHE> clahe_;
    ClaheParams cachedClaheParams_;
    bool claheValid_{false};

    // Internal ops
    bool opContrastOffset(cv::Mat& bgr, const ContrastOffsetParams& p);
    bool opGrayWorld(cv::Mat& bgr, const GrayWorldParams& p);
    bool opGamma(cv::Mat& bgr, const GammaParams& p);
    bool opClahe(cv::Mat& bgr, const ClaheParams& p);

    void ensureSize(cv::Mat& m, int rows, int cols, int type);
    void buildGammaLUT(double gamma);
    void ensureClahe(const ClaheParams& p);
};

} // namespace videostrip
