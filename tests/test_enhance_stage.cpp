#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <catch2/catch_test_macros.hpp>
#include <videostrip_core/enhance/enhance_yaml.hpp>
#include <videostrip_core/enhance/pipeline_enhance.hpp>

using namespace videostrip;

TEST_CASE("EnhanceStage: converts to 8UC3 and applies sequence", "[enhance][stage]")
{
    // Start with 16U gray; stage should convert and then process
    cv::Mat src16(32, 32, CV_16UC1);
    for (int r = 0; r < src16.rows; ++r)
        for (int c = 0; c < src16.cols; ++c)
            src16.at<uint16_t>(r, c) = uint16_t((r * 32 + c) % 1024);

    cv::Mat frame = src16; // deliberately non-8UC3 input

    EnhanceConfig cfg;
    cfg.enable = true;
    cfg.sequence = {{EnhanceType::GrayWorldWB, GrayWorldParams{}},
                    {EnhanceType::Gamma, GammaParams{1.2}}};

    EnhanceStage stage;
    std::string err;
    REQUIRE(stage.configure(cfg, err));
    REQUIRE(stage.process(frame));
    CHECK(frame.type() == CV_8UC3);
    CHECK(frame.channels() == 3);
}
