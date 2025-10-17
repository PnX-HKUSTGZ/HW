#ifndef LIGHT_DETECTION_HPP_
#define LIGHT_DETECTION_HPP_

#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

const int RED = 0;
const int BLUE = 1;

struct DetectorParams
{
  struct Preprocess
  {
    int binary_threshold = 100;
    int open_ksize = 3;
    int open_iters = 1;
    int close_ksize = 5;
    int close_iters = 1;
    bool use_clahe = true;
    double clahe_clip_limit = 2.0;
    int clahe_tile_size = 8;
  } preprocess;

  struct LightFilter
  {
    double min_ratio = 0.1;
    double max_ratio = 0.4;
    double max_angle_deg = 40.0;
  } light;

  struct Matcher
  {
    bool enforce_same_color = true;
    float min_avg_length = 8.0f;
    float max_horizontal_angle_deg = 12.0f;
    float max_axis_alignment_cos = 0.35f;
    float max_parallel_angle_deg = 12.0f;
    float min_dx_factor = 0.6f;
    float max_dx_factor = 5.0f;
    float max_dy_factor = 0.35f;
    float max_angle_diff_deg = 20.0f;
    float max_length_ratio = 1.6f;
    float preferred_dx_factor = 2.0f;
    float score_angle_weight = 1.0f;
    float score_length_weight = 10.0f;
    float score_dy_weight = 5.0f;
    float score_dx_weight = 2.0f;
  } matcher;

  struct Drawing
  {
    int light_thickness = 2;
    float light_width_scale = 1.05f;
    int pair_thickness = 2;
    float pair_width_scale = 1.05f;
  } drawing;

  struct Video
  {
    bool enable_write = true;
    double min_valid_fps = 5.0;
    double max_valid_fps = 120.0;
    double fallback_fps = 30.0;
    std::string default_output = "annotated.avi";
  } video;

  bool load(const std::string &yaml_path);
};

class Detector
{
public:
  Detector() = default;
  explicit Detector(const DetectorParams &params);

  void setParams(const DetectorParams &params);
  const DetectorParams &params() const;

  cv::Mat preprocessImage(const cv::Mat &rgb_img);

  struct Light : public cv::RotatedRect
  {
    Light() = default;
    explicit Light(cv::RotatedRect box)
        : cv::RotatedRect(box)
    {
      initializeLight(box);
    }

    double min_ratio = 0.1;
    double max_ratio = 0.4;
    double max_angle = 40.0;

    int color = BLUE;
    cv::Point2f top, bottom;
    double length = 0.0;
    double width = 0.0;
    float tilt_angle = 0.0f;

  private:
    void initializeLight(const cv::RotatedRect &box)
    {
      cv::Point2f p[4];
      box.points(p);
      std::sort(p, p + 4, [](const cv::Point2f &a, const cv::Point2f &b)
                { return a.y < b.y; });
      top = (p[0] + p[1]) * 0.5f;
      bottom = (p[2] + p[3]) * 0.5f;

      length = cv::norm(top - bottom);
      width = cv::norm(p[0] - p[1]);

      tilt_angle = static_cast<float>(std::atan2(std::abs(top.x - bottom.x), std::abs(top.y - bottom.y)) * 180.0 / CV_PI);
    }
  };

  std::vector<Light> findLights(const cv::Mat &rgb_img, const cv::Mat &binary_img);

private:
  DetectorParams params_;
};

#endif
