/**
 * @file feature_extractor.cpp
 * @brief Feature extraction implementations (kept private to this TU).
 */

#include <videostrip_core/feature/feature_extractor.hpp>

#include <opencv2/opencv.hpp>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <stdexcept>

namespace fs = std::filesystem;

namespace videostrip
{

// ------------------------------
// Helpers (local to this TU)
// ------------------------------
namespace
{
    // Simple sharpness measure: variance of Laplacian
    double compute_quality_score(const cv::Mat& gray)
    {
        cv::Mat lap;
        cv::Laplacian(gray, lap, CV_64F);
        cv::Scalar mu, sigma;
        cv::meanStdDev(lap, mu, sigma);
        return sigma[0] * sigma[0];
    }

    // Write a very simple text feature file (one keypoint per line)
    // Format: x y size angle response octave class_id
    void write_feature_file(const std::string& path, const std::vector<cv::KeyPoint>& kpts)
    {
        std::ofstream ofs(path);
        if (!ofs) return;
        ofs << "# keypoints: x y size angle response octave class_id\n";
        for (const auto& k : kpts) {
            ofs << k.pt.x << " " << k.pt.y << " "
                << k.size  << " " << k.angle << " "
                << k.response << " " << k.octave << " " << k.class_id << "\n";
        }
    }
} // anonymous namespace


// ------------------------------
// ORB implementation (internal)
// ------------------------------
namespace
{
    class ORBFeatureExtractor final : public FeatureExtractor
    {
    public:
        ORBFeatureExtractor()
        : orb_(cv::ORB::create()) {}

        std::string type() const override { return "ORB"; }

        int extract(const std::string& image_path,
                    std::string& feature_file_out,
                    double& quality_score_out) override
        {
            cv::Mat img = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
            if (img.empty()) {
                // Could not read image
                quality_score_out = 0.0;
                return 0;
            }

            // Quality
            quality_score_out = compute_quality_score(img);

            // Detect keypoints
            std::vector<cv::KeyPoint> keypoints;
            cv::Mat descriptors; // not written for now (text output for keypoints only)
            orb_->detectAndCompute(img, cv::noArray(), keypoints, descriptors);

            // Ensure parent dir exists
            try {
                fs::create_directories(fs::path(feature_file_out).parent_path());
            } catch (...) {
                // Ignore directory errors; write may still fail
            }

            // Write textual keypoints
            write_feature_file(feature_file_out, keypoints);

            return static_cast<int>(keypoints.size());
        }

    private:
        cv::Ptr<cv::ORB> orb_;
    };
} // anonymous namespace


// ------------------------------
// Factory
// ------------------------------
std::unique_ptr<FeatureExtractor> make_default_extractor(const std::string& type)
{
    // As of now, return ORB for anything (safe default; no contrib/nonfree)
    // In the future, add SIFT/KAZE/SURF implementations here as needed.
    (void)type; // suppress unused warning
    return std::make_unique<ORBFeatureExtractor>();
}

} // namespace videostrip
