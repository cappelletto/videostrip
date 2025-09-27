#include <opencv2/imgcodecs.hpp>

#include <algorithm>
#include <cctype>
#include <sstream>
#include <videostrip_core/enhance/image_enhancers.hpp>

namespace videostrip
{

//--------------------------
// Public API
//--------------------------
Enhancer& Enhancer::setSequence(std::vector<EnhanceStep> seq)
{
    sequence_ = std::move(seq);
    return *this;
}

Enhancer& Enhancer::addStep(const EnhanceStep& step)
{
    sequence_.push_back(step);
    return *this;
}

void Enhancer::clear()
{
    sequence_.clear();
}

bool Enhancer::apply(cv::Mat& bgr)
{
    if (bgr.empty() || bgr.type() != CV_8UC3)
        return false;

    for (const auto& step : sequence_)
    {
        switch (step.type)
        {
        case EnhanceType::ContrastOffset:
            if (!opContrastOffset(bgr, std::get<ContrastOffsetParams>(step.params)))
                return false;
            break;
        case EnhanceType::GrayWorldWB:
            if (!opGrayWorld(bgr, std::get<GrayWorldParams>(step.params)))
                return false;
            break;
        case EnhanceType::Gamma:
            if (!opGamma(bgr, std::get<GammaParams>(step.params)))
                return false;
            break;
        case EnhanceType::CLAHE:
            if (!opClahe(bgr, std::get<ClaheParams>(step.params)))
                return false;
            break;
        default:
            return false;
        }
    }
    return true;
}

// Simple, permissive parser for convenience on CLI glue. Not bulletproof.
std::vector<EnhanceStep> Enhancer::parseSequence(const std::string& spec)
{
    std::vector<EnhanceStep> out;
    auto trim = [](std::string s)
    {
        s.erase(s.begin(),
                std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); })
                    .base(),
                s.end());
        return s;
    };

    std::stringstream ss(spec);
    std::string token;
    while (std::getline(ss, token, ';'))
    {
        token = trim(token);
        if (token.empty())
            continue;

        auto lparen = token.find('(');
        auto rparen = token.rfind(')');
        std::string name = (lparen == std::string::npos) ? token : token.substr(0, lparen);
        std::string args =
            (lparen != std::string::npos && rparen != std::string::npos && rparen > lparen)
                ? token.substr(lparen + 1, rparen - lparen - 1)
                : "";

        // normalize
        std::string lname;
        lname.resize(name.size());
        std::transform(name.begin(), name.end(), lname.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        auto parseKey = [&](const std::string& key, const std::string& def = "") -> std::string
        {
            // very minimal: expects key=value or "grid=8x8", comma separated
            size_t pos = args.find(key + "=");
            if (pos == std::string::npos)
                return def;
            pos += key.size() + 1;
            size_t end = args.find(',', pos);
            std::string v =
                args.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
            // trim spaces
            v.erase(0, v.find_first_not_of(" \t"));
            v.erase(v.find_last_not_of(" \t") + 1);
            return v;
        };

        if (lname.rfind("contrast", 0) == 0)
        {
            ContrastOffsetParams p;
            if (!args.empty())
            {
                if (auto s = parseKey("alpha"); !s.empty())
                    p.alpha = std::stod(s);
                if (auto s = parseKey("beta"); !s.empty())
                    p.beta = std::stod(s);
            }
            out.push_back({EnhanceType::ContrastOffset, p});
        }
        else if (lname.rfind("grayworld", 0) == 0 || lname == "gray" || lname == "gw")
        {
            out.push_back({EnhanceType::GrayWorldWB, GrayWorldParams{}});
        }
        else if (lname.rfind("gamma", 0) == 0)
        {
            GammaParams p;
            if (!args.empty())
            {
                // allow gamma(1.1) shorthand
                try
                {
                    p.gamma = std::stod(args);
                }
                catch (...)
                {
                    if (auto s = parseKey("value"); !s.empty())
                        p.gamma = std::stod(s);
                }
            }
            out.push_back({EnhanceType::Gamma, p});
        }
        else if (lname.rfind("clahe", 0) == 0)
        {
            ClaheParams p;
            if (!args.empty())
            {
                if (auto s = parseKey("clip"); !s.empty())
                    p.clipLimit = std::stod(s);
                if (auto s = parseKey("grid"); !s.empty())
                {
                    auto x = s.find('x');
                    if (x != std::string::npos)
                    {
                        p.tileGrid.width = std::stoi(s.substr(0, x));
                        p.tileGrid.height = std::stoi(s.substr(x + 1));
                    }
                }
                if (auto s = parseKey("space"); !s.empty())
                {
                    std::string ls;
                    ls.resize(s.size());
                    std::transform(s.begin(), s.end(), ls.begin(),
                                   [](unsigned char c) { return std::tolower(c); });
                    if (ls == "ycrcb" || ls == "ycbcr")
                        p.space = ClaheSpace::YCrCb;
                    else if (ls == "hsv")
                        p.space = ClaheSpace::HSV;
                    else if (ls == "lab")
                        p.space = ClaheSpace::Lab;
                    else if (ls == "bgr")
                        p.space = ClaheSpace::BGR;
                }
            }
            out.push_back({EnhanceType::CLAHE, p});
        }
        else
        {
            // ignore unknown token
        }
    }
    return out;
}

