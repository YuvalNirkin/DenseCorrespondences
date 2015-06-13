/************************************************************************************
*									   Includes										*
************************************************************************************/
#include "Utilities.h"
#include <iostream>
#include <openMVG/numeric/numeric.h>

/************************************************************************************
*									  Namespaces									*
************************************************************************************/
using std::cout;
using std::endl;
using std::string;
using namespace openMVG;
using namespace openMVG::SfMIO;

/************************************************************************************
*									 Declarations									*
************************************************************************************/

/************************************************************************************
*									Implementation									*
************************************************************************************/

bool compare (IntrinsicCameraInfo const &ci1, IntrinsicCameraInfo const &ci2)
{
	if (ci1.m_bKnownIntrinsic)
		return ci1.m_K == ci2.m_K;

	bool bequal = false;
	if (ci1.m_sCameraMaker.compare("") != 0 && ci1.m_sCameraModel.compare("") != 0)
	{
		if (ci1.m_sCameraMaker.compare(ci2.m_sCameraMaker) == 0
			&& ci1.m_sCameraModel.compare(ci2.m_sCameraModel) == 0
			&& ci1.m_w == ci2.m_w
			&& ci1.m_h == ci2.m_h
			&& ci1.m_focal == ci2.m_focal)
		{
			bequal = true;
		}
	}
	return bequal;
}

bool readImageList(std::vector<openMVG::SfMIO::CameraInfo> & vec_camImageName,
	std::vector<openMVG::SfMIO::IntrinsicCameraInfo> & vec_focalGroup,
	std::string sFileName, bool bVerbose)
{
	typedef std::set<IntrinsicCameraInfo, IntrinsicCameraInfo> setIntrinsicCameraInfo;
	setIntrinsicCameraInfo set_focalGroup;

	std::ifstream in(sFileName.c_str());
	if (!in.is_open())  {
		std::cerr << std::endl
			<< "Impossible to read the specified file." << std::endl;
	}
	std::string sValue;
	std::vector<std::string> vec_str;
	while (getline(in, sValue))
	{
		vec_str.clear();
		IntrinsicCameraInfo intrinsicCamInfo;
		split(sValue, ";", vec_str);
		if (vec_str.size() == 1)
		{
			std::cerr << "Invalid input file" << std::endl;
			in.close();
			return false;
		}
		std::stringstream oss;
		oss.clear(); oss.str(vec_str[1]);
		size_t width, height;
		oss >> width;
		oss.clear(); oss.str(vec_str[2]);
		oss >> height;

		intrinsicCamInfo.m_w = width;
		intrinsicCamInfo.m_h = height;

		switch (vec_str.size())
		{
		case 3: // a camera without exif data
		{
					intrinsicCamInfo.m_focal = -1;
					intrinsicCamInfo.m_bKnownIntrinsic = false;
					intrinsicCamInfo.m_sCameraMaker = "";
					intrinsicCamInfo.m_sCameraModel = "";
		}
			break;
		case 5: // a camera with exif data found in the database
		{
					intrinsicCamInfo.m_focal = -1;
					intrinsicCamInfo.m_bKnownIntrinsic = false;
					intrinsicCamInfo.m_sCameraMaker = vec_str[3];
					intrinsicCamInfo.m_sCameraModel = vec_str[4];
		}
			break;
		case  6: // a camera with exif data not found in the database
		{
					 oss.clear(); oss.str(vec_str[3]);
					 float focal;
					 oss >> focal;
					 intrinsicCamInfo.m_focal = focal;
					 intrinsicCamInfo.m_bKnownIntrinsic = true;
					 intrinsicCamInfo.m_sCameraMaker = vec_str[4];
					 intrinsicCamInfo.m_sCameraModel = vec_str[5];

					 Mat3 K;
					 K << focal, 0, float(width) / 2.f,
						 0, focal, float(height) / 2.f,
						 0, 0, 1;
					 intrinsicCamInfo.m_K = K;

		}
			break;
		case 12: // a camera with known intrinsic
		{
					 intrinsicCamInfo.m_bKnownIntrinsic = true;
					 intrinsicCamInfo.m_sCameraMaker = intrinsicCamInfo.m_sCameraModel = "";

					 Mat3 K = Mat3::Identity();

					 oss.clear(); oss.str(vec_str[3]);
					 oss >> K(0, 0);
					 oss.clear(); oss.str(vec_str[4]);
					 oss >> K(0, 1);
					 oss.clear(); oss.str(vec_str[5]);
					 oss >> K(0, 2);
					 oss.clear(); oss.str(vec_str[6]);
					 oss >> K(1, 0);
					 oss.clear(); oss.str(vec_str[7]);
					 oss >> K(1, 1);
					 oss.clear(); oss.str(vec_str[8]);
					 oss >> K(1, 2);
					 oss.clear(); oss.str(vec_str[9]);
					 oss >> K(2, 0);
					 oss.clear(); oss.str(vec_str[10]);
					 oss >> K(2, 1);
					 oss.clear(); oss.str(vec_str[11]);
					 oss >> K(2, 2);

					 intrinsicCamInfo.m_K = K;
					 intrinsicCamInfo.m_focal = static_cast<float>(K(0, 0)); // unkown sensor size;
		}
			break;
		default:
		{
				   std::cerr << "Invalid line : wrong number of arguments" << std::endl;
		}
		}

		// Check if the current intrinsic parameters already exist
		bool bNewFocalGroup = true;
		int id = vec_focalGroup.size();
		for (int i = 0; i < vec_focalGroup.size(); ++i)
		{
			if (compare(vec_focalGroup[i], intrinsicCamInfo))
			{
				bNewFocalGroup = false;
				id = i;
				break;
			}
		}

		//std::pair<setIntrinsicCameraInfo::iterator, bool> ret = set_focalGroup.insert(intrinsicCamInfo);
		if (bNewFocalGroup)
		{
			vec_focalGroup.push_back(intrinsicCamInfo);
		}
		//size_t id = std::distance(ret.first, set_focalGroup.end()) - 1;
		CameraInfo camInfo;
		camInfo.m_sImageName = vec_str[0];
		camInfo.m_intrinsicId = id;
		vec_camImageName.push_back(camInfo);

		vec_str.clear();
	}
	in.close();
	return !(vec_camImageName.empty());
}

