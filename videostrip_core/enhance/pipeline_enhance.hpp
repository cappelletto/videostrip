#pragma once
#include <opencv2/core.hpp>
#include <string>

#include <iostream>
#include <fstream>

#include <videostrip_core/enhance/image_enhancers.hpp>
#include <videostrip_core/enhance/enhance_yaml.hpp>

namespace videostrip {

/// Thin pipeline wrapper around Enhancer. One instance per pipeline.
class EnhanceStage {
public:
    EnhanceStage() = default;

    // Configure from parsed EnhanceConfig
    bool configure(const EnhanceConfig& cfg, std::string& err) {
        enabled_ = cfg.enable;
        if (!enabled_) return true;
        if (cfg.sequence.empty()) { err = "EnhanceStage: enabled but sequence is empty"; return false; }
        enhancer_.setSequence(cfg.sequence);
        return true;
    }

    // Process a BGR frame in-place. No-op if disabled.
    bool process(cv::Mat& bgr) {
        if (!enabled_) return true;
        if (bgr.empty()) return false;
        std::cout << "EnhanceStage: applying enhancement steps" << std::endl;
        // Expect CV_8UC3. Convert conservatively if needed.
        if (bgr.type() != CV_8UC3) {
            cv::Mat tmp;
            if (bgr.channels() == 1)      cv::cvtColor(bgr, tmp, cv::COLOR_GRAY2BGR);
            else if (bgr.type() == CV_16UC3) {
                cv::Mat f; bgr.convertTo(f, CV_32FC3, 1.0/65535.0);
                f.convertTo(tmp, CV_8UC3, 255.0);
            } else                         bgr.convertTo(tmp, CV_8UC3);
            bgr = std::move(tmp);
        }
        return enhancer_.apply(bgr);
    }

    bool enabled() const { return enabled_; }

private:
    bool enabled_{false};
    Enhancer enhancer_;
};

} // namespace videostrip
