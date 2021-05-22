#include<iostream>
#include<cstdlib>
#include<ctime>
#include<thread>
#include<opencv2/opencv.hpp>
#include<Windows.h>

using namespace cv;
using namespace std;

int srcheight, srcwidth;

Mat hwnd2mat(HWND hwnd) {
	HDC hwindowDC, hwindowCompatibleDC;
	HBITMAP hbwindow;
	Mat src;
	BITMAPINFOHEADER  bi{};
	hwindowDC = GetDC(hwnd);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);
	RECT windowsize;    // get the height and width of the screen
	GetClientRect(hwnd, &windowsize);

	RECT rect = { 0 };

	if (hwnd == NULL) {
		LPCWSTR str = TEXT("DISPLAY");
		HDC hdc = CreateDC(str, NULL, NULL, NULL);
		srcheight = GetDeviceCaps(hdc, VERTRES);
		srcwidth = GetDeviceCaps(hdc, HORZRES);
	} else {
		GetWindowRect(hwnd, &rect);
		srcwidth = rect.right - rect.left;
		srcheight = rect.bottom - rect.top;
	}

	src.create(srcheight, srcwidth, CV_8UC4);

	// create a bitmap
	hbwindow = CreateCompatibleBitmap(hwindowDC, srcwidth, srcheight);

	bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
	bi.biWidth = srcwidth;
	bi.biHeight = -srcheight;  //this is the line that makes it draw upside down or not
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
	StretchBlt(hwindowCompatibleDC, 0, 0, srcwidth, srcheight, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, srcheight, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow
	// avoid memory leak
	DeleteObject(hbwindow); DeleteDC(hwindowCompatibleDC); ReleaseDC(hwnd, hwindowDC);
	return src;
}

struct MatchResult{
	bool isFind = false;
	Point2i pos;
};

MatchResult findTemplate(Mat & _src, Mat & _mytemplate) {
	Mat src;
	MatchResult Result;

	cvtColor(_src, src, COLOR_BGRA2BGR);//CV_RGBA2RGB表示4通道转成3通道

	int result_cols = src.cols - _mytemplate.cols + 1;
	int result_rows = src.rows - _mytemplate.rows + 1;
	Mat result(result_cols, result_rows, CV_32FC1);

	matchTemplate(src, _mytemplate, result, 1);//这里我们使用的匹配算法是标准平方差匹配 method=CV_TM_SQDIFF_NORMED，数值越小匹配度越好
	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
	
	//imshow("result", result);

	double minVal;
	double maxVal;

	Point minLoc;
	Point maxLoc;
	Point matchLoc;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	//cout << minVal << endl;
	if (minVal == 0) {

		matchLoc = minLoc;

		Result.isFind = true;
		Result.pos.x = matchLoc.x + _mytemplate.cols / 2.f;
		Result.pos.y = matchLoc.y + _mytemplate.rows / 2.f;

	}

	

	//circle(_src, Result.pos, 5, Scalar(0, 0, 255), -1, LINE_AA, 0);
	return Result;
}

int main() {
	Mat src;

	Mat mytemplate = imread("template.png");
	MatchResult result;
	
	//imshow("template", mytemplate);

	Sleep(3000);
	cout << "Begin!" << endl;

	while (true) {

		src = hwnd2mat(NULL);

		result = findTemplate(src, mytemplate);

		//imshow("src", src);
		
		//
		if (result.isFind) {

			cout << result.pos << endl;

			//mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, result.pos.x * 65536 / srcwidth, result.pos.y * 65536 / srcheight, 0, 0);
			
			mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, result.pos.x * 65536 / srcwidth, result.pos.y * 65536 / srcheight, 0, 0);
			mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, result.pos.x * 65536 / srcwidth, result.pos.y * 65536 / srcheight, 0, 0);
			Sleep(500);
			mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, result.pos.x * 65536 / srcwidth, result.pos.y * 65536 / srcheight, 0, 0);
			

			mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, 0.5 * result.pos.x * 65536 / srcwidth, 0.5 * result.pos.y * 65536 / srcheight, 0, 0);
		}
		

		//if (waitKey(1) == 27) break;
		Sleep(2000);
	}


	return 0;
}