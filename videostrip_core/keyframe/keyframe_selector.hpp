#pragma once

#include <opencv2/opencv.hpp>

namespace videostrip {

/**
 * @brief Compute geometric/appearance overlap between a reference keyframe and a new frame.
 *
 * Pipeline-friendly behavior:
 *  - Detect ORB features, match with Hamming + Lowe ratio test.
 *  - If enough matches, estimate homography with RANSAC.
 *  - Return the inlier ratio in [0,1] as an overlap proxy.
 *  - On failure to estimate (too few matches / no H), return -2.0f (legacy sentinel).
 *
 * @param keyframe_bgr_or_gray  Reference image (keyframe). BGR or Gray.
 * @param frame_bgr_or_gray     Current frame. BGR or Gray.
 * @return float  Overlap score in [0,1], or -2.0f if overlap cannot be computed.
 */
float calcOverlap(const cv::Mat& keyframe_bgr_or_gray,
                  const cv::Mat& frame_bgr_or_gray);

/**
 * @brief Compute sharpness proxy via variance of Laplacian (higher = sharper).
 *
 * @param bgr_or_gray  Input frame (BGR or Gray).
 * @return float       Variance (non-negative). Typical ranges depend on content/resolution.
 */
float calcBlur(const cv::Mat& bgr_or_gray);

} // namespace videostrip
