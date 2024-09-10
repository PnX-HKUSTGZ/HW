// OpenCV
#include <opencv2/highgui.hpp>
#include <opencv4/opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <opencv2/core/types.hpp>
#include "Light_detection/Light_detection.hpp"


Detector::Detector(const int &bin_thres, const int &color)
    : binary_thres(bin_thres), detect_color(color) {}
//---------------------------------------------------------------------------------------------------------------------------------
cv::Mat Detector::preprocessImage(const cv::Mat & rgb_img)
{
  // 检查输入图像的类型
  if (rgb_img.type() != CV_8UC3 && rgb_img.type() != CV_8UC4) {
  throw std::invalid_argument("Input image must be either CV_8UC3 or CV_8UC4.");
  }       
  // 将图像转换为灰度图像
  cv::Mat gray_img;
  cv::cvtColor(rgb_img, gray_img, cv::COLOR_BGR2GRAY);
  // 对图像进行二值化
  cv::Mat binary_img;
  cv::threshold(gray_img, binary_img, binary_thres, 255, cv::THRESH_BINARY);
  // 返回二值化图像
  return binary_img;
}
//void findLights(const cv::Mat & rbg_img, const cv::Mat & binary_img); 
//-----------------------------------------------------------------------------------------
bool isLight(const Detector::Light & light)
{
  // 灯条的短边长 / 长边长
  float ratio = light.width / light.length;
  // 判断灯条的短边长 / 长边长是否在范围内
  bool ratio_ok = light.min_ratio < ratio && ratio < light.max_ratio;
  // 判断灯条的倾斜角度是否在范围内
  bool angle_ok = light.tilt_angle < light.max_angle;
  // 判断是否为灯条
  bool is_light = ratio_ok && angle_ok;

  // 返回是否为灯条
  return is_light;
}

//------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------
    // cv::RotatedRect rect;  // 使用旋转矩形来表示灯条
    // cv::Scalar color;      // 灯条颜色

  std::vector<Detector::Light> Detector::findLights(const cv::Mat & rgb_img, const cv::Mat & binary_img)
  {
    using std::vector;
        // 确认二值化图像是单通道
    CV_Assert(binary_img.type() == CV_8UC1);  
    // 用来存储轮廓
    vector<vector<cv::Point>> contours;
    // 用来存储轮廓的层次结构 
    //vector<cv::Vec4i> hierarchy;  
    cv::Mat hierarchy;
    //------------------------------------------------------------------------------------------------------------------ 
    // 寻找轮廓
    //-------------------------------------------------------------------------------------------------------------------------------------
    cv::findContours(binary_img, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    //------------------------------------------------------------------------------------------------------------------
    //std::cout << "找到轮廓数量: " << contours.size() << std::endl;
    // 创建灯条容器
    vector<Detector::Light> lights;
  // 遍历轮廓，寻找灯条
  for (const auto & contour : contours) {
  //std::cout << "当前轮廓点数: " << contour.size() << std::endl;
    // 轮廓点数小于 5 个，不是灯条
    //if (contour.size() < 5) {continue;}
    // 寻找轮廓的最小外接矩形
    auto r_rect = cv::minAreaRect(contour);
    // 将旋转矩形转化为灯条
    auto light = Detector::Light(r_rect);
    // 判断是否为灯条
    if (isLight(light)) {
      // 如果是灯条，计算灯条的颜色
      auto rect = light.boundingRect();
      // 防止越界，检查矩形是否在图像内
      if (0 <= rect.x && 0 <= rect.width && rect.x + rect.width <= rgb_img.cols &&
          0 <= rect.y && 0 <= rect.height && rect.y + rect.height <= rgb_img.rows)
      {
        // 如果矩形在图像内，计算矩形内红色和蓝色像素的和
        int sum_r = 0, sum_b = 0;
        // 获取矩形内的 ROI ，ROI即 Region of Interest，感兴趣区域，用来提取矩形内的像素
        auto roi = rgb_img(rect);
        // 遍历 ROI
        for (int i = 0; i < roi.rows; i++) {
          for (int j = 0; j < roi.cols; j++) {
            // 判断像素是否在轮廓内，如果在轮廓内，计算红色和蓝色像素的和
            if (cv::pointPolygonTest(contour, cv::Point2f(j + rect.x, i + rect.y), false) >= 0) {
              // 累加红色和蓝色像素的和
              sum_r += roi.at<cv::Vec3b>(i, j)[2];
              sum_b += roi.at<cv::Vec3b>(i, j)[0];
            }
          }
        }              
        // 红色像素的和大于蓝色像素的和，灯条为红色
        light.color = sum_r > sum_b ? RED : BLUE;
        // 将灯条存入灯条容器
        lights.emplace_back(light);
      } 
    } 
  }

    //返回灯条容器
 return lights;
  }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int main() {
    // 加载图像
    cv::Mat bgr_img = cv::imread("images/image.png", cv::IMREAD_COLOR);
    if (bgr_img.empty()) {
        std::cerr << "Could not read the image: " << "images/image.png" << std::endl;
        return -1;
    }
    // std::cout << "Image type: " << bgr_img.type() << std::endl;
    // std::cout << "Image size: " << bgr_img.size() << std::endl;
    // std::cout << "Number of channels: " << bgr_img.channels() << std::endl;
    // std::cout << "Image depth: " << bgr_img.depth() << std::endl;
        // 输出图像的深度
    //int depth = bgr_img.depth();
    //std::cout << "Image depth: " << depth << std::endl;
    // 创建 Detector 对象
    Detector detector;
    // 预处理图像
    cv::Mat binary_img = detector.preprocessImage(bgr_img);
    // 找到灯条
    std::vector<Detector::Light> lights  = detector.findLights(bgr_img, binary_img);
      std::cout << "Number of detected lights:" <<lights.size() << std::endl;
    // 显示二值化图像
    cv::imshow("Binary Image", binary_img);
    cv::waitKey(0);
    // 在原图上绘制检测到的灯条
    for (const auto& light : lights) {
    cv::Point topLeft(light.top.x - 10, light.top.y - 20); // 根据实际情况计算
    cv::Point bottomRight(light.bottom.x + 10, light.bottom.y + 20); // 根据实际情况计算
    cv::rectangle(bgr_img, topLeft, bottomRight, cv::Scalar(0, 255, 0), 2);    // 使用绿色绘制矩形框
    }
      // 显示结果图像
    cv::imshow("Detected Lights", bgr_img);
    // 保存处理后的图像
    cv::imwrite("images/binary_image.png", binary_img);
    cv::imwrite("images/detected_lights_image.png", bgr_img);
    // 等待用户按下任意键
    cv::waitKey(0);

    return 0;
}