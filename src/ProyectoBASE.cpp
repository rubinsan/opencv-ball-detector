// Final project for the Perception Systems course: Red ball tracker using OpenCV
// Authors:
// 100345968 - Álvaro Victoria Gijón
// 100349094 - Rubén Sánchez Blázquez
// 100345851 - Luis Cambero Piqueras

#include "opencv/cv.hpp"
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	VideoCapture cam(0); // Opens camera video

	if (!cam.isOpened()) 
	{
		cout << "No se ha podido abrir la cámara" << endl;
		return -1;
	}

	vector<vector<Point2f>> lines;

	while (true)
	{
		Mat imgOriginal;

		bool bSuccess = cam.read(imgOriginal); // new frame

		int fps = (int)cam.get(CV_CAP_PROP_FPS); // fps number
		if (!bSuccess) 
			cout << "No se ha podido capturar el frame" << endl;
			break;
		}

		Mat imgHSV, imgFiltered,imgThresholded;;

		medianBlur(imgOriginal, imgFiltered, 5); // noise filtering

		// print into the image
		string fpsStr = "Fps: " + to_string(fps);
		putText(imgOriginal, fpsStr, Point(0, 10), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255), 1, 8, false);

		cvtColor(imgFiltered, imgHSV, COLOR_BGR2HSV); // BGR to HSV conversion

		////////////////////////////////////////////////////////////////////////////////
		/*
		// Attempt to detect a red ball against a red background
		Mat imgFilteredGray, modified_image, edge;
		Mat hue_channel(imgFiltered.size(), CV_8UC1);
		Mat saturation_channel(imgFiltered.size(), CV_8UC1);
		Mat value_channel(imgFiltered.size(), CV_8UC1);

		Mat channels_array[] = { hue_channel,saturation_channel,value_channel };

		split(imgHSV, channels_array);

		int histogram_size = 256;
		bool fondorojo = 0;
		unsigned int acumulado = 0;
		float histogram_range[] = { 0, 256 };
		const float* histogram_ranges[] = { histogram_range };
		Mat hue_histogram;

		calcHist(&hue_channel, 1, 0, Mat(), hue_histogram, 1, &histogram_size, histogram_ranges);

		// Pixel with the highest variation when introducing a red background (red garment), calculated by trial and error
		if ((int)hue_histogram.at<float>(120) < 2000) fondorojo = 1;
		// cout << hue_histogram.at<float>(120) << endl;

		vector<Vec3f> circles;//guarda los circulos que detecta Hough
		cvtColor(imgFiltered, imgFilteredGray, CV_BGR2GRAY);

		// Opening (Remove small objects from the foreground)
		erode(imgFilteredGray, imgFilteredGray, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		dilate(imgFilteredGray, imgFilteredGray, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		// Closing (Remove small holes from the foreground)
		dilate(imgFilteredGray, imgFilteredGray, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
		erode(imgFilteredGray, imgFilteredGray, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
		Canny(imgFilteredGray, edge, 50, 150, 3);

		HoughCircles(edge, circles, HOUGH_GRADIENT, 1, edge.rows / 16, 100, 30, 10, 100);

		for (size_t i = 0; i < circles.size(); i++)
		{
			Vec3i c = circles[i];
			Point center = Point(c[0], c[1]);
			circle(imgFiltered, center, 1, Scalar(0, 100, 100), 3, LINE_AA);
			int radius = c[2];
			circle(imgFiltered, center, radius, Scalar(255, 0, 255), 3, LINE_AA);
		}
		imshow("Ball_RedBottom", imgFiltered);
		imshow("Edges", edge);
		
		*/
		///////////////////////////////////////////////
		

		// Color detection range
		Mat lower_red_hue_range, upper_red_hue_range;
		Mat blue_hue_range, green_hue_range;

		inRange(imgHSV, Scalar(0, 100, 140), Scalar(10, 255, 255), lower_red_hue_range);
		inRange(imgHSV, Scalar(170, 100, 140), Scalar(179, 255, 255), upper_red_hue_range);
		imgThresholded = lower_red_hue_range + upper_red_hue_range;

		inRange(imgHSV, Scalar(80, 140, 140), Scalar(125, 255, 255), green_hue_range);
		imgThresholded += green_hue_range;

		inRange(imgHSV, Scalar(115, 100, 60), Scalar(150, 255, 255), blue_hue_range);
		imgThresholded += blue_hue_range;

		// Opening (Remove small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		// Closing (Remove small holes from the foreground)
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));

		morphologyEx(imgThresholded, imgThresholded, MORPH_CLOSE, getStructuringElement(MORPH_CROSS, Size(5, 5)));

		vector<vector<Point>> cont;
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;

		findContours(imgThresholded, cont, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		// Contours of objects considered noise due to their small size are removed
		for (unsigned int i = 0; i < cont.size(); i++)
		{
			if (contourArea(cont[i]) > 1000)
			{
				contours.push_back(cont[i]);
			}
		}

		vector<vector<Point>>contours_poly(contours.size());
		vector<Point2f>center(contours.size());
		vector<float>radius(contours.size());

		for (int i = 0; i < contours.size(); i++)
		{
			approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
			minEnclosingCircle((Mat)contours_poly[i], center[i], radius[i]);
		}

		// A history of centers is stored to draw the trajectory
		lines.insert(lines.begin(), center);
		if (lines.size() > 10)
		{
			lines.pop_back();
		}

		for (int i = 0; i < contours.size(); i++)
		{
			drawContours(imgOriginal, contours_poly, i, Scalar(255, 0, 0), 1, 8, vector<Vec4i>(), 1, Point());
			circle(imgOriginal, center[i], (int)radius[i], Scalar(0, 255, 0), 2, 8, 0);
			circle(imgOriginal, center[i], 3, Scalar(255, 255, 0), -1, 8, 0);

			string posx = "PosX: " + to_string((int)center[i].x);
			string posy = "PosY: " + to_string((int)center[i].y);
			center[i].x -= 35;
			putText(imgOriginal, posx, center[i], CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 0), 1, 8, false);
			center[i].y += 15;
			putText(imgOriginal, posy, center[i], CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 0), 1, 8, false);
			if (contours.size() == 1)
			{
				for (int j = 0; j < lines.size(); j++)
				{
					try
					{
						line(imgOriginal, lines.at(j).at(i), lines.at(j + 1).at(i), Scalar(255, 255, 0), 3);
					}
					catch (exception& e)
					{
						break;
					}
				}

			}

		}

		imshow("Thresholded Image", imgThresholded); // kernel

		imshow("Original", imgOriginal); // original image
		
		if (waitKey(30) == 27) // wait to escape key
		{
			cout << "Tecla escape presionada por el usuario" << endl;
			destroyAllWindows();
			break;
		}
	}

	return 0;
}
