/*
 * ObjectTracker.h
 *
 *  Created on: Feb 26, 2016
 *      Author: ericwadkins
 */

#ifndef OBJECTTRACKER_H_
#define OBJECTTRACKER_H_

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <stdio.h>
#include <math.h>
#include <string>
#include <sstream>

class ObjectTracker {
public:
	static void run();
private:
	static cv::Mat thresholdedImg(cv::Mat original, cv::Scalar lowHSV, cv::Scalar highHSV, bool removeNoise, bool convertBackToRGB);
	static cv::Vec3b RGBtoHSV(float r, float g, float b);
};

#endif /* OBJECTTRACKER_H_ */
