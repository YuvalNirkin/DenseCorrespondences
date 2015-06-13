#ifndef __SIFT_FLOW__
#define __SIFT_FLOW__

/************************************************************************************
*									   Includes										*
************************************************************************************/
#include "DenseSIFT_Export.h"
#include "Image.h"
#include <string>
#include <vector>
#include <opencv2\core.hpp>

/************************************************************************************
*									 Declarations									*
************************************************************************************/
namespace dc
{
	/** A class that provides dense SIFT functionality
	*/
	class DENSE_SIFT_API SiftFlow
	{
	public:

		/** Constructor with optical flow parameters.
			\param d Smoothness term threshold.
			\param nLevels The number of pyramid levels.
			\param alpha Smoothness term weight.
			\param gamma Range term weight.
			\param nIterations The number of iterations for each inner pyramid level.
			\param nTopIterations The number of iterations for the top pyramid level.
			\param wsize The window size for each inner pyramid level.
			\param topwsize The window size for the top pyramid level.
		*/
		SiftFlow(double d, int nlevels = 4, double alpha = 0.01,
			double gamma = 0.001, int nIterations = 40, int nTopIterations = 100,
			int wsize = 3, int topwsize = 10);

		/** Constructor with optical flow parameters.
			\param nLevels The number of pyramid levels.
			\param alpha Smoothness term weight.
			\param gamma Range term weight.
			\param nIterations The number of iterations for each inner pyramid level.
			\param nTopIterations The number of iterations for the top pyramid level.
			\param wsize The window size for each inner pyramid level.
			\param topwsize The window size for the top pyramid level.
		*/
		SiftFlow(int nlevels = 4, double alpha = 0.01,
			double gamma = 0.001, int nIterations = 40, int nTopIterations = 100,
			int wsize = 3, int topwsize = 10);

		/** Initialize optical flow parameters.
			\param d Smoothness term threshold.
			\param nLevels The number of pyramid levels.
			\param alpha Smoothness term weight.
			\param gamma Range term weight.
			\param nIterations The number of iterations for each inner pyramid level.
			\param nTopIterations The number of iterations for the top pyramid level.
			\param wsize The window size for each inner pyramid level.
			\param topwsize The window size for the top pyramid level.
		*/
		void init(double d, int nlevels = 4, double alpha = 0.01,
			double gamma = 0.001, int nIterations = 40, int nTopIterations = 100,
			int wsize = 3, int topwsize = 10);

		/** Initialize optical flow parameters.
			\param nLevels The number of pyramid levels.
			\param alpha Smoothness term weight.
			\param gamma Range term weight.
			\param nIterations The number of iterations for each inner pyramid level.
			\param nTopIterations The number of iterations for the top pyramid level.
			\param wsize The window size for each inner pyramid level.
			\param topwsize The window size for the top pyramid level.
		*/
		void init(int nlevels = 4, double alpha = 0.01,
			double gamma = 0.001, int nIterations = 40, int nTopIterations = 100,
			int wsize = 3, int topwsize = 10);

		/** Create SIFT image.
			\param img Input grayscale image.
			\param siftImg Output SIFT image.
			\param scaleMap Optionally specify the scale of each SIFT descriptor.
			\param stepSize The step size in pixels on the input image. 
			Use this to decrease the size of the SIFT image.
		*/
		void createSIFTImage(const cv::Mat& img, cv::Mat& siftImg,
			const std::vector<float>& scaleMap = std::vector<float>(),
			int stepSize = 1);

		/** Create pyramids.
			\param img Input image to create the pyramids from.
			\param pyramids Output the created pyramids.
			\param levels The number of pyramid levels to create.
		*/
		void createPyramids(const cv::Mat& img, std::vector<cv::Mat>& pyramids, 
			int levels = 4);

		/** Convert flow to color image.
		*/
		cv::Mat flow2Color(const DImage& flow, double maxFlow = 0);

		/** Do SIFT flow.
			\param siftImg1 The source SIFT image.
			\param siftImg2 The target SIFT image.
			\param flow Output the flow along the X and Y axes [x y x y...]
			\param gamma Range term weight.
			\param nIterations The number of iterations.
			\param nHierarchy The number of resolution levels.
			\param OffsetX The offset along the X axis.
			\param OffsetY The offset along the Y axis.
			\param WinSizeX Optional windows size along the X axis per pixel.
			\param WinSizeY Optional windows size along the Y axis per pixel.
		*/
		void discreteFlow(const cv::Mat& siftImg1, const cv::Mat& siftImg2,
			DImage& flow, double gamma = 0.001, int nIterations = 40,
			int nHierarchy = 2, int wsize = 5,
			const cv::Mat& OffsetX = cv::Mat(), const cv::Mat& OffsetY = cv::Mat(),
			const IntImage& WinSizeX = IntImage(), const IntImage& WinSizeY = IntImage());

		/** Do course to fine SIFT flow
			\param siftImg1 The source SIFT image.
			\param siftImg2 The target SIFT image.
			\param vx The flow along the X axis.
			\param vy The flow along the Y axis.
		*/
		void coarse2fineFlow(const cv::Mat& siftImg1, const cv::Mat& siftImg2,
			cv::Mat& vx, cv::Mat& vy);

	private:

		// Parameters
		double d;			///< Smoothness term threshold
		int nlevels;		///< The number of pyramid levels
		double alpha;		///< Smoothness term weight
		double gamma;		///< Range term weight
		int nIterations;	///< The number of iterations for each inner pyramid level
		int nTopIterations;	///< The number of iterations for the top pyramid level
		int wsize;			///< The window size for each inner pyramid level
		int topwsize;		///< The window size for the top pyramid level
	};
}	// namespace dc

#endif	// __SIFT_FLOW__