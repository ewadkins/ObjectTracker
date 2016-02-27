/*
 * Main.cpp
 *
 *  Created on: Feb 26, 2016
 *      Author: ericwadkins
 */

#include "Main.h"

int main() {

	std::cout << "Hello World!" << std::endl;

	cv::VideoCapture cap(0);

	if (!cap.isOpened()) {
		std::cout << "Failed to open default camera" << std::endl;
		exit(-1);
	}

	cv::Mat edges;
	cv::namedWindow("Control", 1);

	int iLowH = 0;
	int iHighH = 179;
	int iLowS = 0;
	int iHighS = 125;
	int iLowV = 80;
	int iHighV = 255;

	bool displayOriginal = true;

	cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Control", &iHighH, 179);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);

	while(true) {
		/*cv::Mat frame;
		cap >> frame;
		cv::imshow("Object Tracker", frame);
		if(cv::waitKey(30) >= 0) {
			break;
		}

		if(cv::waitKey(30) >= 0) {
			break;
		}*/

		cv::Mat imgOriginal;
		bool bSuccess = cap.read(imgOriginal); // read a new frame from video

		if (!bSuccess) {
			std::cout << "Cannot read a frame from video stream" << std::endl;
			break;
		}

		cv::Mat imgHSV;
		cvtColor(imgOriginal, imgHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
		cv::Mat imgThresholded;
		inRange(imgHSV, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

		//morphological opening (remove small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );
		dilate( imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );

		//morphological closing (fill small holes in the foreground)
		dilate( imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );
		erode(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );

		imshow("Object Tracker", displayOriginal ? imgOriginal : imgThresholded); //show the thresholded image

		int key = cv::waitKey(1);
		if (key == 32) {
			if (displayOriginal) {
				displayOriginal = false;
			}
			else {
				displayOriginal = true;
			}
		}
		else if (key == 27) {
			break;
		}
	}

}
