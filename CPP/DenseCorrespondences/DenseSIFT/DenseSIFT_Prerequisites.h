#ifndef __DENSE_SIFT_PREREQUISITES__
#define __DENSE_SIFT_PREREQUISITES__

/************************************************************************************
*									   Includes										*
************************************************************************************/

/************************************************************************************
*								 Forward Declarations								*
************************************************************************************/
// Allows use of pointers in header files without including individual .h
// so decreases dependencies between files

namespace dc
{
	// DenseSIFT classes
	class DenseSIFT;
	class ScaleMaps;
	class SiftFlow;

}	// namespace dc

namespace cv
{
	class Mat;
}

#endif	// __DENSE_SIFT_PREREQUISITES__