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


	int lastLowH = 0;
	int lastHighH = 179;
	int lastLowS = 0;
	int lastHighS = 255;
	int lastLowV = 0;
	int lastHighV = 255;

	cv::Vec3b lastInternalAverage = cv::Vec3b(0, 0, 0);
	cv::Vec3b lastExternalAverage = cv::Vec3b(255, 255, 255);

	// Initial
//	int lowH = 0;
//	int highH = 179;
//	int lowS = 0;
//	int highS = 125;
//	int lowV = 0;
//	int highV = 255;

	// My face
//	int lowH = 0;
//	int highH = 25;
//	int lowS = 85;
//	int highS = 135;
//	int lowV = 100;
//	int highV = 255;

	// Red button
//	int lowH = 0;
//	int highH = 12;
//	int lowS = 155;
//	int highS = 225;
//	int lowV = 115;
//	int highV = 255;

	// Red ball
//	int lowH = 160;
//	int highH = 179;
//	int lowS = 85;
//	int highS = 190;
//	int lowV = 135;
//	int highV = 250;

	// Green ball
//	int lowH = 35;
//	int highH = 80;
//	int lowS = 50;
//	int highS = 175;
//	int lowV = 20;
//	int highV = 210;

	// Dylan's beard
//	int lowH = 85;
//	int highH = 120;
//	int lowS = 75;
//	int highS = 235;
//	int lowV = 0;
//	int highV = 50;

	// Peanutbutter lid
	int lowH = 95;
	int highH = 110;
	int lowS = 90;
	int highS = 220;
	int lowV = 150;
	int highV = 225;


	bool displayOriginal = true;

	/*
	cvCreateTrackbar("LowH", "Control", &lowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Control", &highH, 179);

	cvCreateTrackbar("LowS", "Control", &lowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &highS, 255);

	cvCreateTrackbar("LowV", "Control", &lowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &highV, 255);
	*/

	int state = SEARCHING;
	int trainingState = 0;
	int initialStep = 2;
	int step = initialStep;

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

		cv::Mat imgThresholded = thresholdedImg(imgOriginal, cv::Scalar(lowH, lowS, lowV), cv::Scalar(highH, highS, highV), true, false);

		cv::Mat imgCircle = cv::Mat::zeros( imgOriginal.size(), CV_8UC3 );

		if (state == TRAINING) {
			double progress = (double) lastInternalAverage[0];
			//double progress = (double) trainingState * 255 / 12;
			cv::circle(imgCircle, lastCenter, lastRadius, cv::Scalar(0, progress, 255 - progress), 3, 8);
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
		         cv::circle(imgCircle, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
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
			imgThresholded = thresholdedImg(imgTraining, cv::Scalar(lowH, lowS, lowV), cv::Scalar(highH, highS, highV), false, true);
			imgDisplay = imgThresholded;
			//imgDisplay = imgTraining;
			imgDisplay += imgCircle;
			break;
		case (TRACKING):
			imgDisplay = displayOriginal ? imgOriginal + imgCircle + imgLines : imgThresholded;
			break;
		}

		std::ostringstream os;

		os << "H: (" << lowH << " - " << highH << ")";
		cv::putText(imgDisplay, os.str(), cv::Point(40, 50),
		    cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(255, 255, 255), 2, 8);

		os.str("");
		os << "S: (" << lowS << " - " << highS << ")";
		cv::putText(imgDisplay, os.str(), cv::Point(40, 70),
		    cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(255, 255, 255), 2, 8);

		os.str("");
		os << "V: (" << lowV << " - " << highV << ")";
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
			        	internalAverage[0] += color[0];
			        	internalAverage[1] += color[1];
			        	internalAverage[2] += color[2];
			        	internalCount++;
			        }
			        else {
			        	externalAverage[0] += color[0];
			        	externalAverage[1] += color[1];
			        	externalAverage[2] += color[2];
			        	externalCount++;
			        }
			    }
			}
			internalAverage[0] /= internalCount;
			internalAverage[1] /= internalCount;
			internalAverage[2] /= internalCount;
			externalAverage[0] /= externalCount;
			externalAverage[1] /= externalCount;
			externalAverage[2] /= externalCount;

			cv::Vec3b internal = cv::Vec3b(internalAverage[0], internalAverage[1], internalAverage[2]);
			cv::Vec3b external = cv::Vec3b(externalAverage[0], externalAverage[1], externalAverage[2]);

			double internalAbsoluteChange = ((double) internal[0] - lastInternalAverage[0] + internal[1] - lastInternalAverage[1] + internal[2] - lastInternalAverage[2]) / 3;
			double externalAbsoluteChange = ((double) external[0] - lastExternalAverage[0] + external[1] - lastExternalAverage[1] + external[2] - lastExternalAverage[2]) / 3;
			//double internalPercentChange = (double) (internal[0] - lastInternalAverage[0]) / internal[0];
			//double externalPercentChange = (double) (external[0] - lastExternalAverage[0]) / external[0];

			if (lastLowH != lowH || lastHighH != highH || lastLowS != lowS || lastHighS != highS || lastLowV != lowV || lastHighV != highV) {
				/*std::cout << "Training state: " << trainingState << std::endl;
				//std::cout << "Internal percent change: " << internalPercentChange << std::endl;
				//std::cout << "External percent change: " << externalPercentChange << std::endl;
				std::cout << "Internal absolute change: " << internalAbsoluteChange << std::endl;
				std::cout << "External absolute change: " << externalAbsoluteChange << std::endl;
				std::cout << "New internal: " << internal << ", Old internal: " << lastInternalAverage << std::endl;
				std::cout << "New external: " << external << ", Old external: " << lastExternalAverage << std::endl;*/
			}
			if (trainingState > 12) {
				state = TRACKING;
			}
			else if ((internalAbsoluteChange >= 0 && externalAbsoluteChange >= 0
					&& internalAbsoluteChange >= externalAbsoluteChange)
					|| (internalAbsoluteChange >= 0 && externalAbsoluteChange <= 0)
					|| (internalAbsoluteChange <= 0 && externalAbsoluteChange <= 0
							&& internalAbsoluteChange >= externalAbsoluteChange)) {
				lastInternalAverage = internal;
				lastExternalAverage = external;
				lastLowH = lowH;
				lastHighH = highH;
				lastLowS = lowS;
				lastHighS = highS;
				lastLowV = lowV;
				lastHighV = highV;

				if (trainingState == 1) {
					lowH = std::max(0, lowH - step);
				}
				else if (trainingState == 2) {
					highH = std::min(179, highH + step);
				}
				else if (trainingState == 3) {
					lowS = std::max(0, lowS - step);
				}
				else if (trainingState == 4) {
					highS = std::min(255, highS + step);
				}
				else if (trainingState == 5) {
					lowV = std::max(0, lowV - step);
				}
				else if (trainingState == 6) {
					highV = std::min(255, highV + step);
				}
				else if (trainingState == 7) {
					lowH = std::min(179, lowH + step);
				}
				else if (trainingState == 8) {
					highH = std::max(0, highH - step);
				}
				else if (trainingState == 9) {
					lowS = std::min(255, lowS + step);
				}
				else if (trainingState == 10) {
					highS = std::max(0, highS - step);
				}
				else if (trainingState == 11) {
					lowV = std::min(255, lowV + step);
				}
				else if (trainingState == 12) {
					highV = std::max(0, highV - step);
				}

				if (lastLowH == lowH && lastHighH == highH && lastLowS == lowS && lastHighS == highS && lastLowV == lowV && lastHighV == highV) {
					trainingState += 1;
				}
			}
			else if (step < 6) {
				lowH = lastLowH;
				highH = lastHighH;
				lowS = lastLowS;
				highS = lastHighS;
				lowV = lastLowV;
				highV = lastHighV;
				step++;
			}
			else {
				step = initialStep;
				trainingState += 1;
			}

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
				trainingState = 1;

				lowH = 0;
				highH = 179;
				lowS = 0;
				highS = 255;
				lowV = 0;
				highV = 255;

				lastLowH = 0;
				lastHighH = 179;
				lastLowS = 0;
				lastHighS = 255;
				lastLowV = 0;
				lastHighV = 255;

				lastInternalAverage = cv::Vec3b(0, 0, 0);
				lastExternalAverage = cv::Vec3b(255, 255, 255);

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
				cv::Vec3b hsv = RGBtoHSV(average[0], average[1], average[2]);
				int rangeH = 20;
				int rangeS = 20;
				int rangeV = 20;
				lowH = std::max(0, hsv[0] - rangeH);
				highH = std::min(179, hsv[0] + rangeH);
				lowS = std::max(0, hsv[1] - rangeS);
				highS = std::min(255, hsv[1] + rangeS);
				lowV = std::max(0, hsv[2] - rangeV);
				highV = std::min(255, hsv[2] + rangeV);

				cvCreateTrackbar("LowH", "Control", &lowH, 179); //Hue (0 - 179)
				cvCreateTrackbar("HighH", "Control", &highH, 179);

				cvCreateTrackbar("LowS", "Control", &lowS, 255); //Saturation (0 - 255)
				cvCreateTrackbar("HighS", "Control", &highS, 255);

				cvCreateTrackbar("LowV", "Control", &lowV, 255); //Value (0 - 255)
				cvCreateTrackbar("HighV", "Control", &highV, 255);

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

