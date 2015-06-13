#ifndef __SCALE_MAPS__
#define __SCALE_MAPS__

/************************************************************************************
*									   Includes										*
************************************************************************************/
#include "DenseSIFT_Export.h"
#include "DenseSIFT_Prerequisites.h"
#include "DenseSIFT_Types.h"
#include <vector>

/************************************************************************************
*									 Declarations									*
************************************************************************************/
namespace dc
{
	/** Scale maps implementation using scale propagation.
		References:
		[1] M. Tau and T. Hassner, "Dense Correspondences Across Scenes and 
		Scales," arXiv preprint arXiv:1406.6323, 24 Jun. 2014. Please see
		project webpage for information on more recent versions:
		http://www.openu.ac.il/home/hassner/scalemaps
		
		[2] C. Liu, J. Yuen, A. Torralba, J. Sivic, and W. Freeman. "Sift
		flow: dense correspondence across different scenes." In European
		Conf. Comput. Vision, pages 28-42, 2008.
		http://people.csail.mit.edu/celiu/ECCV2008/
	*/
	class DENSE_SIFT_API ScaleMaps
	{
	public:

		/**	Default Constructor.
		*/
		ScaleMaps();
		
		/** Set weight function to either linear or exponential.
			linearWeights = 1 + (1 / var) * (weights - mean) * (image(i, j) - mean)
			expWeights = exp(-(weights - image(i, j)) .^ 2 / (0.6 * var))
			Default is linear.
			\param exponential true for exponential weight function and false for linear
		*/
		void setWeightFunc(bool exponential);

		/** Executes scale maps algorithm.
			\param img Grayscales floating point image in the range [0 1]
			\param features Sparse features to seed from (0 based pixel indices)
			\param scaleMap Output the scale for each pixel
		*/
		void run(const cv::Mat& img, const std::vector<FeatureType>& features,
			std::vector<float>& scaleMap);

	private:

		/// Function pointer to the selected weight function
		void(*mWeightFunc)(std::vector<float>& weights, size_t weightSize,
			float center, float mean, float var);
	};

}	// namespace dc

#endif	// __SCALE_MAPS__