/*
 * ObjectTracker.cpp
 *
 *  Created on: Feb 26, 2016
 *      Author: ericwadkins
 */

#include "ObjectTracker.h"

const static int SEARCHING = 0;
const static int TRAINING = 1;
const static int TRACKING = 2;

void ObjectTracker::run() {

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
//	int iLowH = 160;
//	int iHighH = 179;
//	int iLowS = 85;
//	int iHighS = 190;
//	int iLowV = 135;
//	int iHighV = 250;

	// Green ball
	int iLowH = 35;
	int iHighH = 80;
	int iLowS = 50;
	int iHighS = 175;
	int iLowV = 20;
	int iHighV = 210;


	bool displayOriginal = true;

	cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Control", &iHighH, 179);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);

	int state = TRACKING;

	cv::Mat imgOriginal;
	cv::Mat imgTraining;

	int iLastX = -1;
	int iLastY = -1;
	cv::Point lastCenter = cv::Point(0, 0);
	int lastRadius = 0;

	//Capture a temporary image from the camera
	cv::Mat imgTmp;
	cap.read(imgTmp);

	//Create a black image with the size as the camera output
	cv::Mat imgLines = cv::Mat::zeros(imgTmp.size(), CV_8UC3);


	int test = 0;


	while(true) {

		if (state != TRAINING) {
			bool success = cap.read(imgOriginal); // read a new frame from video
			cv::flip(imgOriginal, imgOriginal, 1);
			if (!success) {
				std::cout << "Cannot read a frame from video stream" << std::endl;
				break;
			}
		}

		cv::Mat imgGray;
		cvtColor(imgOriginal, imgGray, CV_BGR2GRAY);

		cv::Mat imgThresholded = thresholdedImg(imgOriginal, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), true);

		cv::Mat imgCircle = cv::Mat::zeros( imgOriginal.size(), CV_8UC3 );

		if (state == TRAINING) {
			cv::circle(imgCircle, cv::Point(lastCenter.x, lastCenter.y), lastRadius, cv::Scalar(255, 0, 0), 3, 8);
		}

		if (state == TRACKING) {
			//Calculate the moments of the thresholded image
			cv::Moments oMoments = moments(imgThresholded);

			double dM01 = oMoments.m01;
			double dM10 = oMoments.m10;
			double dArea = oMoments.m00;

			// if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero
			if (dArea > 100000) {
				//calculate the position of the ball
				int posX = dM10 / dArea;
				int posY = dM01 / dArea;
				cv::circle(imgCircle, cv::Point(posX, posY), 15, cv::Scalar(255, 0, 0), 3, 8);

				if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0) {
					//Draw a red line from the previous point to the current point
					cv::line(imgLines, cv::Point(posX, posY), cv::Point(iLastX, iLastY), cv::Scalar(0,0,255), 2);
				}

				iLastX = posX;
				iLastY = posY;
			}
		}

		if (state == SEARCHING) {
		std::vector<cv::Vec3f> circles;
		    cv::HoughCircles(imgGray, circles, CV_HOUGH_GRADIENT,
		                 2, imgGray.rows * 2, 250, 125, 0, 0);
		    for( size_t i = 0; i < circles.size(); i++ ) {
		         cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		         int radius = cvRound(circles[i][2]);
		         //std::cout << center << std::endl << radius << std::endl;
		         // draw the circle center
		         cv::circle(imgCircle, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
		         // draw the circle outline
		         cv::circle(imgCircle, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);
		         lastCenter = center;
		         lastRadius = radius;
		    }
		}

		cv::Mat imgDisplay;
		cv::Mat temp;
		switch (state) {
		case (SEARCHING):
			imgDisplay = imgOriginal + imgCircle;
			break;
		case (TRAINING):
			imgThresholded = thresholdedImg(imgTraining, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), false);
			imgDisplay = imgThresholded;
			//imgDisplay = imgTraining;
			break;
		case (TRACKING):
			imgDisplay = displayOriginal ? imgOriginal + imgCircle + imgLines : imgThresholded;
			break;
		}

		std::ostringstream os;

		os << "H: (" << iLowH << " - " << iHighH << ")";
		cv::putText(imgDisplay, os.str(), cv::Point(40, 50),
		    cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(255, 255, 255), 2, 8);

		os.str("");
		os << "S: (" << iLowS << " - " << iHighS << ")";
		cv::putText(imgDisplay, os.str(), cv::Point(40, 70),
		    cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(255, 255, 255), 2, 8);

		os.str("");
		os << "V: (" << iLowV << " - " << iHighV << ")";
		cv::putText(imgDisplay, os.str(), cv::Point(40, 90),
		    cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(255, 255, 255), 2, 8);


		imshow("Object Tracker", imgDisplay);

		if (state == TRAINING) {
			std::vector<double> externalAverage = std::vector<double>(3);
			std::vector<double> internalAverage = std::vector<double>(3);
			int internalCount = 0;
			int externalCount = 0;
			for (int x = 0; x < imgThresholded.rows; x++) {
			    for (int y = 0; y < imgThresholded.cols; y++) {
			        double dist = sqrt((y - lastCenter.x) * (y - lastCenter.x) + (x - lastCenter.y) * (x - lastCenter.y));
		        	cv::Vec3b color = imgThresholded.at<cv::Vec3b>(x, y);
			        if (dist < lastRadius) {
			        	uchar b = imgThresholded.data[imgThresholded.channels()*(imgThresholded.cols*y + x) + 0];
			        	uchar g = imgThresholded.data[imgThresholded.channels()*(imgThresholded.cols*y + x) + 1];
			        	uchar r = imgThresholded.data[imgThresholded.channels()*(imgThresholded.cols*y + x) + 2];
			        	//std::cout << b << " " << g << " " << r << std::endl;
			        	internalAverage[0] += r;
			        	internalAverage[1] += g;
			        	internalAverage[2] += b;
			        	internalCount++;
			        	//imgTraining.at<cv::Vec3b>(x, y) = cv::Vec3b((test) % 255, (test * 2) % 255, (test * 3) % 255);
						//test += 1;
			        }
			        else {
			        	externalAverage[0] += color[0];
			        	externalAverage[1] += color[1];
			        	externalAverage[2] += color[2];
			        	externalCount++;
			        }
			    }
			}
			std::cout << "Internal: " << internalAverage[0] << ", External: " << externalAverage[0] << std::endl;
			internalAverage[0] /= internalCount;
			internalAverage[1] /= internalCount;
			internalAverage[2] /= internalCount;
			externalAverage[0] /= externalCount;
			externalAverage[1] /= externalCount;
			externalAverage[2] /= externalCount;

			cv::Vec3b internal = cv::Vec3b(internalAverage[0], internalAverage[1], internalAverage[2]);
			cv::Vec3b external = cv::Vec3b(externalAverage[0], externalAverage[1], externalAverage[2]);

			std::cout << "Internal: " << internal << ", External: " << external << std::endl;
		}

		int key = cv::waitKey(1);
		if (key == 32) { // space
			if (displayOriginal) {
				displayOriginal = false;
			}
			else {
				displayOriginal = true;
			}
		}
		else if (key == 116) { // t
			if (state == SEARCHING) {
				state = TRAINING;

				std::vector<double> average = std::vector<double>(3);
				int count = 0;
				for (int x = 0; x < imgOriginal.rows; x++) {
				    for (int y = 0; y < imgOriginal.cols; y++) {
				        double dist = sqrt((y - lastCenter.x) * (y - lastCenter.x) + (x - lastCenter.y) * (x - lastCenter.y));
				        if (dist < lastRadius) {
				        	cv::Vec3b color = imgOriginal.at<cv::Vec3b>(x, y);
				        	average[0] += color[0];
				        	average[1] += color[1];
				        	average[2] += color[2];
				        	count++;
				        }
				    }
				}
				average[0] /= count;
				average[1] /= count;
				average[2] /= count;

				imgTraining = cv::Mat(imgOriginal);
				/*for (int x = 0; x < imgTraining.rows; x++) {
				    for (int y = 0; y < imgTraining.cols; y++) {
				        double dist = sqrt((y - lastCenter.x) * (y - lastCenter.x) + (x - lastCenter.y) * (x - lastCenter.y));
				        if (dist < lastRadius) {
				        	imgTraining.at<cv::Vec3b>(x, y) = cv::Vec3b(average[0], average[1], average[2]);
				        }
				    }
				}*/
			}
			else {
				state = SEARCHING;
			}
		}
		else if (key == 99) { // c
			imgLines = cv::Mat::zeros(imgTmp.size(), CV_8UC3);
		}
		else if (key == 27) { // esc
			break;
		}
	}

}

cv::Mat ObjectTracker::thresholdedImg(cv::Mat original, cv::Scalar lowHSV, cv::Scalar highHSV, bool removeNoise) {
	cv::Mat imgHSV;
	cvtColor(original, imgHSV, cv::COLOR_BGR2HSV);

	cv::Mat imgThresholded;
	inRange(imgHSV, lowHSV, highHSV, imgThresholded); //Threshold the image

	if (removeNoise) {
		//morphological opening (remove small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );
		dilate( imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );

		//morphological closing (fill small holes in the foreground)
		dilate( imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );
		erode(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );
	}

	return imgThresholded;
}