//--------------------------
// Internals
//--------------------------
void Enhancer::ensureSize(cv::Mat& m, int rows, int cols, int type)
{
    if (m.rows != rows || m.cols != cols || m.type() != type)
        m.create(rows, cols, type);
}

bool Enhancer::opContrastOffset(cv::Mat& bgr, const ContrastOffsetParams& p)
{
    // convertTo reuses dst allocation if size/type match
    bgr.convertTo(bgr, bgr.type(), p.alpha, p.beta);
    return true;
}

bool Enhancer::opGrayWorld(cv::Mat& bgr, const GrayWorldParams&)
{
    // Compute per-channel mean; scale each channel to the global mean.
    cv::Scalar means = cv::mean(bgr);
    double mB = means[0], mG = means[1], mR = means[2];
    double mGlobal = (mB + mG + mR) / 3.0;
    if (mB <= 1e-9 || mG <= 1e-9 || mR <= 1e-9)
        return true; // avoid blow-ups

    double sB = mGlobal / mB;
    double sG = mGlobal / mG;
    double sR = mGlobal / mR;

    std::vector<cv::Mat> ch;
    ch.reserve(3);
    cv::split(bgr, ch);
    ch[0].convertTo(ch[0], ch[0].type(), sB, 0);
    ch[1].convertTo(ch[1], ch[1].type(), sG, 0);
    ch[2].convertTo(ch[2], ch[2].type(), sR, 0);
    cv::merge(ch, bgr);
    return true;
}

void Enhancer::buildGammaLUT(double gamma)
{
    if (gamma <= 0.0)
        gamma = 1.0;
    ensureSize(lut_, 1, 256, CV_8UC1);
    double inv = 1.0 / gamma;
    for (int i = 0; i < 256; ++i)
    {
        double n = i / 255.0;
        int v = static_cast<int>(std::round(std::pow(n, inv) * 255.0));
        if (v < 0)
            v = 0;
        else if (v > 255)
            v = 255;
        lut_.at<uchar>(i) = static_cast<uchar>(v);
    }
}

bool Enhancer::opGamma(cv::Mat& bgr, const GammaParams& p)
{
    // Note: gamma LUT is built per call. If you alternate gamma values often,
    // you can cache last gamma value and rebuild conditionally.
    buildGammaLUT(p.gamma);
    cv::LUT(bgr, lut_, bgr);
    return true;
}

void Enhancer::ensureClahe(const ClaheParams& p)
{
    if (!clahe_ || !claheValid_ || p.clipLimit != cachedClaheParams_.clipLimit ||
        p.tileGrid != cachedClaheParams_.tileGrid)
    {
        clahe_ = cv::createCLAHE(p.clipLimit, p.tileGrid);
        cachedClaheParams_ = p;
        claheValid_ = true;
    }
}

static inline void convertBGRTo(const cv::Mat& bgr, cv::Mat& out, ClaheSpace s)
{
    switch (s)
    {
    case ClaheSpace::YCrCb:
        cv::cvtColor(bgr, out, cv::COLOR_BGR2YCrCb);
        break;
    case ClaheSpace::HSV:
        cv::cvtColor(bgr, out, cv::COLOR_BGR2HSV);
        break;
    case ClaheSpace::Lab:
        cv::cvtColor(bgr, out, cv::COLOR_BGR2Lab);
        break;
    case ClaheSpace::BGR:
        out = bgr; /* shallow copy ok */
        break;
    }
}
static inline void convertToBGR(const cv::Mat& in, cv::Mat& bgr, ClaheSpace s)
{
    switch (s)
    {
    case ClaheSpace::YCrCb:
        cv::cvtColor(in, bgr, cv::COLOR_YCrCb2BGR);
        break;
    case ClaheSpace::HSV:
        cv::cvtColor(in, bgr, cv::COLOR_HSV2BGR);
        break;
    case ClaheSpace::Lab:
        cv::cvtColor(in, bgr, cv::COLOR_Lab2BGR);
        break;
    case ClaheSpace::BGR:
        bgr = in; /* shallow copy ok */
        break;
    }
}

bool Enhancer::opClahe(cv::Mat& bgr, const ClaheParams& p)
{
    ensureClahe(p);

    if (p.space == ClaheSpace::BGR)
    {
        // Apply CLAHE per channel (costlier; sometimes desirable)
        std::vector<cv::Mat> ch;
        ch.reserve(3);
        cv::split(bgr, ch);
        for (auto& c : ch)
            clahe_->apply(c, c);
        cv::merge(ch, bgr);
        return true;
    }

    // colorspace -> operate only on the luminance/value-like channel
    ensureSize(scratch1_, bgr.rows, bgr.cols, bgr.type());
    convertBGRTo(bgr, scratch1_, p.space);

    std::vector<cv::Mat> ch;
    ch.reserve(3);
    cv::split(scratch1_, ch);
    // Choose channel: Y(YCrCb)=0, V(HSV)=2, L(Lab)=0 (OpenCV channel order applies)
    int idx = 0;
    if (p.space == ClaheSpace::HSV)
        idx = 2; // H,S,V
    else if (p.space == ClaheSpace::YCrCb)
        idx = 0; // Y,Cr,Cb
    else if (p.space == ClaheSpace::Lab)
        idx = 0; // L,a,b

    clahe_->apply(ch[idx], ch[idx]);
    cv::merge(ch, scratch1_);
    convertToBGR(scratch1_, bgr, p.space);
    return true;
}

} // namespace videostrip
