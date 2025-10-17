#include "Light_detection/LightMatcher.hpp"

#include <algorithm>
#include <cmath>

namespace
{
  inline cv::Point2f centerOf(const Detector::Light &light)
  {
    return (light.top + light.bottom) * 0.5f;
  }

  inline float lengthOf(const Detector::Light &light)
  {
    return std::hypot(light.bottom.x - light.top.x, light.bottom.y - light.top.y);
  }

  inline void axisDirs(const Detector::Light &light,
                       cv::Point2f &longAxis,
                       cv::Point2f &shortAxis,
                       float &halfLength,
                       float &halfWidth)
  {
    cv::Point2f v = light.bottom - light.top;
    const float len = std::max(1e-3f, std::hypot(v.x, v.y));
    longAxis = v * (1.0f / len);
    shortAxis = cv::Point2f(-longAxis.y, longAxis.x);
    halfLength = 0.5f * len;
    halfWidth = 0.5f * std::max(2.0f, static_cast<float>(light.width));
  }
} // namespace

std::vector<LightPair> matchLights(const std::vector<Detector::Light> &lights,
                                   const DetectorParams::Matcher &cfg)
{
  struct Candidate
  {
    int left;
    int right;
    float score;
  };

  std::vector<Candidate> candidates;
  const int count = static_cast<int>(lights.size());

  for (int i = 0; i < count; ++i)
  {
    for (int j = i + 1; j < count; ++j)
    {
      if (cfg.enforce_same_color && lights[i].color != lights[j].color)
        continue;

      const float len_i = lengthOf(lights[i]);
      const float len_j = lengthOf(lights[j]);
      const float Lavg = 0.5f * (len_i + len_j);

      if (Lavg < cfg.min_avg_length)
        continue;

      const cv::Point2f ci = centerOf(lights[i]);
      const cv::Point2f cj = centerOf(lights[j]);
      const float dx = std::fabs(ci.x - cj.x);
      const float dy = std::fabs(ci.y - cj.y);

      const float horiz_deg = std::atan2(dy, dx) * 180.0f / CV_PI;
      if (horiz_deg > cfg.max_horizontal_angle_deg)
        continue;

      cv::Point2f dA, nA, dB, nB;
      float hLA, wA, hLB, wB;
      axisDirs(lights[i], dA, nA, hLA, wA);
      axisDirs(lights[j], dB, nB, hLB, wB);

      const cv::Point2f vij = cj - ci;
      const float vijL = std::max(1e-3f, std::hypot(vij.x, vij.y));
      const cv::Point2f uij = vij * (1.0f / vijL);

      const float cosA = std::fabs(dA.x * uij.x + dA.y * uij.y);
      const float cosB = std::fabs(dB.x * uij.x + dB.y * uij.y);
      if (cosA > cfg.max_axis_alignment_cos || cosB > cfg.max_axis_alignment_cos)
        continue;

      const float dotAB = dA.x * dB.x + dA.y * dB.y;
      const float alignDeg = std::acos(std::clamp(dotAB, -1.f, 1.f)) * 180.0f / CV_PI;
      if (alignDeg > cfg.max_parallel_angle_deg)
        continue;

      if (dx < cfg.min_dx_factor * Lavg || dx > cfg.max_dx_factor * Lavg)
        continue;
      if (dy > cfg.max_dy_factor * Lavg)
        continue;

      const float angDiff = std::fabs(lights[i].tilt_angle - lights[j].tilt_angle);
      if (angDiff > cfg.max_angle_diff_deg)
        continue;

      const float lenRatio = std::max(len_i, len_j) / std::max(1.f, std::min(len_i, len_j));
      if (lenRatio > cfg.max_length_ratio)
        continue;

      const float score =
          cfg.score_angle_weight * angDiff +
          cfg.score_length_weight * std::fabs(std::log(len_i / len_j)) +
          cfg.score_dy_weight * (dy / Lavg) +
          cfg.score_dx_weight * std::fabs(dx / Lavg - cfg.preferred_dx_factor);

      candidates.push_back({i, j, score});
    }
  }

  std::sort(candidates.begin(), candidates.end(), [](const Candidate &a, const Candidate &b)
            { return a.score < b.score; });

  std::vector<char> used(count, 0);
  std::vector<LightPair> pairs;
  for (const auto &cand : candidates)
  {
    if (!used[cand.left] && !used[cand.right])
    {
      used[cand.left] = used[cand.right] = 1;
      pairs.push_back({cand.left, cand.right});
    }
  }
  return pairs;
}
