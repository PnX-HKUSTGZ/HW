#include "Light_detection/FrameProcessing.hpp"

#include <opencv2/opencv.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>

namespace
{
  static cv::Scalar pairColor(int k)
  {
    static const cv::Scalar palette[] = {
        {0, 255, 0},     // 绿
        {0, 165, 255},   // 橙
        {255, 0, 0},     // 蓝
        {255, 255, 0},   // 青
        {255, 0, 255},   // 品红
        {0, 255, 255},   // 黄
        {128, 0, 128},   // 紫
        {203, 192, 255}, // 浅粉
        {147, 20, 255},  // 深粉
        {50, 205, 50}    // 亮绿
    };
    return palette[k % (int)(sizeof(palette) / sizeof(palette[0]))];
  }

  static void drawLightBox(cv::Mat &img, const Detector::Light &light,
                           const cv::Scalar &color, int thickness, float width_scale)
  {
    cv::Point2f t = light.top;
    cv::Point2f b = light.bottom;
    cv::Point2f c = (t + b) * 0.5f;
    cv::Point2f v = b - t;
    float L = std::sqrt(v.x * v.x + v.y * v.y);
    if (L < 1e-3f)
      return;
    cv::Point2f d = v * (1.0f / L);
    cv::Point2f n(-d.y, d.x);

    float half_len = 0.5f * L;
    float half_w = 0.5f * std::max(2.0f, static_cast<float>(light.width)) * width_scale;

    cv::Point2f pts[4] = {
        c + d * half_len + n * half_w,
        c + d * half_len - n * half_w,
        c - d * half_len - n * half_w,
        c - d * half_len + n * half_w};

    for (int i = 0; i < 4; ++i)
      cv::line(img, pts[i], pts[(i + 1) % 4], color, thickness);
  }

  static inline cv::Point2f centerOf(const Detector::Light &L)
  {
    return (L.top + L.bottom) * 0.5f;
  }

  static inline float lengthOf(const Detector::Light &L)
  {
    return std::hypot(L.bottom.x - L.top.x, L.bottom.y - L.top.y);
  }

  static inline void axisDirs(const Detector::Light &L, cv::Point2f &d, cv::Point2f &n, float &half_len, float &half_w)
  {
    cv::Point2f v = L.bottom - L.top;
    float Llen = std::max(1e-3f, std::hypot(v.x, v.y));
    d = v * (1.0f / Llen);
    n = cv::Point2f(-d.y, d.x);
    half_len = 0.5f * Llen;
    half_w = 0.5f * std::max(2.0f, static_cast<float>(L.width));
  }

  static void drawXBetween(cv::Mat &img,
                           const Detector::Light &A,
                           const Detector::Light &B,
                           const cv::Scalar &color,
                           int thickness,
                           float width_scale)
  {
    cv::Point2f cA = centerOf(A), cB = centerOf(B);
    cv::Point2f dA, nA, dB, nB;
    float hLA, wA, hLB, wB;
    axisDirs(A, dA, nA, hLA, wA);
    axisDirs(B, dB, nB, hLB, wB);
    wA *= width_scale;
    wB *= width_scale;

    float sA = ((cB - cA).x * nA.x + (cB - cA).y * nA.y) >= 0 ? 1.f : -1.f;
    float sB = ((cA - cB).x * nB.x + (cA - cB).y * nB.y) >= 0 ? 1.f : -1.f;

    cv::Point2f A_top = cA + dA * hLA;
    cv::Point2f A_bottom = cA - dA * hLA;
    cv::Point2f B_top = cB + dB * hLB;
    cv::Point2f B_bottom = cB - dB * hLB;

    cv::Point2f A_in_top = A_top + nA * (sA * wA);
    cv::Point2f A_in_bottom = A_bottom + nA * (sA * wA);
    cv::Point2f B_in_top = B_top + nB * (sB * wB);
    cv::Point2f B_in_bottom = B_bottom + nB * (sB * wB);

    cv::line(img, A_in_top, B_in_bottom, color, thickness);
    cv::line(img, A_in_bottom, B_in_top, color, thickness);
  }

  static bool endsWithCaseInsensitive(const std::string &text, const std::string &suffix)
  {
    if (text.size() < suffix.size())
      return false;
    return std::equal(suffix.rbegin(), suffix.rend(), text.rbegin(),
                      [](char a, char b)
                      {
                        return std::tolower(static_cast<unsigned char>(a)) ==
                               std::tolower(static_cast<unsigned char>(b));
                      });
  }

  static bool waitWhilePaused()
  {
    while (true)
    {
      const int key = cv::waitKey(0);
      if (key == 'p')
        return true;
      if (key == 27 || key == 'q')
        return false;
    }
  }

