#include "Light_detection/DetectorParamsLoader.hpp"
#include "Light_detection/Light_detection.hpp"

#include <opencv2/core/persistence.hpp>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <type_traits>

namespace
{
  template <typename T>
  void readIfPresent(const cv::FileNode &node, const std::string &key, T &value)
  {
    if (node.isNone())
      return;
    const cv::FileNode child = node[key];
    if (child.isNone())
      return;

    if constexpr (std::is_same_v<T, bool>)
    {
      if (child.isString())
      {
        std::string text = static_cast<std::string>(child);
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch)
                       { return static_cast<char>(std::tolower(ch)); });
        if (text == "true" || text == "yes" || text == "on" || text == "1")
          value = true;
        else if (text == "false" || text == "no" || text == "off" || text == "0")
          value = false;
      }
      else
      {
        int tmp = 0;
        child >> tmp;
        value = (tmp != 0);
      }
    }
    else
    {
      child >> value;
    }
  }
} // namespace

bool loadDetectorParams(DetectorParams &params, const std::string &yaml_path)
{
  if (yaml_path.empty())
    return false;

  cv::FileStorage fs;
  try
  {
    if (!fs.open(yaml_path, cv::FileStorage::READ))
    {
      std::cerr << "WARN: unable to open config file (not opened): " << yaml_path << std::endl;
      return false;
    }
  }
  catch (const cv::Exception &e)
  {
    std::cerr << "WARN: failed to open config '" << yaml_path
              << "': " << e.what() << std::endl;
    return false;
  }

  const cv::FileNode root = fs.root();

  cv::FileNode preprocess_node = fs["preprocess"];
  if (preprocess_node.isNone())
    preprocess_node = root["preprocess"];
  readIfPresent(preprocess_node, "binary_threshold", params.preprocess.binary_threshold);
  readIfPresent(preprocess_node, "open_ksize", params.preprocess.open_ksize);
  readIfPresent(preprocess_node, "open_iters", params.preprocess.open_iters);
  readIfPresent(preprocess_node, "close_ksize", params.preprocess.close_ksize);
  readIfPresent(preprocess_node, "close_iters", params.preprocess.close_iters);
  readIfPresent(preprocess_node, "use_clahe", params.preprocess.use_clahe);
  readIfPresent(preprocess_node, "clahe_clip_limit", params.preprocess.clahe_clip_limit);
  readIfPresent(preprocess_node, "clahe_tile_size", params.preprocess.clahe_tile_size);

  readIfPresent(root, "binary_threshold", params.preprocess.binary_threshold);

  cv::FileNode light_node = fs["light_filter"];
  if (light_node.isNone())
    light_node = fs["light"];
  if (light_node.isNone())
    light_node = root["light_filter"];
  if (light_node.isNone())
    light_node = root["light"];
  readIfPresent(light_node, "min_ratio", params.light.min_ratio);
  readIfPresent(light_node, "max_ratio", params.light.max_ratio);
  readIfPresent(light_node, "max_angle_deg", params.light.max_angle_deg);

  cv::FileNode matcher_node = fs["matcher"];
  if (matcher_node.isNone())
    matcher_node = root["matcher"];
  readIfPresent(matcher_node, "enforce_same_color", params.matcher.enforce_same_color);
  readIfPresent(matcher_node, "min_avg_length", params.matcher.min_avg_length);
  readIfPresent(matcher_node, "max_horizontal_angle_deg", params.matcher.max_horizontal_angle_deg);
  readIfPresent(matcher_node, "max_axis_alignment_cos", params.matcher.max_axis_alignment_cos);
  readIfPresent(matcher_node, "max_parallel_angle_deg", params.matcher.max_parallel_angle_deg);
  readIfPresent(matcher_node, "min_dx_factor", params.matcher.min_dx_factor);
  readIfPresent(matcher_node, "max_dx_factor", params.matcher.max_dx_factor);
  readIfPresent(matcher_node, "max_dy_factor", params.matcher.max_dy_factor);
  readIfPresent(matcher_node, "max_angle_diff_deg", params.matcher.max_angle_diff_deg);
  readIfPresent(matcher_node, "max_length_ratio", params.matcher.max_length_ratio);
  readIfPresent(matcher_node, "preferred_dx_factor", params.matcher.preferred_dx_factor);
  readIfPresent(matcher_node, "score_angle_weight", params.matcher.score_angle_weight);
  readIfPresent(matcher_node, "score_length_weight", params.matcher.score_length_weight);
  readIfPresent(matcher_node, "score_dy_weight", params.matcher.score_dy_weight);
  readIfPresent(matcher_node, "score_dx_weight", params.matcher.score_dx_weight);

  cv::FileNode drawing_node = fs["drawing"];
  if (drawing_node.isNone())
    drawing_node = root["drawing"];
  readIfPresent(drawing_node, "light_thickness", params.drawing.light_thickness);
  readIfPresent(drawing_node, "light_width_scale", params.drawing.light_width_scale);
  readIfPresent(drawing_node, "pair_thickness", params.drawing.pair_thickness);
  readIfPresent(drawing_node, "pair_width_scale", params.drawing.pair_width_scale);

  cv::FileNode video_node = fs["video"];
  if (video_node.isNone())
    video_node = root["video"];
  readIfPresent(video_node, "enable_write", params.video.enable_write);
  readIfPresent(video_node, "min_valid_fps", params.video.min_valid_fps);
  readIfPresent(video_node, "max_valid_fps", params.video.max_valid_fps);
  readIfPresent(video_node, "fallback_fps", params.video.fallback_fps);
  readIfPresent(video_node, "default_output", params.video.default_output);

  return true;
}

bool DetectorParams::load(const std::string &yaml_path)
{
  return loadDetectorParams(*this, yaml_path);
}
