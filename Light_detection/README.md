# HW
#
# 注意图像读取部分，如果自行选择图像加载失败可能是图像格式原因，需要添加代码转换图像格式例如：
<code> if (rgb_img.type() == CV_32FC3) {
    std::cout << "Converting 32-bit float image to 8-bit..." << std::endl;
    rgb_img.convertTo(img_8bit, CV_8UC3, 255);
} else {
    std::cerr << "Unsupported image type for conversion! Type: " << rgb_img.type() << std::endl;
}</code>

# 如果图像加载成功但是识别效果较差可能是由于图像亮度过高，可在hpp文件里调整binary_thres阈值
# ChannelSubtraction.cpp里的是通道分离相减的方法示例
