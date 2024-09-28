/*
if (enemyColor == ENEMY_RED)
//假设敌方为红色
{        
        cv::threshold(_graySrc, _graySrc, _para.grayThreshold_RED, 255, cv::THRESH_BINARY);//灰度二值化
        //对 _graySrc 图像进行阈值化处理，使用指定的红色阈值 _para.grayThreshold_RED。像素的强度大于该阈值的被设为255（白色），其他被设为0（黑色）。
        
        cv::subtract(splitSrc[2], splitSrc[0], _separationSrc);//红蓝通道相减
        //得到图像 _separationSrc，其中显现出了红色强度较大的区域。
        
        cv::subtract(splitSrc[2], splitSrc[1], _separationSrcGreen);//红绿通道相减
        //得到图像 _separationSrcGreen，突出显示了红色较强但绿色强度较低的区域。
        
        cv::threshold(_separationSrc, _separationSrc, _para.separationThreshold_RED, 255, cv::THRESH_BINARY);//红蓝二值化
        //对 _separationSrc 应用阈值化处理，使用指定的阈值 _para.separationThreshold_RED，得到一个二值图像，其中红色强度超过阈值的区域被标为白色。
        
        cv::threshold(_separationSrcGreen, _separationSrcGreen, _para.separationThreshold_GREEN, 255, cv::THRESH_BINARY);//红绿二值化
        //对 _separationSrcGreen 应用阈值化处理，使用指定的阈值 _para.separationThreshold_GREEN，得到一个二值图像，其中红色强度较大但绿色强度较低的区域被突出显示。
        
        cv::dilate(_separationSrc, _separationSrc, Util::structuringElement3());
        cv::dilate(_separationSrcGreen, _separationSrcGreen, Util::structuringElement3());//膨胀
        //对两个二值图像 _separationSrc 和 _separationSrcGreen 执行膨胀操作，增强突出显示的区域。

        _maxColor = _separationSrc & _graySrc & _separationSrcGreen & _separationSrcWhite;//逻辑与获得最终二值化图像
        //将所有二值图像进行逻辑与操作，得到 _maxColor，突出显示出所有条件都满足的区域。
        
        cv::dilate(_maxColor, _maxColor, Util::structuringElement3());//对_maxColor执行膨胀操作，进一步增强突出显示的区域
    }
//最终得到的图像是 _maxColor = _separationSrc & _graySrc & _separationSrcGreen & _separationSrcWhite（是四个二值化得来的）
*/


/*
//如果敌方为蓝色需要加一行防止误识别敌方基地
cv::threshold(splitSrc[2], _purpleSrc, _para.grayThreshold_PURPLE, 255, cv::THRESH_BINARY);
//这段代码对splitSrc[2]（即紫色通道）进行阈值处理，使用指定的紫色阈值 _para.grayThreshold_PURPLE。像素的强度大于该阈值的被设为255（白色），其他被设为0（黑色）。

cv::bitwise_not(_purpleSrc, _purpleSrc); 
//这一行代码对 _purpleSrc 进行位求反操作，将图像中的白色区域变为黑色，黑色区域变为白色。
*/