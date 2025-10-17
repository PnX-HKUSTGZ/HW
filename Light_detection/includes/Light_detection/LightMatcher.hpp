#ifndef LIGHT_MATCHER_HPP_
#define LIGHT_MATCHER_HPP_

#include <vector>

#include <opencv2/core/types.hpp>

#include "Light_detection/Light_detection.hpp"

struct LightPair
{
  int left = -1;
  int right = -1;
};

std::vector<LightPair> matchLights(const std::vector<Detector::Light> &lights,
                                   const DetectorParams::Matcher &cfg);

#endif