  static bool ensureWriter(cv::VideoWriter &writer,
                           std::string &outPath,
                           bool &wantSave,
                           const DetectorParams::Video &cfg,
                           double fps,
                           const cv::Size &frameSize)
  {
    if (!wantSave || writer.isOpened())
      return writer.isOpened();

    const int api = cv::CAP_FFMPEG;
    auto tryOpen = [&](const std::string &path, int fourcc)
    {
      writer.release();
      return writer.open(path, api, fourcc, fps, frameSize, true);
    };

    auto candidate = outPath;
    const std::string defaultOutput = cfg.default_output.empty() ? "annotated.avi" : cfg.default_output;

    if (endsWithCaseInsensitive(candidate, ".mp4"))
    {
      if (tryOpen(candidate, cv::VideoWriter::fourcc('m', 'p', '4', 'v')))
      {
        outPath = candidate;
        std::cout << "Saving via FFMPEG to " << outPath << " @ " << fps << " FPS\n";
        return true;
      }
      std::cerr << "mp4v(ffmpeg) failed, fallback to MJPG/AVI\n";
      candidate.clear();
    }

    if (candidate.empty())
      candidate = defaultOutput;
    if (candidate.empty())
      candidate = "annotated.avi";

    if (!endsWithCaseInsensitive(candidate, ".avi"))
      candidate += ".avi";

    if (tryOpen(candidate, cv::VideoWriter::fourcc('M', 'J', 'P', 'G')) ||
        tryOpen(candidate, cv::VideoWriter::fourcc('X', 'V', 'I', 'D')))
    {
      outPath = candidate;
      std::cout << "Saving via FFMPEG to " << outPath << " @ " << fps << " FPS\n";
      return true;
    }

    std::cerr << "WARN: cannot open video writer. Disable saving.\n";
    wantSave = false;
    writer.release();
    return false;
  }
} // namespace

DetectionResult detectFrame(Detector &detector, const cv::Mat &frame)
{
  DetectionResult result;
  result.binary = detector.preprocessImage(frame);
  result.lights = detector.findLights(frame, result.binary);
  result.pairs = matchLights(result.lights, detector.params().matcher);
  return result;
}

void annotateFrame(cv::Mat &frame,
                   const DetectionResult &detection,
                   const DetectorParams::Drawing &cfg)
{
  std::vector<int> tags(detection.lights.size(), -1);
  for (size_t idx = 0; idx < detection.pairs.size(); ++idx)
  {
    const auto &p = detection.pairs[idx];
    if (p.left >= 0 && p.left < (int)tags.size())
      tags[p.left] = static_cast<int>(idx);
    if (p.right >= 0 && p.right < (int)tags.size())
      tags[p.right] = static_cast<int>(idx);
  }

  for (size_t i = 0; i < detection.lights.size(); ++i)
  {
    const cv::Scalar color = tags[i] >= 0 ? pairColor(tags[i]) : cv::Scalar(180, 180, 180);
    drawLightBox(frame, detection.lights[i], color, cfg.light_thickness, cfg.light_width_scale);
  }

  for (size_t idx = 0; idx < detection.pairs.size(); ++idx)
  {
    const auto &p = detection.pairs[idx];
    drawXBetween(frame,
                 detection.lights[p.left],
                 detection.lights[p.right],
                 pairColor(static_cast<int>(idx)),
                 cfg.pair_thickness,
                 cfg.pair_width_scale);
  }
}

int processVideo(Detector &detector, const std::string &inputPath, std::string &outputPath)
{
  cv::VideoCapture cap(inputPath);
  if (!cap.isOpened())
    return -1;

  std::cout << "Open as video: " << inputPath << std::endl;

  const auto &videoCfg = detector.params().video;
  bool wantSave = videoCfg.enable_write && !outputPath.empty();
  cv::VideoWriter writer;

  const double fpsIn = cap.get(cv::CAP_PROP_FPS);
  const double fps = (std::isfinite(fpsIn) && fpsIn >= videoCfg.min_valid_fps && fpsIn <= videoCfg.max_valid_fps)
                         ? std::round(fpsIn)
                         : videoCfg.fallback_fps;

  cv::namedWindow("Binary Image", cv::WINDOW_AUTOSIZE);
  cv::namedWindow("Detected Lights", cv::WINDOW_AUTOSIZE);

  cv::Mat frame;
  while (cap.read(frame) && !frame.empty())
  {
    ensureWriter(writer, outputPath, wantSave, videoCfg, fps, frame.size());

    const auto detection = detectFrame(detector, frame);
    annotateFrame(frame, detection, detector.params().drawing);

    cv::imshow("Binary Image", detection.binary);
    cv::imshow("Detected Lights", frame);
    if (writer.isOpened())
      writer.write(frame);

    const int key = cv::waitKey(1);
    if (key == 27 || key == 'q')
      break;
    if (key == 'p' && !waitWhilePaused())
      break;
  }

  if (writer.isOpened())
  {
    writer.release();
    if (!outputPath.empty())
      std::cout << "Saved annotated video to: " << outputPath << std::endl;
  }

  cap.release();
  return 0;
}
