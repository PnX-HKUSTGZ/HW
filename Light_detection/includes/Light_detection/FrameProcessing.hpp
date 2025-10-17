#ifndef FRAME_PROCESSING_HPP_
#define FRAME_PROCESSING_HPP_

#include <opencv2/core.hpp>

#include <string>
#include <vector>

#include "Light_detection/Light_detection.hpp"
#include "Light_detection/LightMatcher.hpp"

struct DetectionResult
{
  cv::Mat binary;
  std::vector<Detector::Light> lights;
  std::vector<LightPair> pairs;
};

DetectionResult detectFrame(Detector &detector, const cv::Mat &frame);
void annotateFrame(cv::Mat &frame, const DetectionResult &detection, const DetectorParams::Drawing &cfg);
int processVideo(Detector &detector, const std::string &inputPath, std::string &outputPath);

#endif
