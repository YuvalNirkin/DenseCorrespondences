#include <mex.h>
#include "MxArray.hpp"
#include "ScaleMaps.h"
#include <exception>

// OpenCV (for handling images)
#include <opencv2\core.hpp>

using std::exception;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	try
	{
		// Get input from MATLAB
		//cv::Mat frames = MxArray(prhs[0]).toMat();	
		cv::Mat img = MxArray(prhs[1]).toMat(CV_32F);	// Convert to single floating point grayscale image
		std::string weight_func = MxArray(prhs[2]).toString();

		// Convert features
		int numdims = mxGetNumberOfDimensions(prhs[0]);
		if (numdims != 2) throw exception("frames must be a 2D matrix!");
		const mwSize *dims = mxGetDimensions(prhs[0]);
		if (dims[0] != 4) throw exception("The number of rows for \"frames\" must be 4!");
		double* frames = mxGetPr(prhs[0]);
		int frameCount = dims[1];
		std::vector<dc::FeatureType> features(frameCount);
		std::copy(frames, frames + frameCount * 4, (float*)features.data());

		for (size_t i = 0; i < features.size(); ++i)
		{
			dc::FeatureType& feat = features[i];
			feat.x -= 1;
			feat.y -= 1;
		}

		// Initialize
		dc::ScaleMaps sm;
		sm.setWeightFunc(weight_func == "exponential");

		// Process
		std::vector<float> scaleMap;
		sm.run(img, features, scaleMap);

		// Send output back to MATLAB
		if (nlhs < 1) return;

		cv::Mat_<double> output(img.total(), 4);
		int r, c;
		double* output_data = (double*)output.data;
		float* scaleMap_data = scaleMap.data();
		for (c = 1; c <= img.cols; ++c)
		{
			for (r = 1; r <= img.rows; ++r)
			{
				*output_data++ = (double)c;	// x
				*output_data++ = (double)r;	// y
				*output_data++ = (double)scaleMap[(r - 1)*img.cols + (c - 1)];	// scale
				*output_data++ = 0.0;	// orientation
				//*output_data++ = (double)c;	// x
				//*output_data++ = (double)r;	// y
				//*output_data++ = (double)*scaleMap_data++;	// scale
				//*output_data++ = 0.0;	// orientation
			}
		}

		plhs[0] = MxArray(output.t());
	}
	catch (exception& e)
	{
		mexPrintf("Error: %s\n", e.what());
	}
}