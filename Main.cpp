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
	cv::namedWindow("Control", CV_WINDOW_AUTOSIZE);

	// Initial
//	int iLowH = 0;
//	int iHighH = 179;
//	int iLowS = 0;
//	int iHighS = 125;
//	int iLowV = 0;
//	int iHighV = 255;

	// My face
//	int iLowH = 0;
//	int iHighH = 25;
//	int iLowS = 85;
//	int iHighS = 135;
//	int iLowV = 100;
//	int iHighV = 255;

	// Red button
//	int iLowH = 0;
//	int iHighH = 12;
//	int iLowS = 155;
//	int iHighS = 225;
//	int iLowV = 115;
//	int iHighV = 255;

	// Red ball
	int iLowH = 160;
	int iHighH = 179;
	int iLowS = 85;
	int iHighS = 190;
	int iLowV = 135;
	int iHighV = 250;


	bool displayOriginal = true;

	cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Control", &iHighH, 179);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);

	int iLastX = -1;
	int iLastY = -1;

	//Capture a temporary image from the camera
	cv::Mat imgTmp;
	cap.read(imgTmp);

	//Create a black image with the size as the camera output
	cv::Mat imgLines = cv::Mat::zeros( imgTmp.size(), CV_8UC3 );;

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
		cv::flip(imgOriginal, imgOriginal, 1);

		if (!bSuccess) {
			std::cout << "Cannot read a frame from video stream" << std::endl;
			break;
		}

		cv::Mat imgHSV;
		cvtColor(imgOriginal, imgHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
		cv::Mat imgThresholded;
		inRange(imgHSV, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

		//morphological opening (remove small objects from the foreground)
		//erode(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );
		//dilate( imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );

		//morphological closing (fill small holes in the foreground)
		//dilate( imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );
		//erode(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );

		//Calculate the moments of the thresholded image
		cv::Moments oMoments = moments(imgThresholded);

		double dM01 = oMoments.m01;
		double dM10 = oMoments.m10;
		double dArea = oMoments.m00;

		// if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero
		if (dArea > 10000) {
			//calculate the position of the ball
			int posX = dM10 / dArea;
			int posY = dM01 / dArea;

			if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0) {
				//Draw a red line from the previous point to the current point
				cv::line(imgLines, cv::Point(posX, posY), cv::Point(iLastX, iLastY), cv::Scalar(0,0,255), 2);
			}

			iLastX = posX;
			iLastY = posY;
		}

		cv::Mat imgDisplay = (displayOriginal ? imgOriginal + imgLines : imgThresholded);
		imshow("Object Tracker", imgDisplay); //show the thresholded image

		std::cout << "H: (" << iLowH << " - " << iHighH << ")"
				<< "\tS: (" << iLowS << " - " << iHighS << ")"
				<< "\tV: (" << iLowV << " - " << iHighV << ")" << std::endl;

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
