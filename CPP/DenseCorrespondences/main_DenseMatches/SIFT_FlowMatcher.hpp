#ifndef __SIFT_FLOW_MATCHER__
#define __SIFT_FLOW_MATCHER__

/************************************************************************************
*									   Includes										*
************************************************************************************/
#include <vector>
#include <string>
#include <set>
#include <DenseSIFT.h>
#include "openMVG/matching_image_collection/Matcher.hpp"

/************************************************************************************
*									 Declarations									*
************************************************************************************/
namespace openMVG
{
	class SIFT_FlowMatcher : Matcher
	{
	public:

		/// Load all features and descriptors in memory
		bool loadData(
			const std::vector<std::string> & vec_fileNames, // input filenames
			const  std::vector<std::pair<size_t, size_t> >& vec_imagesSize,	// image sizes (width, height)
			int stepSize,
			const std::string& sSparseMatchesDir,
			const std::string & sDenseMatchesDir,
			bool bScaleMaps = true) // where the data are saved
		{
			if (vec_fileNames.empty()) return false;

			m_bScaleMaps = bScaleMaps;

			// Get all the SIFT image paths
			mSIFTImgPaths.clear();
			for (size_t j = 0; j < vec_fileNames.size(); ++j)
			{	
				std::string sSIFTImgPath = stlplus::create_filespec(sDenseMatchesDir,
					stlplus::basename_part(vec_fileNames[j]), "sift");
				mSIFTImgPaths.push_back(sSIFTImgPath);

				std::string sFeat = stlplus::create_filespec(sDenseMatchesDir,
					stlplus::basename_part(vec_fileNames[j]), "feat");
				mFeatPaths.push_back(sFeat);
			}

			// Adjust to SIFT image sizes
			mImgSizes.reserve(vec_imagesSize.size());
			for (int i = 0; i < vec_imagesSize.size(); ++i)
			{
				mImgSizes.push_back(make_pair(
					(size_t)vec_imagesSize[i].first / stepSize,
					(size_t)vec_imagesSize[i].second / stepSize));
			}

			mStepSize = stepSize;

			// Read sparse matches (Fundamental)
			std::string sSparseMatchesFile_F = stlplus::create_filespec(sSparseMatchesDir, "matches.f", "txt");
			if (!stlplus::is_file(sSparseMatchesFile_F))
			{
				std::cerr << std::endl
					<< "The file \"matches.f.txt\" is missing from the sparseMatches directory!" << std::endl;
				return false;
			}
			if (!matching::PairedIndMatchImport(sSparseMatchesFile_F, m_mapSparseMatches_F)) {
				std::cerr << "Unable to read the Fundamental matrix matches" << std::endl;
				return false;
			}

			// Read sparse features
			for (size_t i = 0; i < vec_fileNames.size(); ++i)  {
				std::string sFeat = stlplus::create_filespec(sSparseMatchesDir,
					stlplus::basename_part(vec_fileNames[i]), "feat");
				if (!loadFeatsFromFile(sFeat, m_mapSparseFeatures[i])) {
					std::cerr << "Bad reading of feature files" << std::endl;
					return false;
				}
			}

			return true;
		}

