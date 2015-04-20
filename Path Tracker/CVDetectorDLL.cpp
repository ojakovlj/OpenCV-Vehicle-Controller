#include <stdio.h>
#include "CVDetectorDLL.h"
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <tchar.h>
#include <Windows.h>

const int FRAME_WIDTH = 1366/2;
const int FRAME_HEIGHT = 768/2;
int H_MIN = 0;
int H_MAX = 21;
int S_MIN = 156;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;
const int MAX_NUM_OBJECTS=50;
const int MIN_OBJECT_AREA = 2000;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/7;

using namespace cv;

namespace MathFuncs
{
	static HWND hDesktopWnd;

	Mat hwnd2mat(HWND hwnd){

		HDC hwindowDC,hwindowCompatibleDC;

		int height,width,srcheight,srcwidth;
		HBITMAP hbwindow;
		Mat src;
		BITMAPINFOHEADER  bi;

		hwindowDC=GetDC(hwnd);
		hwindowCompatibleDC=CreateCompatibleDC(hwindowDC);
		SetStretchBltMode(hwindowCompatibleDC,COLORONCOLOR);  

		RECT windowsize;    // get the height and width of the screen
		GetClientRect(hwnd, &windowsize);

		srcheight = windowsize.bottom;
		srcwidth = windowsize.right;
		height = windowsize.bottom/2;  //change this to whatever size you want to resize to
		width = windowsize.right/2;

		src.create(height,width,CV_8UC4);

		// create a bitmap
		hbwindow = CreateCompatibleBitmap( hwindowDC, width, height);
		bi.biSize = sizeof(BITMAPINFOHEADER);  
		bi.biWidth = width;    
		bi.biHeight = -height;  //this is the line that makes it draw upside down or not
		bi.biPlanes = 1;    
		bi.biBitCount = 32;    
		bi.biCompression = BI_RGB;    
		bi.biSizeImage = 0;  
		bi.biXPelsPerMeter = 0;    
		bi.biYPelsPerMeter = 0;    
		bi.biClrUsed = 0;    
		bi.biClrImportant = 0;

		// use the previously created device context with the bitmap
		SelectObject(hwindowCompatibleDC, hbwindow);
		// copy from the window device context to the bitmap device context
		StretchBlt( hwindowCompatibleDC, 0,0, width, height, hwindowDC, 0, 0,srcwidth,srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
		GetDIBits(hwindowCompatibleDC,hbwindow,0,height,src.data,(BITMAPINFO *)&bi,DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

		// avoid memory leak
		DeleteObject (hbwindow); DeleteDC(hwindowCompatibleDC); ReleaseDC(hwnd, hwindowDC);

		return src;
	}

	void drawObject(int x, int y,Mat &frame){

		//use some of the openCV drawing functions to draw crosshairs
		//on your tracked image!

		//UPDATE:JUNE 18TH, 2013
		//added 'if' and 'else' statements to prevent
		//memory errors from writing off the screen (ie. (-25,-25) is not within the window!)

		circle(frame,Point(x,y),20,Scalar(0,255,0),2);
		if(y-25>0)
			line(frame,Point(x,y),Point(x,y-25),Scalar(0,255,0),2);
		else line(frame,Point(x,y),Point(x,0),Scalar(0,255,0),2);
		if(y+25<FRAME_HEIGHT)
			line(frame,Point(x,y),Point(x,y+25),Scalar(0,255,0),2);
		else line(frame,Point(x,y),Point(x,FRAME_HEIGHT),Scalar(0,255,0),2);
		if(x-25>0)
			line(frame,Point(x,y),Point(x-25,y),Scalar(0,255,0),2);
		else line(frame,Point(x,y),Point(0,y),Scalar(0,255,0),2);
		if(x+25<FRAME_WIDTH)
			line(frame,Point(x,y),Point(x+25,y),Scalar(0,255,0),2);
		else line(frame,Point(x,y),Point(FRAME_WIDTH,y),Scalar(0,255,0),2);

		//putText(frame,intToString(x)+","+intToString(y),Point(x,y+30),1,1,Scalar(0,255,0),2);

	}

	void trackFilteredObject(Mat thresholded, Mat frame){
		int x, y;
		Mat temp;
		thresholded.copyTo(temp);
		//these two vectors needed for output of findContours
		vector< vector<Point> > contours;
		vector<Vec4i> hierarchy;
		//find contours of filtered image using openCV findContours function
		findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
		//use moments method to find our filtered object
		double refArea = 0;
		bool objectFound = false;
		if (hierarchy.size() > 0) {
			int numObjects = hierarchy.size();
			//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
			if(numObjects<MAX_NUM_OBJECTS){
				objectFound = false;
				for (int index = 0; index >= 0; index = hierarchy[index][0]) {

					Moments moment = moments((cv::Mat)contours[index]);
					double area = moment.m00;

					//if the area is less than 20 px by 20px then it is probably just noise
					//if the area is the same as the 3/2 of the image size, probably just a bad filter
					//we only want the object with the largest area so we safe a reference area each
					//iteration and compare it to the area in the next iteration.
					if(area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea){
						x = moment.m10/area;
						y = moment.m01/area;
						objectFound = true;
						refArea = area;
					}


				}
				//let user know you found an object
				if(objectFound ==true){
					putText(thresholded,"Tracking Object",Point(0,50),2,1,Scalar(0,255,0),2);
					//draw object location on screen
					drawObject(x,y,frame);
				}

			}else putText(thresholded,"TOO MUCH NOISE! ADJUST FILTER",Point(0,50),1,2,Scalar(0,0,255),2);
		}
	}

	extern "C"{
		__declspec(dllexport) double __cdecl MyMathFuncs::Add(double a, double b)
		{	
			printf("In C this would be %lf + %d = %lf\n",a,b,a+b);
			return a + b;
		}

		__declspec(dllexport) double __cdecl MyMathFuncs::Subtract(double a, double b)
		{
			Mat dst, frame;
			HDC hDesktopDC;
			hDesktopWnd=GetDesktopWindow();
			hDesktopDC=GetDC(hDesktopWnd);
			VideoCapture vcap;

			int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
			int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);

			// create a bitmap
			HBITMAP hbDesktop = CreateCompatibleBitmap( hDesktopDC, width, height);

			while(1){
				frame = hwnd2mat(hDesktopWnd);
				// Perform a Gaussian blur
				GaussianBlur(frame, dst, Size( 15, 15), 0, 0);
				//Convert to HSV
				cvtColor(dst,dst,COLOR_BGR2HSV);
				// Apply threshold to eliminate noise and emphasize filtered objects
				inRange(dst,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),dst);
				//erode and dilate with larger element so make sure object is nicely visible
				Mat erodeElement = getStructuringElement( MORPH_RECT,Size(2,2));
				Mat dilateElement = getStructuringElement( MORPH_RECT,Size(4,4));
				erode(dst,dst,erodeElement);
				erode(dst,dst,erodeElement);
				dilate(dst,dst,dilateElement);
				dilate(dst,dst,dilateElement);

				// Perform Canny edge detection
				//findContours(dst, dst, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
				//Canny(dst, dst, 1, 3);

				trackFilteredObject(dst, frame);
				// Show the processed image
				imshow("Example3-out", dst);
				imshow("Example3-in", frame);
				waitKey(30); //this must be here or image will not be displayed :/

			}


			cvWaitKey(0);
			return a-b;
		}

		__declspec(dllexport) double __cdecl MyMathFuncs::Multiply(double a, double b)
		{
			return a * b;
		}

		__declspec(dllexport) double __cdecl MyMathFuncs::Divide(double a, double b)
		{
			if (b == 0)
			{
				return -1;
			}

			return a / b;
		}
	}
}


