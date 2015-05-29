#include <stdio.h>
#include "CVDetectorDLL.h"
#include "opencv2/core/core.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <tchar.h>
#include <Windows.h>

using namespace cv;

string window_name;
String cascade_enable, cascade_disable;
const int MAX_NUM_OBJECTS=50;
const int MIN_OBJECT_AREA = 2000;
int MAX_OBJECT_AREA;

namespace CVDetector
{
	static HWND hDesktopWnd;
	static CascadeClassifier *enable_cascade, *disable_cascade;
	static int height,width;

	string intToString(int number){
		std::stringstream ss;
		ss << number;
		return ss.str();
	}

	Mat hwnd2mat(HWND hwnd){

		HDC hwindowDC,hwindowCompatibleDC;

		int srcheight,srcwidth;
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
		height = windowsize.bottom;  //change this to whatever size you want to resize to
		width = windowsize.right;

		src.create(height,width,CV_8UC4);

		MAX_OBJECT_AREA = height/2*width/7;

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

	int detectAndDisplay( Mat frame, CascadeClassifier enableCascade, CascadeClassifier disableCascade )
	{
		std::vector<Rect> enableSigns, disableSigns;
		Mat frame_gray;

		cvtColor( frame, frame_gray, CV_BGR2GRAY );
		equalizeHist( frame_gray, frame_gray );

		//-- Detect signs, load 'em up into appropriate arrays
		enableCascade.detectMultiScale( frame_gray, enableSigns, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30)  );
		disableCascade.detectMultiScale( frame_gray, disableSigns, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30));
		for( size_t i = 0; i < enableSigns.size(); i++ )
		{
			Point center( enableSigns[i].x + enableSigns[i].width*0.5, enableSigns[i].y + enableSigns[i].height*0.5 );
			ellipse( frame, center, Size( enableSigns[i].width*0.5, enableSigns[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );
		}

		//-- Show what you got
		//imshow( window_name, frame ); // -----------> comment this for release

		if (enableSigns.size() > 0)
			return 1;
		if (disableSigns.size() > 0)
			return 2;
		// otherwise
		return 0;
	}


	extern "C"{

		__declspec(dllexport) double signID;
		__declspec(dllexport) double axisValue;

		double trackFilteredObject(Mat thresholded){
			int x, y;
			double axis = -2.0;
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
					if(objectFound == true){
						/*if( x > width/2 + 20)
							axis = 1; //RIGHT
						else if( x < width/2 - 20)
							axis = -1; //LEFT
						else axis = 0; //STRAIGHT*/
						axisValue = (double)((2.0*x/width)-1.0);
						return (double)(2*x/width)-1.0;

					}
				}
			}
			return axis;
		}

		__declspec(dllexport) int __cdecl SignDetector::initialiseDetector()
		{			
			HDC hDesktopDC;
			hDesktopWnd=GetDesktopWindow();
			hDesktopDC=GetDC(hDesktopWnd);
			disable_cascade = new CascadeClassifier;
			enable_cascade = new CascadeClassifier;
			
			window_name  = "Detecting ...";
			cascade_enable = "classifier_enable.xml";
			cascade_disable = "classifier_disable.xml";
			if( !(*enable_cascade).load( cascade_enable ) ){ return -1; };
			if( !(*disable_cascade).load( cascade_disable ) ){ return -1; };

			// get the height and width of the screen
			int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
			int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);

			// create a bitmap
			HBITMAP hbDesktop = CreateCompatibleBitmap( hDesktopDC, width, height);

			/*while(1){ //----------->comment this for release mode
				//detectSigns();
				RoadTracker::doTracking();
			}*/
			
			return 0;
		}

		__declspec(dllexport) void __cdecl SignDetector::detectSigns()
		{
			Mat frame = hwnd2mat(hDesktopWnd);
			signID = detectAndDisplay(frame, *enable_cascade, *disable_cascade);
			waitKey(30); // -------------->uncomment this for debug mode
		}

		__declspec(dllexport) void __cdecl RoadTracker::doTracking()
		{			
			Mat frame = hwnd2mat(hDesktopWnd);
			Mat dst;
			//Convert to HSV
			cvtColor(frame,dst,COLOR_BGR2HSV);
			// Apply threshold to eliminate noise and emphasize filtered objects
			// These HSV values work best for Red color
			inRange(dst,Scalar(0,156,0),Scalar(21,256,256),dst);
			//erode and dilate with larger element so make sure object is nicely visible
			Mat erodeElement = getStructuringElement( MORPH_RECT,Size(2,2));
			Mat dilateElement = getStructuringElement( MORPH_RECT,Size(4,4));
			erode(dst,dst,erodeElement);
			erode(dst,dst,erodeElement);
			dilate(dst,dst,dilateElement);
			dilate(dst,dst,dilateElement);
			trackFilteredObject(dst);
			printf("Axis value = %d\n",axisValue);
			// Show the processed image
			//imshow("Example3-out", dst);
			//imshow("Example3-in", dst);
			//waitKey(30); //this must be here or image will not be displayed :/
			//... uncomment for debug mode
			
		}

		__declspec(dllexport) int __cdecl SignDetector::getFoundSign()
		{
			return signID;
		}

		__declspec(dllexport) double __cdecl RoadTracker::getHorizontalAxis()
		{
			return axisValue;
		}
	}
}