cv::Mat ObjectTracker::thresholdedImg(cv::Mat original, cv::Scalar lowHSV, cv::Scalar highHSV, bool removeNoise, bool convertBackToRGB) {
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

	cv::GaussianBlur(imgThresholded, imgThresholded, cv::Size(51, 51), 0);

	if (convertBackToRGB) {
		cvtColor(imgThresholded, imgThresholded, cv::COLOR_GRAY2RGB);
	}

	/*for (int x = 0; x < imgThresholded.rows; x++) {
	    for (int y = 0; y < imgThresholded.cols; y++) {
	        cv::Vec3b &color = imgThresholded.at<cv::Vec3b>(x, y);
	        if (color[0] > 127) {
	        	color = cv::Vec3b(255, 255, 255);
	        }
	        else {
	        	color = cv::Vec3b(255, 255, 255);
	        }
	    }
	}*/

	return imgThresholded;
}

cv::Vec3b ObjectTracker::RGBtoHSV(float r, float g, float b) {
	float h, s, v, min, max, delta;
	r /= 255;
	g /= 255;
	b /= 255;
	min = std::min(std::min(r, g), b);
	max = std::max(std::max(r, g), b);
	v = max;
	delta = max - min;
	if (max != 0) {
		s = delta / max;
	}
	else {
		s = 0;
		h = 0;
		return cv::Vec3b(h, s, v * 255);
	}
	if (r == max) {
		h = (g - b) / delta;
	}
	else if (g == max) {
		h = 2 + (b - r) / delta;
	}
	else {
		h = 4 + (r - g) / delta;
	}
	h *= 60;
	if (h < 0) {
		h += 360;
	}
	return cv::Vec3b(h / 2, s * 255, v * 255);
}

