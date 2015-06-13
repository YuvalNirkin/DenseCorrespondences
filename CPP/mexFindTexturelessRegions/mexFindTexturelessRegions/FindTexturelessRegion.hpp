/************************************************************************************
*									   Includes										*
************************************************************************************/
#include <vector>
#include <exception>
#include <opencv2\core.hpp>
#include <opencv2\imgproc.hpp>

/************************************************************************************
*									  Namespaces									*
************************************************************************************/
using std::vector;

/************************************************************************************
*									 Declarations									*
************************************************************************************/

/************************************************************************************
*									Implementation									*
************************************************************************************/

void findTexturelessRegion(const cv::Mat& img, unsigned int winSize, float t, 
	std::vector<bool>& texturelessImg)
{
	// Check the size of window to see if it is an odd number.
	if (winSize % 2 == 0)
		throw std::exception("The window size must be an odd number!");
	if (winSize <= 1)
		throw std::exception("The window size must be greater than 1!");

	// Initialize
	texturelessImg.resize(img.total());
	cv::Mat sqGradImg(img.rows, img.cols, CV_32F, 0.0f);
	int r, c, k, channels = img.channels(), winSize2 = winSize*winSize;
	int hlimit, vlimit;
	float diff, sum, avg, t2 = t*t / (255*255);

	// Produce Squared Horizontal Gradient image sqGradImg
	hlimit = img.cols - 1; vlimit = img.rows - 1;
	for (r = 1; r < vlimit; ++r)
	{
		for (c = 1; c < hlimit; ++c)
		{
			float* img_data = ((float*)(img.data) + ((r*img.cols + c)*channels));
			sum = 0.0f;
			//for (k = 0; k < channels; ++k)
			{
				diff = *img_data++;
				diff -= *img_data;
				sum += (diff*diff);
			}
			//sum /= channels;
			sqGradImg.at<float>(r, c) = sum;
		}
	}

	// Find out how many rows and columns are to the left / right / up / down of the
	// central pixel based on the window size
	int win = ((int)winSize - 1) / 2;

	// Compute average within predefined box window of size windowSize^2
	hlimit = img.cols - win; vlimit = img.rows - win;
	int rw, cw;
	for (r = win; r < vlimit; ++r)
	{
		for (c = win; c < hlimit; ++c)
		{
			// Go over the square window
			sum = 0.0f;
			avg = 0.0f;
			for (rw = r - win; rw <= (r + win); ++rw)
			{
				for (cw = c - win; cw <= (c + win); ++cw)
				{
					sum += sqGradImg.at<float>(rw, cw);
				}
			}

			// Compute the average
			avg = sum / winSize2;

			// Apply threshold
			if (avg < t2)
				texturelessImg[r*img.cols + c] = true; // Mark detected textureless pixel
		}
	}
}