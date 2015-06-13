/************************************************************************************
*									   Includes										*
************************************************************************************/
#include "DenseSIFT.h"
#include "ScaleMaps.h"
#include "SiftFlow.h"
#include "ImageFeature.h"
#include "BPFlow.h"
#include <fstream>	// Debug
#include <boost\format.hpp>	// Debug
#include <boost\timer\timer.hpp>	// Debug
#include <boost\filesystem.hpp>
#include <vl\mathop.h>
#include <vl\sift.h>
#include <opencv2\imgproc.hpp>

/************************************************************************************
*									  Namespaces									*
************************************************************************************/
using std::ofstream;	// Debug
using namespace boost::filesystem;

/************************************************************************************
*									 Declarations									*
************************************************************************************/

/************************************************************************************
*									Implementation									*
************************************************************************************/
namespace dc
{
	DenseSIFT::DenseSIFT() :
		mOutDir("Output"),
		mSource(new cv::Mat()),
		mTarget(new cv::Mat()),
		mSourceSIFT(new cv::Mat()),
		mTargetSIFT(new cv::Mat()),
		mScaleMaps(nullptr),
		mSiftFlow(nullptr),
		mStepSize(1),
		mFilterTexturelessRegions(false)
	{
		mScaleMaps = new ScaleMaps();
		mSiftFlow = new SiftFlow();
	}

	DenseSIFT::~DenseSIFT()
	{
		shutdown();
	}

	DenseSIFT& DenseSIFT::getInstance()
	{
		static DenseSIFT instance;
		return instance;
	}

	void DenseSIFT::init(int stepSize, bool filterTexturelessRegion)
	{
		mStepSize = stepSize;
		mFilterTexturelessRegions = filterTexturelessRegion;
	}

	void DenseSIFT::shutdown()
	{
		if (mScaleMaps != nullptr) delete mScaleMaps;
		if (mSiftFlow != nullptr) delete mSiftFlow;
	}

