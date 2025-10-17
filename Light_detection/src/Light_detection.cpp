#include <opencv2/opencv.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Light_detection/FrameProcessing.hpp"
#include "Light_detection/Light_detection.hpp"

Detector::Detector(const DetectorParams &params)
    : params_(params)
{
}

void Detector::setParams(const DetectorParams &params)
{
  params_ = params;
}

const DetectorParams &Detector::params() const
{
  return params_;
}

// ========================= Detector 实现（保持不变） =========================

// 预处理：灰度 + 二值
// 预处理：灰度 + 锐化 + 二值 + 形态学（开/闭）
cv::Mat Detector::preprocessImage(const cv::Mat &rgb_img)
{
  if (rgb_img.type() != CV_8UC3 && rgb_img.type() != CV_8UC4)
  {
    throw std::invalid_argument("Input image must be either CV_8UC3 or CV_8UC4.");
  }

  // 1) 灰度
  cv::Mat gray;
  cv::cvtColor(rgb_img, gray, cv::COLOR_BGR2GRAY);

  // ——局部对比度增强（可配）——
  if (params_.preprocess.use_clahe)
  {
    int tile = std::max(2, params_.preprocess.clahe_tile_size);
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(
        std::max(1.0, params_.preprocess.clahe_clip_limit),
        cv::Size(tile, tile));
    clahe->apply(gray, gray);
  }

  // 3) 二值化
  cv::Mat binary;
  cv::threshold(gray, binary, params_.preprocess.binary_threshold, 255, cv::THRESH_BINARY);
  // 4) 形态学：先开再闭
  auto odd = [](int k)
  { k = std::max(1, k); return (k % 2 == 0) ? (k + 1) : k; };
  const int kOpen = odd(params_.preprocess.open_ksize);
  const int kClose = odd(params_.preprocess.close_ksize);
  const int itOpen = std::max(0, params_.preprocess.open_iters);
  const int itClose = std::max(0, params_.preprocess.close_iters);

  if (itOpen > 0)
  {
    cv::Mat kernelOpen = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kOpen, kOpen));
    cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernelOpen, cv::Point(-1, -1), itOpen);
  }
  if (itClose > 0)
  {
    cv::Mat kernelClose = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kClose, kClose));
    cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, kernelClose, cv::Point(-1, -1), itClose);
  }

  return binary;
}

// 判定是否为灯条（保持不变）
static bool isLight(const Detector::Light &light)
{
  float ratio = light.width / light.length;
  bool ratio_ok = light.min_ratio < ratio && ratio < light.max_ratio;
  bool angle_ok = light.tilt_angle < light.max_angle;
  return ratio_ok && angle_ok;
}

// 寻找灯条（保持不变）
std::vector<Detector::Light> Detector::findLights(const cv::Mat &rgb_img, const cv::Mat &binary_img)
{
  using std::vector;
  CV_Assert(binary_img.type() == CV_8UC1);

  vector<vector<cv::Point>> contours;
  cv::Mat hierarchy;
  cv::findContours(binary_img, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  vector<Detector::Light> lights;
  for (const auto &contour : contours)
  {
    if (contour.size() < 10)
      continue;

    auto r_rect = cv::minAreaRect(contour);
    auto light = Detector::Light(r_rect);
    light.min_ratio = params_.light.min_ratio;
    light.max_ratio = params_.light.max_ratio;
    light.max_angle = params_.light.max_angle_deg;

    if (isLight(light))
    {
      auto rect = light.boundingRect();
      if (0 <= rect.x && 0 <= rect.width && rect.x + rect.width <= rgb_img.cols &&
          0 <= rect.y && 0 <= rect.height && rect.y + rect.height <= rgb_img.rows)
      {
        int sum_r = 0, sum_b = 0;
        auto roi = rgb_img(rect);
        for (int i = 0; i < roi.rows; i++)
        {
          for (int j = 0; j < roi.cols; j++)
          {
            if (cv::pointPolygonTest(contour, cv::Point2f(j + rect.x, i + rect.y), false) >= 0)
            {
              sum_r += roi.at<cv::Vec3b>(i, j)[2];
              sum_b += roi.at<cv::Vec3b>(i, j)[0];
            }
          }
        }
        light.color = sum_r > sum_b ? RED : BLUE;
        lights.emplace_back(light);
      }
    }
  }
  return lights;
}

int main(int argc, char **argv)
{
  std::string config_path = "../config/light_detection.yaml";
  std::vector<std::string> positional_args;
  positional_args.reserve(static_cast<size_t>(argc));

  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];
    if (arg == "--config" || arg == "-c")
    {
      if (i + 1 >= argc)
      {
        std::cerr << "--config requires a file path argument." << std::endl;
        return -1;
      }
      config_path = argv[++i];
    }
    else if (arg == "--help" || arg == "-h")
    {
      std::cout << "Usage: " << argv[0] << " [--config path] [input_path] [output_path]\n"
                << "  input_path  : image or video to process (default ../images/image.png)\n"
                << "  output_path : video output path when processing a video (default from config)\n";
      return 0;
    }
    else if (!arg.empty() && arg[0] == '-')
    {
      std::cerr << "Unknown option: " << arg << std::endl;
      return -1;
    }
    else
    {
      positional_args.push_back(arg);
    }
  }

  DetectorParams params;
  const bool config_loaded = params.load(config_path);
  if (!config_loaded && !config_path.empty())
  {
    std::cerr << "WARN: failed to load config '" << config_path << "', using defaults." << std::endl;
  }

  std::string input_path = positional_args.empty() ? "../images/image.png" : positional_args[0];
  std::string output_path = params.video.default_output;
  if (positional_args.size() > 1)
  {
    output_path = positional_args[1];
  }

  Detector detector(params);
  std::string video_output = output_path;

  if (processVideo(detector, input_path, video_output) == 0)
  {
    return 0;
  }

  cv::Mat bgr_img = cv::imread(input_path, cv::IMREAD_COLOR);
  if (bgr_img.empty())
  {
    std::cerr << "Could not open as video or image: " << input_path << std::endl;
    return -1;
  }

  const auto detection = detectFrame(detector, bgr_img);
  std::cout << "Number of detected lights: " << detection.lights.size() << std::endl;

  annotateFrame(bgr_img, detection, detector.params().drawing);

  cv::imshow("Binary Image", detection.binary);
  cv::imshow("Detected Lights", bgr_img);
  cv::imwrite("../images/binary_image.png", detection.binary);
  cv::imwrite("../images/detected_lights_image.png", bgr_img);
  cv::waitKey(0);
  return 0;
}
