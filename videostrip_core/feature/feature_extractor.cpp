/**
 * @file feature_extractor.cpp
 * @brief Feature extraction implementations (kept private to this TU).
 */

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <stdexcept>
#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

#include <videostrip_core/feature/feature_extractor.hpp>

namespace fs = std::filesystem;

namespace videostrip
{

// ------------------------------
// Helpers (local to this TU)
// ------------------------------
namespace
{
    inline cv::Mat to_gray(const cv::Mat& img)
    {
        if (img.channels() == 1) return img;
        cv::Mat gray;
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
        return gray;
    }

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

    // Normalize type string
    std::string up(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return static_cast<char>(std::toupper(c)); });
        return s;
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
                quality_score_out = 0.0;
                return 0;
            }

            // Quality
            quality_score_out = compute_quality_score(img);

            // Detect keypoints (descriptors discarded for now)
            std::vector<cv::KeyPoint> keypoints;
            cv::Mat descriptors;
            orb_->detectAndCompute(img, cv::noArray(), keypoints, descriptors);

            // Ensure parent dir exists
            try {
                fs::create_directories(fs::path(feature_file_out).parent_path());
            } catch (...) { /* ignore */ }

            write_feature_file(feature_file_out, keypoints);
            return static_cast<int>(keypoints.size());
        }

    private:
        cv::Ptr<cv::ORB> orb_;
    };
} // anonymous namespace


// ------------------------------
// AKAZE implementation (internal)
// ------------------------------
namespace
{
    class AKAZEFeatureExtractor final : public FeatureExtractor
    {
    public:
        AKAZEFeatureExtractor()
        : akaze_(cv::AKAZE::create(
              cv::AKAZE::DESCRIPTOR_MLDB, // descriptor_type
              0,                          // descriptor_size (0=auto)
              3,                          // descriptor_channels
              0.001f,                     // threshold
              4,                          // nOctaves
              4,                          // nOctaveLayers
              cv::KAZE::DIFF_PM_G2        // diffusivity
          )) {}

        std::string type() const override { return "AKAZE"; }

        int extract(const std::string& image_path,
                    std::string& feature_file_out,
                    double& quality_score_out) override
        {
            cv::Mat img = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
            if (img.empty()) {
                quality_score_out = 0.0;
                return 0;
            }

            quality_score_out = compute_quality_score(img);

            std::vector<cv::KeyPoint> keypoints;
            cv::Mat descriptors;
            akaze_->detectAndCompute(img, cv::noArray(), keypoints, descriptors);

            try {
                fs::create_directories(fs::path(feature_file_out).parent_path());
            } catch (...) { /* ignore */ }

            write_feature_file(feature_file_out, keypoints);
            return static_cast<int>(keypoints.size());
        }

    private:
        cv::Ptr<cv::AKAZE> akaze_;
    };
} // anonymous namespace


// ------------------------------
// SURF implementation (internal, optional)
// ------------------------------
#if VS_HAVE_XFEATURES2D
namespace
{
    class SURFFeatureExtractor final : public FeatureExtractor
    {
    public:
        SURFFeatureExtractor()
        : surf_(cv::xfeatures2d::SURF::create(
              400.0,   // hessianThreshold
              4,       // nOctaves
              3,       // nOctaveLayers
              false,   // extended descriptor (64 if false, 128 if true)
              false    // upright
          )) {}

        std::string type() const override { return "SURF"; }

        int extract(const std::string& image_path,
                    std::string& feature_file_out,
                    double& quality_score_out) override
        {
            cv::Mat img = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
            if (img.empty()) {
                quality_score_out = 0.0;
                return 0;
            }

            quality_score_out = compute_quality_score(img);

            std::vector<cv::KeyPoint> keypoints;
            cv::Mat descriptors;
            surf_->detectAndCompute(img, cv::noArray(), keypoints, descriptors);

            try {
                fs::create_directories(fs::path(feature_file_out).parent_path());
            } catch (...) { /* ignore */ }

            write_feature_file(feature_file_out, keypoints);
            return static_cast<int>(keypoints.size());
        }

    private:
        cv::Ptr<cv::xfeatures2d::SURF> surf_;
    };
} // anonymous namespace
#endif // VS_HAVE_XFEATURES2D


// ------------------------------
// Factory
// ------------------------------
std::unique_ptr<FeatureExtractor> make_default_extractor(const std::string& type_in)
{
    const std::string t = up(type_in);

    if (t == "AKAZE") {
        return std::make_unique<AKAZEFeatureExtractor>();
    }
#if VS_HAVE_XFEATURES2D
    if (t == "SURF") {
        return std::make_unique<SURFFeatureExtractor>();
    }
#endif
    // Default ORB for unrecognized or unavailable types (e.g., SURF without xfeatures2d)
    return std::make_unique<ORBFeatureExtractor>();
}

} // namespace videostrip