	void findTexturelessRegions(const cv::Mat& img, unsigned int winSize, float t,
		std::vector<bool>& texturelessImg)
	{
		// Check the size of window to see if it is an odd number.
		if (winSize % 2 == 0)
			throw std::exception("The window size must be an odd number!");
		if (winSize <= 1)
			throw std::exception("The window size must be greater than 1!");

		// Initialize
		texturelessImg.resize(img.total());
		std::fill(texturelessImg.begin(), texturelessImg.end(), 0);
		cv::Mat sqGradImg(img.rows, img.cols, CV_32F, 0.0f);
		int r, c, k, channels = img.channels(), winSize2 = winSize*winSize;
		int hlimit, vlimit;
		float diff, sum, avg, t2 = t*t / (255 * 255);

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
					// Add X gradient
					diff = *img_data++;
					diff -= *img_data;
					sum += (diff*diff);

					// Add Y gradient
					diff = img.at<float>(r, c) - img.at<float>(r + 1, c);
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

	void DenseSIFT::setSIFTFlowParams(double d, int nlevels, double alpha,
		double gamma, int nIterations, int nTopIterations,
		int wsize, int topwsize)
	{
		mSiftFlow->init(d, nlevels, alpha, gamma, nIterations, nTopIterations,
			wsize, topwsize);
	}

	void DenseSIFT::setOutputDir(const std::string& outDir)
	{
		mOutDir = outDir;
	}

	cv::Mat readSIFTImage(const string& siftPath)
	{
		cv::Mat siftImg;
		int width, height;

		std::ifstream file(siftPath);

		// Read sift image size
		file.read((char *)&width, sizeof(int));
		file.read((char *)&height, sizeof(int));

		// Read sift image data
		siftImg.create(height, width, CV_32FC(128));
		file.read((char*)siftImg.data, siftImg.total()*siftImg.elemSize());

		return siftImg;
	}

	void writeSIFTImage(const string& siftPath, cv::Mat& siftImg)
	{
		std::ofstream file(siftPath);
		file.write((char*)&siftImg.cols, sizeof(int));
		file.write((char*)&siftImg.rows, sizeof(int));
		file.write((char*)siftImg.data, siftImg.total()*siftImg.elemSize());
	}

	void DenseSIFT::setSource(const std::string& img, const std::vector<FeatureType>& sparseFeatures)
	{
		loadGrayscaleImage(img, *mSource);
		mSrcSparseFeatures = sparseFeatures;

		if (mFilterTexturelessRegions)
			findTexturelessRegions(*mSource, 21, 4.0f, mSrcTexturelessRegions);

		// If not using scale maps create SIFT image
		if (sparseFeatures.empty())
		{
			/*
			path siftPath = path(mOutDir) /= path(img).replace_extension("sift").filename();
			if (exists(siftPath)) *mSourceSIFT = readSIFTImage(siftPath.string());
			else
			{
				mSiftFlow->createSIFTImage(*mSource, *mSourceSIFT, std::vector<float>(), mStepSize);
				writeSIFTImage(siftPath.string(), *mSourceSIFT);
			}
			*/
			mSiftFlow->createSIFTImage(*mSource, *mSourceSIFT, std::vector<float>(), mStepSize);
		}
	}

	void DenseSIFT::setTarget(const std::string& img, const std::vector<FeatureType>& sparseFeatures)
	{
		loadGrayscaleImage(img, *mTarget);
		mTgtSparseFeatures = sparseFeatures;

		if (mFilterTexturelessRegions)
			findTexturelessRegions(*mTarget, 21, 4.0f, mTgtTexturelessRegions);

		/*/// Debug ///
		cv::Mat debug(mTarget->rows, mTarget->cols, CV_8U);
		unsigned char* debug_data = debug.data;
		for (size_t i = 0; i < mTgtTexturelessRegions.size(); i++)
			*debug_data++ = 255*(int)mTgtTexturelessRegions[i];
		cv::imshow("mTarget", *mTarget);
		cv::imshow("debug", debug);
		cv::waitKey(0);
		/////////////*/

		// If not using scale maps create SIFT image
		if (sparseFeatures.empty())
		{
			/*
			path siftPath = path(mOutDir) /= path(img).replace_extension("sift").filename();
			if (exists(siftPath)) *mTargetSIFT = readSIFTImage(siftPath.string());
			else
			{
				mSiftFlow->createSIFTImage(*mTarget, *mTargetSIFT, std::vector<float>(), mStepSize);
				writeSIFTImage(siftPath.string(), *mTargetSIFT);
			}
			*/

			mSiftFlow->createSIFTImage(*mTarget, *mTargetSIFT, std::vector<float>(), mStepSize);
		}
	}

	void DenseSIFT::loadGrayscaleImage(const std::string& path, cv::Mat& img)
	{
		cv::Mat grayImg = cv::imread(path, cv::IMREAD_GRAYSCALE);
		cv::normalize(grayImg, img, 0, 1, cv::NORM_MINMAX, CV_32F);
	}

	void DenseSIFT::match(std::vector<MatchType>& denseMatches, const std::vector<MatchType>& sparseMatches)
	{
		vector<float> srcScaleMap, tgtScaleMap;
		cv::Mat srcSiftImg, tgtSiftImg;

		// Do scale propgation if requested
		if (!sparseMatches.empty())
		{
			// Extract matched features
			std::vector<FeatureType> srcSparseFeatures(sparseMatches.size());
			std::vector<FeatureType> tgtSparseFeatures(sparseMatches.size());
			for (size_t i = 0; i < sparseMatches.size(); ++i)
			{
				srcSparseFeatures[i] = mSrcSparseFeatures[sparseMatches[i].first];
				tgtSparseFeatures[i] = mTgtSparseFeatures[sparseMatches[i].second];
			}

			// Scale images for scale maps algorithm
			cv::Mat srcImgScaled = *mSource, tgtImgScaled = *mTarget;
			if (mStepSize > 1)
			{
				cv::Size srcScaledSize(mSource->cols / mStepSize, mSource->rows / mStepSize);
				cv::Size tgtScaledSize(mTarget->cols / mStepSize, mTarget->rows / mStepSize);
				cv::resize(*mSource, srcImgScaled, srcScaledSize);
				cv::resize(*mTarget, tgtImgScaled, tgtScaledSize);

				// Scale the features
				for (size_t i = 0; i < sparseMatches.size(); ++i)
				{
					FeatureType& srcFeat = srcSparseFeatures[i];
					FeatureType& tgtFeat = tgtSparseFeatures[i];
					srcFeat.x /= (float)mStepSize;
					srcFeat.y /= (float)mStepSize;
					tgtFeat.x /= (float)mStepSize;
					tgtFeat.y /= (float)mStepSize;
				}
			}

			// Calculate the scale maps for both source and target images
			vector<float> srcScaleMap, tgtScaleMap;
			boost::timer::cpu_timer timer;
			timer.start();
			mScaleMaps->run(srcImgScaled, srcSparseFeatures, srcScaleMap);
			//std::cout << "ScaleMaps time = " << timer.elapsed().wall*1.0e-9f << std::endl;//
			timer.start();
			mScaleMaps->run(tgtImgScaled, tgtSparseFeatures, tgtScaleMap);
			//std::cout << "ScaleMaps time = " << timer.elapsed().wall*1.0e-9f << std::endl;//

			// Create SIFT images
			mSiftFlow->createSIFTImage(*mSource, srcSiftImg, srcScaleMap, mStepSize);
			mSiftFlow->createSIFTImage(*mTarget, tgtSiftImg, tgtScaleMap, mStepSize);
		}
		else
		{
			srcSiftImg = *mSourceSIFT;
			tgtSiftImg = *mTargetSIFT;
		}

		// Do 2-sided SIFT-Flow
		cv::Mat vx, vy;
		mSiftFlow->coarse2fineFlow(srcSiftImg, tgtSiftImg, vx, vy);

		cv::Mat vx_inv, vy_inv;
		mSiftFlow->coarse2fineFlow(tgtSiftImg, srcSiftImg, vx_inv, vy_inv);

		// Find matches
		const double threshold = 1e-6;
		const int border = 20;
		int width = vx.cols, height = vx.rows;
		int r, c, x1, y1, x2, y2, index1, index2;
		double dx, dy, dist;
		float *vx_data = (float*)vx.data, *vy_data = (float*)vy.data,
			*vx_inv_data = (float*)vx_inv.data, *vy_inv_data = (float*)vy_inv.data;
		for (r = border; r < (height - border); ++r)
		{
			for (c = border; c < (width - border); ++c)
			{
				index1 = r*width + c;
				x2 = (int)round(c + vx_data[index1]);
				y2 = (int)round(r + vy_data[index1]);
				if (x2 < 0 || x2 >= width || y2 < 0 || y2 >= height)
					continue;

				// Ignore matches at the same pixel
				//if (x2 == c && y2 == r)	continue;

				// Ignore source's textureless regions
				if (mFilterTexturelessRegions && mSrcTexturelessRegions[index1]) continue;

				index2 = y2*width + x2;
				x1 = (int)round(x2 + vx_inv_data[index2]);
				y1 = (int)round(y2 + vy_inv_data[index2]);
				if (x1 < 0 || x1 >= width || y1 < 0 || y1 >= height)
					continue;

				// Ignore target's textureless regions
				if (mFilterTexturelessRegions && mTgtTexturelessRegions[index2]) continue;

				// Calculate distance
				dx = c - x1;
				dy = r - y1;
				dist = sqrt(dx*dx + dy*dy);

				// Add match within threshold
				if (dist < threshold)
					denseMatches.push_back(std::make_pair(index1, index2));
			}
		}

		//std::cout << "denseMatches.size = " << denseMatches.size() << std::endl;//
	}

}	// namespace dc
