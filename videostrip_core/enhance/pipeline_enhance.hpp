#pragma once
#include <opencv2/core.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <videostrip_core/enhance/enhance_yaml.hpp>
#include <videostrip_core/enhance/image_enhancers.hpp>

namespace videostrip
{

/// Thin pipeline wrapper around Enhancer. One instance per pipeline.
class EnhanceStage
{
public:
    EnhanceStage() = default;

    // Configure from parsed EnhanceConfig
    bool configure(const EnhanceConfig& cfg, std::string& err)
    {
        enabled_ = cfg.enable;
        if (!enabled_)
            return true;
        if (cfg.sequence.empty())
        {
            err = "EnhanceStage: enabled but sequence is empty";
            return false;
        }
        enhancer_.setSequence(cfg.sequence);
        return true;
    }

    // Process a BGR frame in-place. No-op if disabled.
    bool process(cv::Mat& img)
    {
        if (!enabled_)
            return true;
        if (img.empty())
            return false;

        // 1) Depth normalize to 8U
        if (img.depth() != CV_8U)
        {
            // scale per depth
            double alpha = 1.0, beta = 0.0;
            switch (img.depth())
            {
            case CV_16U:
                alpha = 1.0 / 256.0;
                break; // 16U -> 8U (>>8)
            case CV_16S:
                alpha = 1.0 / 256.0;
                beta = 128.0;
                break; // crude shift with bias
            case CV_32F:
                alpha = 255.0;
                break; // assumes [0,1] range
            case CV_64F:
                alpha = 255.0;
                break;
            default:
                break;
            }
            cv::Mat tmp8;
            img.convertTo(tmp8, CV_MAKETYPE(CV_8U, img.channels()), alpha, beta);
            img = std::move(tmp8);
        }

        // 2) Channels normalize to 3 (BGR)
        if (img.channels() == 1)
        {
            cv::Mat bgr;
            cv::cvtColor(img, bgr, cv::COLOR_GRAY2BGR);
            img = std::move(bgr);
        }
        else if (img.channels() == 3)
        {
            // ok
        }
        else if (img.channels() == 4)
        {
            cv::Mat bgr;
            cv::cvtColor(img, bgr, cv::COLOR_BGRA2BGR);
            img = std::move(bgr);
        }
        else
        {
            return false; // unsupported channel count
        }
        return enhancer_.apply(img);
    }

    bool enabled() const
    {
        return enabled_;
    }

private:
    bool enabled_{false};
    Enhancer enhancer_;
};

} // namespace videostrip
