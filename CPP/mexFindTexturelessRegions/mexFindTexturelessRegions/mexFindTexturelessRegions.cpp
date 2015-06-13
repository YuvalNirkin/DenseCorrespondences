/************************************************************************************
*									   Includes										*
************************************************************************************/
#include <vector>
#include <mex.h>
#include "MxArray.hpp"
#include <opencv2\imgproc.hpp>
#include "FindTexturelessRegion.hpp"

/************************************************************************************
*									  Namespaces									*
************************************************************************************/
using std::vector;

/************************************************************************************
*									 Declarations									*
************************************************************************************/
#define printfFnc(...) { mexPrintf(__VA_ARGS__); mexEvalString("drawnow;");}

/************************************************************************************
*									Implementation									*
************************************************************************************/

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	// Load images
	cv::Mat img = MxArray(prhs[0]).toMat(CV_32F);	// Convert to single floating point grayscale image

	// define parameters
	int winSize = 9;
	float t = 4;

	// Get parameters
	if (nrhs >= 1)
	{
		winSize = (int)*mxGetPr(prhs[1]);
		if (nrhs >= 2) t = (float)*mxGetPr(prhs[2]);
	}

	// Process
	vector<bool> texturelessRegion;
	findTexturelessRegion(img, winSize, t, texturelessRegion);
	
	// Output to Matlab
	cv::Mat out(img.rows, img.cols, CV_8U);
	unsigned char* out_data = out.data;
	for (int i = 0; i < texturelessRegion.size(); ++i)
		*out_data++ = (unsigned char)texturelessRegion[i];
	plhs[0] = MxArray(out);
}