		/// Build point indexes correspondences lists between images ids
		void Match(
			const std::vector<std::string> & vec_fileNames,
			matching::PairWiseMatches & map_putatives_matches // the output pairwise photometric corresponding points
			)const
		{
			// Get an instance of DenseSIFT
			dc::DenseSIFT& denseSIFT = dc::DenseSIFT::getInstance();

#ifdef USE_OPENMP
			std::cout << "Using the OPENMP thread interface" << std::endl;
#endif
			C_Progress_display my_progress_bar(vec_fileNames.size()*(vec_fileNames.size() - 1) / 2.0);

			size_t imgNum = vec_fileNames.size();
			std::vector<std::set<int>> allFeatures(vec_fileNames.size());
			std::map<std::pair<size_t, size_t>, std::vector<std::pair<size_t, size_t>>> allMatches;
			size_t i, j, k;
			for (size_t i = 0; i < vec_fileNames.size(); ++i)
			{
				// Set source image with features
				if (m_bScaleMaps)
					denseSIFT.setSource(vec_fileNames[i], m_mapSparseFeatures.at(i));
				else denseSIFT.setSource(vec_fileNames[i]);

#ifdef USE_OPENMP
#pragma omp parallel for schedule(dynamic, 1)
#endif
				for (j = i + 1; j < vec_fileNames.size(); ++j)
				{
					matching::PairWiseMatches::const_iterator it = m_mapSparseMatches_F.find(std::make_pair(i, j));
					if (it == m_mapSparseMatches_F.end())
					{
						++my_progress_bar;
						continue;
					}

					// Match with scale maps
					std::vector<dc::MatchType> matches;
					if (m_bScaleMaps)
					{
						// Set target image with features
						denseSIFT.setTarget(vec_fileNames[j], m_mapSparseFeatures.at(j));

						// Convert sparse matches
						const std::vector<matching::IndMatch>& indMatches = m_mapSparseMatches_F.at(std::make_pair(i, j));
						vector<dc::MatchType> sparseMatches(indMatches.size());
						memcpy(sparseMatches.data(), indMatches.data(), sparseMatches.size()*sizeof(IndMatch));

						// Do matching
						denseSIFT.match(matches, sparseMatches);
					}
					else // Match without scale maps
					{
						// Set target image
						denseSIFT.setTarget(vec_fileNames[j]);

						// Do matching
						denseSIFT.match(matches);
					}

					// Rescale 

					// Add features
					std::set<int>& set1(allFeatures[i]);
					std::set<int>& set2(allFeatures[j]);
					for (k = 0; k < matches.size(); ++k)
					{
						std::pair<size_t, size_t>& match = matches[k];
						set1.insert(match.first);	// Index of SIFT image
						set2.insert(match.second);	// Index of SIFT image
					}

					// Convert format
					std::vector<IndMatch> vec_Matches(matches.size());
					memcpy(vec_Matches.data(), matches.data(), matches.size()*sizeof(IndMatch));				

#ifdef USE_OPENMP
#pragma omp critical
#endif
					{
						//allMatches.insert(make_pair(make_pair(i, j), matches));
						map_putatives_matches.insert(make_pair(make_pair(i, j), vec_Matches));
					}

					// Report progress
					++my_progress_bar;
				}
			}

			// Save features (they are only known after the matching)
			{
				std::cout << "Saving features" << std::endl;
				std::set<int>::iterator it;
				size_t width, height;
				for (i = 0; i < allFeatures.size(); ++i)
				{
					std::set<int>& indices(allFeatures[i]);
					std::vector<SIOPointFeature> features;
					features.reserve(indices.size());
					width = mImgSizes[i].first;
					height = mImgSizes[i].second;
					for (it = indices.begin(); it != indices.end(); ++it)
					{
						features.push_back(SIOPointFeature((*it % width)*mStepSize,
							(*it / width)*mStepSize, 3.0f, 0.0f));
					}

					saveFeatsToFile(mFeatPaths[i], features);
				}
			}

			// Build index map
			std::vector<std::map<size_t, size_t>> indexMaps(allFeatures.size());
			for (i = 0; i < allFeatures.size(); ++i)
			{
				std::set<int>& indices = allFeatures[i];
				std::map<size_t, size_t>& indMap = indexMaps[i];

				std::set<int>::iterator it;
				int counter = 0;
				for (it = indices.begin(); it != indices.end(); ++it)
				{
					// max efficiency inserting
					indMap.insert(indMap.end(), std::pair<size_t, size_t>(*it, counter++));
				}
			}
							
			// Convert matches from image pixel indices to feature indices
			matching::PairWiseMatches::iterator it;
			std::set<int>::iterator sit;
			for (it = map_putatives_matches.begin(); it != map_putatives_matches.end(); ++it)
			{
				std::map<size_t, size_t>& indMap1 = indexMaps[it->first.first];
				std::map<size_t, size_t>& indMap2 = indexMaps[it->first.second];
				std::vector<IndMatch>& matches = it->second;

				for (i = 0; i < matches.size(); ++i)
				{
					IndMatch& match = matches[i];
					match._i = indMap1[match._i];
					match._j = indMap2[match._j];
				}
			}
		}

	private:
		std::vector<std::string> mSIFTImgPaths;
		std::vector<std::string> mFeatPaths;
		std::vector<std::pair<size_t, size_t>> mImgSizes;	// (width, height)
		int mStepSize;
		//std::map<size_t, std::vector<SIOPointFeature>> m_mapSparseFeatures; ///< Feature per images
		std::map<size_t, std::vector<dc::FeatureType>> m_mapSparseFeatures; ///< Feature per images
		matching::PairWiseMatches m_mapSparseMatches_F;
		bool m_bScaleMaps;
	};
}	// namespace openMVG

#endif	// __SIFT_FLOW_MATCHER__