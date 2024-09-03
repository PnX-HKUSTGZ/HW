//  Phx Liang Feng 2024/8/20
#include <opencv2/opencv.hpp>

#include<iostream>

#include"detector.hpp"

int main(){
    // std::cout << "current file_path is: " << __FILE__ << std::endl;

    const int bin_tres = 130;
    const int detect_color = 1;
    // 分别为二值化阈值与Detector要检测的颜色

    Detector detector_(bin_tres,detect_color);

    std::string picName = "test.jpg";

    cv::Mat ori_image = cv::imread(picName);
    
    // image = cv::imread("test.jpg");

    // 检查图像是否读取成功，如果图像文件位置与可执行文件不在同一目录下，尝试在可执行文件的上级目录进行读取
    if (ori_image.empty()) {
        std::cout << "Failed to read image. Help you check outside the build folder\n";
        ori_image = cv::imread("../" + picName);
        if (ori_image.empty()){
            std::cerr << "Failed to read image. Check your path or filename" << std::endl;
            return 1;
        }
    }

    // 显示图像
    // cv::imshow("Input", ori_image);
    // cv::waitKey(0); // 按键按下再继续

    cv::cvtColor(ori_image, ori_image, cv::COLOR_BGR2RGB);
    // 默认的，opencv读取图片以bgr为通道排布，需要转化成rgb。如果是相机读入或许不用？

    // detector_.binary_img = detector_.preprocessImage(ori_image);
    // detector_.findLights(ori_image,detector_.binary_img)
    detector_.WholeProcess(ori_image);

    cv::cvtColor(ori_image, ori_image, cv::COLOR_RGB2BGR);
    cv::imshow("Result", ori_image);
    cv::waitKey(0);

    std::cout << "success!" << std::endl;
    return 0;
}
