#ifndef __DENSE_SIFT_TYPES__
#define __DENSE_SIFT_TYPES__

/************************************************************************************
*									   Includes										*
************************************************************************************/
#include "DenseSIFT_Export.h"
#include <vector>
#include <fstream>

/************************************************************************************
*									 Declarations									*
************************************************************************************/
namespace dc
{
	struct DENSE_SIFT_API FeatureType
	{
		float x, y;
		float scale;
		float orient;

		bool operator ==(const FeatureType& f) const;
	};

	DENSE_SIFT_API std::istream& operator>>(std::istream& is, FeatureType& feature);

	typedef std::pair<size_t, size_t> MatchType;
}	// namespace dc

#endif	// __DENSE_SIFT_TYPES__