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

namespace CVDetector
{
	static HWND hDesktopWnd;
	static CascadeClassifier *enable_cascade, *disable_cascade;

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
				detectSigns();
			}*/
			
			return 0;
		}

		__declspec(dllexport) void __cdecl SignDetector::detectSigns()
		{
			Mat frame = hwnd2mat(hDesktopWnd);
			signID = detectAndDisplay(frame, *enable_cascade, *disable_cascade);
			//printf("Found = %lf\n",signID);
			//waitKey(30); // -------------->uncomment this for debug mode
		}

		__declspec(dllexport) int __cdecl SignDetector::getFoundSign()
		{
			return signID;
		}
	}
}


