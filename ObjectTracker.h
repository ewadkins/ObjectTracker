/*
 * ObjectTracker.h
 *
 *  Created on: Feb 26, 2016
 *      Author: ericwadkins
 */

#ifndef OBJECTTRACKER_H_
#define OBJECTTRACKER_H_

//#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <stdio.h>

class ObjectTracker {
public:
	static void run();
};

#endif /* OBJECTTRACKER_H_ */
