#ifndef __DENSE_SIFT__
#define __DENSE_SIFT__

/************************************************************************************
*									   Includes										*
************************************************************************************/
#include "DenseSIFT_Export.h"
#include "DenseSIFT_Prerequisites.h"
#include "DenseSIFT_Types.h"
#include <string>
#include <vector>
#include <memory>

/************************************************************************************
*								 Forward Declarations								*
************************************************************************************/

/************************************************************************************
*									 Declarations									*
************************************************************************************/
namespace dc
{
	/** A class that provides dense SIFT functionality
	*/
	class DENSE_SIFT_API DenseSIFT
	{
	public:

		/** Destructor.
		*/
		~DenseSIFT();
	
		/** Get a singleton instance of DenseSIFT.
		*/
		static DenseSIFT& getInstance();

		void init(int stepSize = 1, bool filterTexturelessRegions = false);

		/** Shutdown the DenseSIFT engine.
		*/
		void shutdown();

		void setSIFTFlowParams(double d, int nlevels = 4, double alpha = 0.01,
			double gamma = 0.001, int nIterations = 40, int nTopIterations = 100,
			int wsize = 3, int topwsize = 10);

		/** Set output directory for intermediate results.
		*/
		void setOutputDir(const std::string& outDir);

		/** Set source image for matching.
			\param img Path to the souce image
			\param sparseFeatures Sparse features to seed the image's scale map.
			If not provided than the scale for each pixel will be constant.
		*/
		void setSource(const std::string& img, 
			const std::vector<FeatureType>& sparseFeatures = std::vector<FeatureType>());

		/** Set target image for matching.
			\param img Path to the target image.
			\param sparseFeatures Sparse features to seed the image's scale map.
			If not provided than the scale for each pixel will be constant.
		*/
		void setTarget(const std::string& img, 
			const std::vector<FeatureType>& sparseFeatures = std::vector<FeatureType>());

		/**	Do dense matching
			\param denseMatches Output the matched pixels of the source and target images.
			\param sparseMatches The indices of the matched sparse features of the source 
			and target images.
		*/
		void match(std::vector<MatchType>& denseMatches,
			const std::vector<MatchType>& sparseMatches = std::vector<MatchType>());

	private:

		/** Private default constructor.
		*/
		DenseSIFT();

		/**	Load grayscale image from file.
		*/
		void loadGrayscaleImage(const std::string& path, cv::Mat& img);

	private:
		std::string mOutDir;
		std::unique_ptr <cv::Mat> mSource;
		std::unique_ptr <cv::Mat> mTarget;
		std::unique_ptr <cv::Mat> mSourceSIFT;
		std::unique_ptr <cv::Mat> mTargetSIFT;
		std::vector<FeatureType> mSrcSparseFeatures;
		std::vector<FeatureType> mTgtSparseFeatures;
		std::vector<bool> mSrcTexturelessRegions;
		std::vector<bool> mTgtTexturelessRegions;
		ScaleMaps* mScaleMaps;
		SiftFlow* mSiftFlow;

		// Parameters
		int mStepSize;
		bool mFilterTexturelessRegions;
	};
}	// namespace dc

#endif	// __DENSE_SIFT__