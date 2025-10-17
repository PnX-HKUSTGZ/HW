#ifndef DETECTOR_PARAMS_LOADER_HPP_
#define DETECTOR_PARAMS_LOADER_HPP_

#include <string>

struct DetectorParams;

bool loadDetectorParams(DetectorParams &params, const std::string &yaml_path);

#endif
