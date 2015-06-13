/************************************************************************************
*									   Includes										*
************************************************************************************/
#include <SiftFlow.h>
#include <mex.h>
#include "MxArray.hpp"

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
	cv::Mat siftImg1t = MxArray(prhs[0]).toMat(CV_32F, false);
	cv::Mat siftImg2t = MxArray(prhs[1]).toMat(CV_32F, false);

	cv::Mat siftImg1, siftImg2;
	myTranspose(siftImg1t, siftImg1);
	myTranspose(siftImg2t, siftImg2);

	//FImage Im1, Im2;
	//Im1.LoadMatlabImage(prhs[0]);	// height * width * channels (in that order)
	//Im2.LoadMatlabImage(prhs[1]);

	// define parameters
	double alpha = 0.01;
	double d = 1;
	double gamma = 0.001;
	int nlevels = 4;
	int topwsize = 10;
	int wsize = 5;
	int nTopIterations = 60;
	int nIterations = 40;
	int nHierarchy = 2;

	// load the parameters for matching
	if (nrhs >= 3)
	{
		alpha = mxGetPr(mxGetField(prhs[2], 0, "alpha"))[0];
		d = mxGetPr(mxGetField(prhs[2], 0, "d"))[0];
		gamma = mxGetPr(mxGetField(prhs[2], 0, "gamma"))[0];
		nlevels = mxGetPr(mxGetField(prhs[2], 0, "nlevels"))[0];
		topwsize = mxGetPr(mxGetField(prhs[2], 0, "topwsize"))[0];
		wsize = mxGetPr(mxGetField(prhs[2], 0, "wsize"))[0];
		nTopIterations = mxGetPr(mxGetField(prhs[2], 0, "nTopIterations"))[0];
		nIterations = mxGetPr(mxGetField(prhs[2], 0, "nIterations"))[0];
	}


	// Process
	dc::SiftFlow siftFlow(d, nlevels, alpha, gamma, nIterations, nTopIterations,
		wsize, topwsize);
	//DImage vx, vy;
	cv::Mat vx, vy;
	siftFlow.coarse2fineFlow(siftImg1, siftImg2, vx, vy);

	// Output to Matlab
	//vx.OutputToMatlab(plhs[0]);
	//vy.OutputToMatlab(plhs[1]);
	plhs[0] = MxArray(vx);
	plhs[1] = MxArray(vy);
}