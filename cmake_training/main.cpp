// main.cpp
#include <opencv2/opencv.hpp>
#include <eigen3/Eigen/Dense>
#include <iostream>

int main() {
    // Create a synthetic grayscale image (200x200) using OpenCV
    cv::Mat syntheticImage(200, 200, CV_8UC1);
    for (int i = 0; i < syntheticImage.rows; ++i) {
        for (int j = 0; j < syntheticImage.cols; ++j) {
            syntheticImage.at<uchar>(i, j) = static_cast<uchar>((i + j) % 256); // Simple gradient pattern
        }
    }

    // Display the synthetic image
    cv::imshow("Synthetic Image", syntheticImage);
    cv::waitKey(0);

    // Convert the synthetic image to Eigen matrix for further processing
    Eigen::MatrixXf eigenMatrix(syntheticImage.rows, syntheticImage.cols);
    for (int i = 0; i < syntheticImage.rows; ++i) {
        for (int j = 0; j < syntheticImage.cols; ++j) {
            eigenMatrix(i, j) = static_cast<float>(syntheticImage.at<uchar>(i, j));
        }
    }

    // Perform a simple matrix operation using Eigen (e.g., scaling the matrix)
    Eigen::MatrixXf scaledMatrix = 0.5 * eigenMatrix;

    // Output some matrix values
    std::cout << "Original Matrix (Top-left 3x3 block):\n" << eigenMatrix.topLeftCorner(3, 3) << std::endl;
    std::cout << "Scaled Matrix (Top-left 3x3 block):\n" << scaledMatrix.topLeftCorner(3, 3) << std::endl;
    std::cout <<"congratulation , you have successfully installed OpenCV and Eigen" << std::endl;
    cv::imshow("Synthetic Image", syntheticImage);
    cv::waitKey(0);
    return 0;
}
