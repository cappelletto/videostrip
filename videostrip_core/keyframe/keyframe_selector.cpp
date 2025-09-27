#include <algorithm>
#include <videostrip_core/keyframe/keyframe_selector.hpp>

namespace videostrip
{

namespace
{
inline cv::Mat to_gray(const cv::Mat& img)
{
    if (img.channels() == 1)
        return img;
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    return gray;
}
} // namespace

float calcOverlap(const cv::Mat& keyframe_bgr_or_gray, const cv::Mat& frame_bgr_or_gray)
{
    // Preconditions
    if (keyframe_bgr_or_gray.empty() || frame_bgr_or_gray.empty())
        return -2.0f;

    // 1) Grayscale
    cv::Mat kgray = to_gray(keyframe_bgr_or_gray);
    cv::Mat fgray = to_gray(frame_bgr_or_gray);

    // 2) Detect + describe with ORB (keeps us out of nonfree/contrib land)
    //    Parameters are conservative; feel free to tune.
    auto orb = cv::ORB::create(1000, // nfeatures
                               1.2f, // scaleFactor
                               8,    // nlevels
                               31,   // edgeThreshold
                               0,    // firstLevel
                               2,    // WTA_K
                               cv::ORB::HARRIS_SCORE,
                               31, // patchSize
                               20  // fastThreshold
    );

    std::vector<cv::KeyPoint> kptsK, kptsF;
    cv::Mat descK, descF;
    orb->detectAndCompute(kgray, cv::noArray(), kptsK, descK);
    orb->detectAndCompute(fgray, cv::noArray(), kptsF, descF);

    if (descK.empty() || descF.empty() || kptsK.size() < 8 || kptsF.size() < 8)
        return -2.0f;

    // 3) Match (Hamming) + Lowe ratio test
    cv::BFMatcher matcher(cv::NORM_HAMMING, /*crossCheck=*/false);
    std::vector<std::vector<cv::DMatch>> knn;
    matcher.knnMatch(descK, descF, knn, 2);

    std::vector<cv::DMatch> good;
    good.reserve(knn.size());
    const float ratio = 0.75f;
    for (const auto& v : knn)
    {
        if (v.size() < 2)
            continue;
        if (v[0].distance < ratio * v[1].distance)
            good.push_back(v[0]);
    }

    if (good.size() < 8)
        return -2.0f;

    // 4) Estimate homography with RANSAC; compute inlier ratio
    std::vector<cv::Point2f> ptsK, ptsF;
    ptsK.reserve(good.size());
    ptsF.reserve(good.size());
    for (const auto& m : good)
    {
        ptsK.push_back(kptsK[m.queryIdx].pt);
        ptsF.push_back(kptsF[m.trainIdx].pt);
    }

    cv::Mat inlierMask;
    cv::Mat H = cv::findHomography(ptsK, ptsF, cv::RANSAC, 3.0, inlierMask);
    if (H.empty() || inlierMask.empty())
        return -2.0f;

    int inliers = cv::countNonZero(inlierMask);
    int total = static_cast<int>(good.size());
    if (total <= 0)
        return -2.0f;

    float inlier_ratio = static_cast<float>(inliers) / static_cast<float>(total);
    // Clamp to [0,1] just in case of any numeric oddities
    inlier_ratio = std::max(0.0f, std::min(1.0f, inlier_ratio));
    return inlier_ratio;
}

float calcBlur(const cv::Mat& bgr_or_gray)
{
    if (bgr_or_gray.empty())
        return 0.0f;

    cv::Mat gray = to_gray(bgr_or_gray);

    // Variance of Laplacian
    cv::Mat lap;
    cv::Laplacian(gray, lap, CV_64F);
    cv::Scalar mu, sigma;
    cv::meanStdDev(lap, mu, sigma);
    const double var = sigma[0] * sigma[0];

    // Return as float; typical values can be tens to thousands depending on content/scale
    return static_cast<float>(var);
}

} // namespace videostrip
