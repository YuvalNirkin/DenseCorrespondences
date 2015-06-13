#ifndef __UTILITIES__
#define __UTILITIES__

/************************************************************************************
*									   Includes										*
************************************************************************************/
#include <string>
#include <vector>
#include "software/SfM/SfMIOHelper.hpp"

/************************************************************************************
*									 Declarations									*
************************************************************************************/


/************************************************************************************
*										Classes										*
************************************************************************************/

bool readImageList(std::vector<openMVG::SfMIO::CameraInfo> & vec_camImageName,
	std::vector<openMVG::SfMIO::IntrinsicCameraInfo> & vec_focalGroup,
	std::string sFileName, bool bVerbose = true);

#endif	// __UTILITIES__