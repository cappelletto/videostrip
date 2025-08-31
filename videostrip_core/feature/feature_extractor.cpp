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
        // TODO: THe variance and the standard deviation are the same for ranking purposes
        // as both are monotonic functions of each other.
        // TODO: this helper could be moved to a common utility, maybe rely on forward declaration if needed here
        double compute_quality_score(const cv::Mat &gray)
        {
            cv::Mat lap;
            cv::Laplacian(gray, lap, CV_64F);
            cv::Scalar mu, sigma;
            cv::meanStdDev(lap, mu, sigma);
            return sigma[0] * sigma[0];
        }

        // Write a very simple text feature file (one keypoint per line)
        // Format: x y size angle response octave class_id
        // TODO: Use a deferred writer if we go after in-memory processing
        // TODO: Use a more standard format (YAML, XML, JSON, binary, etc.)
        // TODO: Add exporter compatible with Meshroom
        void write_feature_file(const std::string &path, const std::vector<cv::KeyPoint> &kpts)
        {
            std::ofstream ofs(path);
            if (!ofs)
                return;
            ofs << "# keypoints: x y size angle response octave class_id\n";
            for (const auto &k : kpts)
            {
                ofs << k.pt.x << " " << k.pt.y << " "
                    << k.size << " " << k.angle << " "
                    << k.response << " " << k.octave << " " << k.class_id << "\n";
                // TODO: add descriptors if needed - class_id might not be needed
            }
        }
    } // anonymous namespace

    // =======================
    // FeatureExtractor
    // =======================
    // ------------------------------
    // ORB implementation (internal)
    // ------------------------------
    class SIFTFeatureExtractor final : public FeatureExtractor
    {
    public:
        std::string type() const override { return "SIFT"; }

        int extract(const std::string &image_path,
                    std::string &feature_file_out,
                    double &quality_score_out) override
        {
            // Placeholder: load image, use OpenCV SIFT, write features to file
            cv::Mat img = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
            if (img.empty())
                return 0;

            // Compute sharpness as quality score (variance of Laplacian)
            cv::Mat lap;
            cv::Laplacian(img, lap, CV_64F);
            quality_score_out = cv::mean(lap.mul(lap))[0];

            // For now, pretend we found 42 features
            std::ofstream feats(feature_file_out);
            feats << "# Features for " << image_path << "\n";
            feats << "keypoint1 ...\n";
            feats << "keypoint2 ...\n";
            feats.close();

            return 42; // Placeholder
        }
    };

    class ORBFeatureExtractor final : public FeatureExtractor
    {
    public:
        ORBFeatureExtractor()
            : orb_(cv::ORB::create()) {}

        std::string type() const override { return "ORB"; }

        int extract(const std::string &image_path,
                    std::string &feature_file_out,
                    double &quality_score_out) override
        {
            cv::Mat img = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
            if (img.empty())
            {
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
            try
            {
                fs::create_directories(fs::path(feature_file_out).parent_path());
            }
            catch (...)
            {
                // Ignore directory errors; write may still fail
            }

            // Write textual keypoints
            write_feature_file(feature_file_out, keypoints);

            return static_cast<int>(keypoints.size());
        }

    private:
        cv::Ptr<cv::ORB> orb_;
    };

    // ------------------------------
    // Factory
    // ------------------------------
    std::unique_ptr<FeatureExtractor> make_default_extractor(const std::string &type)
    {
        // TODO: support more types as needed
        if (type == "SIFT")
            return std::make_unique<videostrip::SIFTFeatureExtractor>();
        if (type == "ORB")
        {
            // Placeholder: return ORB extractor
            return std::make_unique<ORBFeatureExtractor>();
        }
        // Add: ORBFeatureExtractor, KAZEFeatureExtractor, etc.
        return std::make_unique<videostrip::SIFTFeatureExtractor>();
    }

} // namespace videostrip
