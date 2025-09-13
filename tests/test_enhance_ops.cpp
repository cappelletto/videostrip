#include <catch2/catch_test_macros.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <videostrip_core/enhance/image_enhancers.hpp>

using namespace videostrip;

static cv::Mat toy2x2() {
    cv::Mat m(2,2,CV_8UC3);
    m.at<cv::Vec3b>(0,0) = {10,20,30};
    m.at<cv::Vec3b>(0,1) = {40,50,60};
    m.at<cv::Vec3b>(1,0) = {70,80,90};
    m.at<cv::Vec3b>(1,1) = {100,110,120};
    return m;
}

TEST_CASE("ContrastOffset exactness", "[enhance][ops]") {
    cv::Mat m = toy2x2();
    Enhancer e;
    e.setSequence({ {EnhanceType::ContrastOffset, ContrastOffsetParams{2.0, 10.0}} });
    REQUIRE(e.apply(m));
    auto expect = [&](int r,int c){
        auto v = m.at<cv::Vec3b>(r,c);
        auto in = toy2x2().at<cv::Vec3b>(r,c);
        for (int k=0;k<3;++k) {
            int ref = std::min(255, std::max(0, int(2*in[k] + 10)));
            CHECK(v[k] == ref);
        }
    };
    expect(0,0); expect(0,1); expect(1,0); expect(1,1);
}

TEST_CASE("GrayWorld balances means", "[enhance][ops]") {
    cv::Mat m(32,32,CV_8UC3, cv::Scalar(10, 50, 200)); // B,G,R
    Enhancer e;
    e.setSequence({ {EnhanceType::GrayWorldWB, GrayWorldParams{}} });
    REQUIRE(e.apply(m));
    cv::Scalar mean = cv::mean(m);
    CHECK(std::abs(mean[0] - mean[1]) <= 1.5);
    CHECK(std::abs(mean[1] - mean[2]) <= 1.5);
}

TEST_CASE("Gamma LUT monotonic and anchors", "[enhance][ops]") {
    cv::Mat m(1,4,CV_8UC3);
    m.at<cv::Vec3b>(0,0) = {0,0,0};
    m.at<cv::Vec3b>(0,1) = {64,64,64};
    m.at<cv::Vec3b>(0,2) = {128,128,128};
    m.at<cv::Vec3b>(0,3) = {255,255,255};
    Enhancer e;
    e.setSequence({ {EnhanceType::Gamma, GammaParams{2.0}} });
    REQUIRE(e.apply(m));
    CHECK(m.at<cv::Vec3b>(0,0) == cv::Vec3b(0,0,0));
    CHECK(m.at<cv::Vec3b>(0,3) == cv::Vec3b(255,255,255));
    CHECK(m.at<cv::Vec3b>(0,1)[0] < m.at<cv::Vec3b>(0,2)[0]); // monotonic
    CHECK(m.at<cv::Vec3b>(0,2)[0] > 128); // compressed mid-tones
}

TEST_CASE("CLAHE increases gray stddev", "[enhance][ops]") {
    cv::Mat m(64,64,CV_8UC3, cv::Scalar(60,60,60));
    for (int r=24; r<40; ++r) for (int c=0; c<64; ++c)
        m.at<cv::Vec3b>(r,c) = {200,200,200};
    auto stddev_of = [](const cv::Mat& img){
        cv::Mat g; cv::cvtColor(img, g, cv::COLOR_BGR2GRAY);
        cv::Scalar mean, sd; cv::meanStdDev(g, mean, sd); return sd[0];
    };
    double before = stddev_of(m);
    Enhancer e;
    e.setSequence({ {EnhanceType::CLAHE, ClaheParams{2.0, {8,8}, ClaheSpace::YCrCb}} });
    REQUIRE(e.apply(m));
    double after = stddev_of(m);
    CHECK(after > before);
}
