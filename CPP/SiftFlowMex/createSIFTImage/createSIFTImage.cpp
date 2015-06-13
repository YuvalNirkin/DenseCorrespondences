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

void myTranspose(const cv::Mat& src, cv::Mat& dst)
{
	dst.create(src.cols, src.rows, src.type());
	int r, c, chnl, channels = dst.channels();
	float *src_data, *dst_data = (float*)dst.data;
	int wstep = src.step.p[0], cstep = src.step.p[1];

	// Transpose order
	for (c = 0; c < src.cols; ++c)
	{
		for (r = 0; r < src.rows; ++r)
		{
			src_data = (float*)(src.data + (r*wstep + c*cstep));
			for (chnl = 0; chnl < channels; ++chnl)
			{
				*dst_data++ = *src_data++;
			}
		}
	}
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	cv::Mat img = MxArray(prhs[0]).toMat(CV_32F);	// Convert to single floating point grayscale image

	//cv::Mat img_t;
	//myTranspose(img, img_t);

	dc::SiftFlow siftFlow;
	cv::Mat siftImg;
	siftFlow.createSIFTImage(img, siftImg);

	// Output to Matlab
	cv::Mat siftImg_t;
	//myTranspose(siftImg, siftImg_t);
	siftImg_t = siftImg;
	FImage out(siftImg_t.cols, siftImg_t.rows, siftImg_t.channels());
	memcpy(out.pData, siftImg_t.data, siftImg_t.cols*siftImg_t.rows*siftImg_t.channels()*sizeof(float));
	out.OutputToMatlab(plhs[0]);
}