#ifndef LIGHT_DETECTION_HPP_
#define LIGHT_DETECTION_HPP_


#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>

const int RED = 0;
const int BLUE = 1;

class Detector{ 
public:
  int binary_thres =200;
  int detect_color = 1;  
Detector() : binary_thres(200), detect_color(1) {}
    // 图像预处理函数
  // struct DetectorSettings
  // {
  //   // 灯条的宽高比（灯条的宽度 / 灯条的高度）
  //   double min_ratio; // 灯条的最小宽高比
  //   double max_ratio; // 灯条的最大宽高比
  //   //垂直角度
  //   double max_angle;
  // };
  cv::Mat preprocessImage(const cv::Mat & rgb_img);
//---------------------------------------------
Detector(const int & bin_thres, const int & color);

//--------------------------------------------------------------------------------------------------------------  
  struct Light : public cv::RotatedRect
{
  Light() = default;
explicit Light(cv::RotatedRect box) // explicit 防止隐式转换
  : cv::RotatedRect(box)
  {
    // 将旋转矩形的四个顶点转换为灯条的两个顶点
    cv::Point2f p[4];
    box.points(p);
    // 按照 y 坐标从小到大排序，排序规则为 lambda 表达式，即按照 y 坐标从小到大排序
    std::sort(p, p + 4, [](const cv::Point2f & a, const cv::Point2f & b) {return a.y < b.y;});
    // 计算灯条的顶点和底点
    top = (p[0] + p[1]) / 2;
    bottom = (p[2] + p[3]) / 2;

    // 计算灯条的长度和宽度
    length = cv::norm(top - bottom);
    width = cv::norm(p[0] - p[1]);

    // 计算灯条的倾斜角
    tilt_angle = std::atan2(std::abs(top.x - bottom.x), std::abs(top.y - bottom.y));
    tilt_angle = tilt_angle / CV_PI * 180;
  }
  //--------------------------------------
    // 灯条的宽高比（灯条的宽度 / 灯条的高度）
    double min_ratio=0.1; // 灯条的最小宽高比
    double max_ratio=0.4; // 灯条的最大宽高比
    //垂直角度
    double max_angle=40;
    //-------------------------------------
  // 灯条的颜色
  int color;
  // 灯条的顶点、底点坐标
  cv::Point2f top, bottom;
  // 灯条的长度
  double length;
  // 灯条的宽度
  double width;
  // 灯条的倾斜角
  float tilt_angle;
};
//--------------------------------------------------
std::vector<Light> findLights(const cv::Mat & rgb_img, const cv::Mat & binary_img);

};
#endif
