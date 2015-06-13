/************************************************************************************
*									   Includes										*
************************************************************************************/
#include <SiftFlow.h>
#include <mex.h>
#include "MxArray.hpp"
#include <opencv2\imgproc.hpp>

/************************************************************************************
*									  Namespaces									*
************************************************************************************/

/************************************************************************************
*									 Declarations									*
************************************************************************************/

/************************************************************************************
*									Implementation									*
************************************************************************************/

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	FImage Im;
	Im.LoadMatlabImage(prhs[0]);	// height * width * channels (in that order)

	cv::Mat img = MxArray(prhs[0]).toMat(CV_32F);	// Convert to single floating point grayscale image
	int rows = img.rows;
	int cols = img.cols;
	int channels = img.channels();

	cv::Mat img2, img3;
	cv::GaussianBlur(img, img2, cv::Size(5, 5), 0.67, 0, cv::BORDER_REPLICATE);

	int w = (int)ceil(img.cols / 2.0f);
	int h = (int)ceil(img.rows / 2.0f);
	cv::resize(img, img3, cv::Size(w, h), 0, 0, cv::INTER_CUBIC);

	plhs[0] = MxArray(img2);
	plhs[1] = MxArray(img3);
}