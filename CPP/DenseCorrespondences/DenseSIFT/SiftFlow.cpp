/************************************************************************************
*									   Includes										*
************************************************************************************/
#include "SiftFlow.h"
#include "ImageFeature.h"
//#include "Image.h"
#include "BPFlow.h"
#include <fstream>	// Debug
#include <boost\format.hpp>	// Debug
#include <vl\mathop.h>
#include <vl\sift.h>
#include <opencv2\core.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\xfeatures2d.hpp>

/************************************************************************************
*									  Namespaces									*
************************************************************************************/
using std::ofstream;	// Debug

/************************************************************************************
*									 Declarations									*
************************************************************************************/
int ncols = 0;
#define MAXCOLS 60
int colorwheel[MAXCOLS][3];

/************************************************************************************
*									Implementation									*
************************************************************************************/
namespace dc
{
	SiftFlow::SiftFlow(double d, int nlevels, double alpha,
		double gamma, int nIterations, int nTopIterations,
		int wsize, int topwsize) :
		d(d),
		nlevels(nlevels),
		alpha(alpha),
		gamma(gamma),
		nIterations(nIterations),
		nTopIterations(nTopIterations),
		wsize(wsize),
		topwsize(topwsize)
	{
	}

	SiftFlow::SiftFlow(int nlevels, double alpha,
		double gamma, int nIterations, int nTopIterations,
		int wsize, int topwsize) :
		d(alpha * 20),
		nlevels(nlevels),
		alpha(alpha),
		gamma(gamma),
		nIterations(nIterations),
		nTopIterations(nTopIterations),
		wsize(wsize),
		topwsize(topwsize)
	{
	}

	void SiftFlow::init(double d, int nlevels, double alpha,
		double gamma, int nIterations, int nTopIterations,
		int wsize, int topwsize)
	{
		this->d = d;
		this->nlevels = nlevels;
		this->alpha = alpha;
		this->gamma = gamma;
		this->nIterations = nIterations;
		this->nTopIterations = nTopIterations;
		this->wsize = wsize;
		this->topwsize = topwsize;
	}

	void SiftFlow::init(int nlevels, double alpha,
		double gamma, int nIterations, int nTopIterations,
		int wsize, int topwsize)
	{
		init(alpha * 20, nlevels, alpha, gamma, nIterations, nTopIterations,
			wsize, topwsize);
	}

	VL_INLINE void transpose_descriptor(vl_sift_pix* dst, vl_sift_pix* src)
	{
		int const BO = 8;  /* number of orientation bins */
		int const BP = 4;  /* number of spatial bins     */
		int i, j, t;

		for (j = 0; j < BP; ++j) {
			int jp = BP - 1 - j;
			for (i = 0; i < BP; ++i) {
				int o = BO * i + BP*BO * j;
				int op = BO * i + BP*BO * jp;
				dst[op] = src[o];
				for (t = 1; t < BO; ++t)
					dst[BO - t + op] = src[t + o];
			}
		}
	}

	/*
	void SiftFlow::createSIFTImage(const cv::Mat& img, cv::Mat& siftImg,
		const std::vector<float>& scaleMap, int stepSize)
	{
		// VL SIFT computation
		vl_sift_pix const *data = (vl_sift_pix*)img.data;
		//int M = imgHeight;
		//int N = imgWidth;
		int M = img.cols;	// Width
		int N = img.rows;	// Height

		// VL SIFT PARAMETERS
		int                verbose = 0; // change to 2 for more verbose..
		int                O = -1; //Octaves
		int                S = 3; //Levels
		int                o_min = 0;

		double             edge_thresh = -1;  //-1 will use the default (as in matlab)
		double             peak_thresh = -1;
		double             norm_thresh = -1;
		double             magnif = 1;	// -1
		double             window_size = -1;

		vl_bool            force_orientations = false;
		vl_bool            floatDescriptors = true;

		// Set keypoints
		//double            *ikeys = 0; //?
		//int                nikeys = -1; //?
		int nikeys = (img.cols / stepSize)*(img.rows / stepSize);
		double* ikeys = new double[nikeys * 4];
		int r, c, index = 0, scaleIndex = 0;
		if (scaleMap.empty())
		{
			for (r = 0; r < img.rows; r += stepSize)
			{
				for (c = 0; c < img.cols; c += stepSize)
				{
					ikeys[index++] = (double)c;	// x
					ikeys[index++] = (double)r;	// y
					ikeys[index++] = (double)3.0;
					ikeys[index++] = (double)0;
				}
			}
		}
		else
		{
			for (r = 0; r < img.rows; r += stepSize)
			{
				for (c = 0; c < img.cols; c += stepSize)
				{
					ikeys[index++] = (double)c;	// x
					ikeys[index++] = (double)r;	// y
					ikeys[index++] = (double)scaleMap[scaleIndex++];
					ikeys[index++] = (double)0;
				}
			}
		}

		// Allocate sift image
		//siftImg.allocate(img.cols / stepSize, img.rows / stepSize, 128);
		siftImg.create(img.rows / stepSize, img.cols / stepSize, CV_32FC(128));

		//////////////////////////////////////////////////////////////////////////////////
		// Do job
		//////////////////////////////////////////////////////////////////////////////////
		vl_bool            first;
		double            *frames = 0;
		int				  nframes = 0;
		//vl_uint8          *descr = 0;
		vl_uint8          *descr = siftImg.data;
		int                reserved = 0, i, j, q;

		// create a filter to process the image
		VlSiftFilt* filt = vl_sift_new(M, N, O, S, o_min);

		if (peak_thresh >= 0) vl_sift_set_peak_thresh(filt, peak_thresh);
		if (edge_thresh >= 0) vl_sift_set_edge_thresh(filt, edge_thresh);
		if (norm_thresh >= 0) vl_sift_set_norm_thresh(filt, norm_thresh);
		if (magnif >= 0) vl_sift_set_magnif(filt, magnif);
		if (window_size >= 0) vl_sift_set_window_size(filt, window_size);

		//////////////////////////////////////////////////////////////////////////////////
		// Process each octave
		//////////////////////////////////////////////////////////////////////////////////
		i = 0;
		first = 1;
		while (1)
		{
			int                   err;
			VlSiftKeypoint const *keys = 0;
			int                   nkeys = 0;

			if (verbose) {
				printf("vl_sift: processing octave %d\n",
					vl_sift_get_octave_index(filt));
			}

			// Calculate the GSS for the next octave ....................
			if (first) {
				err = vl_sift_process_first_octave(filt, data);
				first = 0;
			}
			else {
				err = vl_sift_process_next_octave(filt);
			}

			if (err) break;

			if (verbose > 1) {
				printf("vl_sift: GSS octave %d computed\n",
					vl_sift_get_octave_index(filt));
			}

			// Run detector .............................................
			if (nikeys < 0) {
				vl_sift_detect(filt);

				keys = vl_sift_get_keypoints(filt);
				nkeys = vl_sift_get_nkeypoints(filt);
				i = 0;

				if (verbose > 1) {
					printf("vl_sift: detected %d (unoriented) keypoints\n", nkeys);
				}
			}
			else {
				nkeys = nikeys;
			}

			// For each keypoint ........................................
			for (; i < nkeys; ++i) {
				double                angles[4];
				int                   nangles;
				VlSiftKeypoint        ik;
				VlSiftKeypoint const *k;

				// Obtain keypoint orientations ...........................
				if (nikeys >= 0) {
					
					//vl_sift_keypoint_init(filt, &ik,
					//ikeys[4 * i + 1] - 1,	// Make sure this is not just for MATLAB
					//ikeys[4 * i + 0] - 1,
					//ikeys[4 * i + 2]);
					
					vl_sift_keypoint_init(filt, &ik,
						ikeys[4 * i + 0],
						ikeys[4 * i + 1],
						ikeys[4 * i + 2]);

					if (ik.o != vl_sift_get_octave_index(filt)) {
						break;
					}

					k = &ik;

					// optionally compute orientations too
					if (force_orientations) {
						nangles = vl_sift_calc_keypoint_orientations(filt, angles, k);
					}
					else {
						angles[0] = VL_PI / 2 - ikeys[4 * i + 3];
						nangles = 1;
					}
				}
				else {
					k = keys + i;
					nangles = vl_sift_calc_keypoint_orientations
						(filt, angles, k);
				}

				// For each orientation ...................................
				for (q = 0; q < nangles; ++q) {
					vl_sift_pix  buf[128];
					vl_sift_pix rbuf[128];

					// compute descriptor (if necessary)
					//if (nout > 1) {
					vl_sift_calc_keypoint_descriptor(filt, buf, k, angles[q]);
					transpose_descriptor(rbuf, buf);
					//}

					// make enough room for all these keypoints and more 
					if (reserved < nframes + 1) {
						reserved += 2 * nkeys;
						frames = (double*)realloc(frames, 4 * sizeof(double)* reserved);
						//if (nout > 1) {
						if (!floatDescriptors) {
							// SIFT image already allocated
							//descr = (vl_uint8*)realloc(descr, 128 * sizeof(vl_uint8)* reserved);
						}
						else {
							//descr = (float*)realloc(descr, 128 * sizeof(float)* reserved);
						}
						//}
					}

					// Save back with MATLAB conventions. Notice tha the input
					// image was the transpose of the actual image.
					frames[4 * nframes + 0] = k->y + 1;
					frames[4 * nframes + 1] = k->x + 1;
					frames[4 * nframes + 2] = k->sigma;
					frames[4 * nframes + 3] = VL_PI / 2 - angles[q];

					//if (nout > 1) {
					if (!floatDescriptors) {
						for (j = 0; j < 128; ++j) {
							float x = 512.0F * rbuf[j];
							x = (x < 255.0F) ? x : 255.0F;
							//((vl_uint8*)descr)[128 * nframes + j] = (vl_uint8)x;
							((float*)descr)[128 * nframes + j] = (vl_uint8)x;
						}
					}
					else {
						for (j = 0; j < 128; ++j) {
							float x = 512.0F * rbuf[j];
							((float*)descr)[128 * nframes + j] = x;
						}
					}
					//}

					++nframes;
				} // next orientation
			} // next keypoint
		} // next octave

		if (verbose) {
			printf("vl_sift: found %d keypoints\n", nframes);
		}

		// save variables:
		//memcpy(DATAframes, frames, 4 * (*nframes) * sizeof(double));
		//memcpy(DATAdescr, descr, 128 * (*nframes) * sizeof(vl_uint8));

		// cleanup
		vl_sift_delete(filt);
		delete[] ikeys;
	}
	*/

	void SiftFlow::createSIFTImage(const cv::Mat& img, cv::Mat& siftImg,
		const std::vector<float>& scaleMap, int stepSize)
	{
		cv::Ptr< cv::xfeatures2d::SIFT> siftExtractor = cv::xfeatures2d::SIFT::create();
		int r, c, index = 0;

		std::vector<cv::KeyPoint> keypoints((img.rows / stepSize)*(img.cols / stepSize));

		if (scaleMap.empty())
		{
			for (r = 0; r < img.rows; r += stepSize)
			{
				for (c = 0; c < img.cols; c += stepSize)
				{
					cv::KeyPoint& key = keypoints[index++];
					key.pt.x = c;
					key.pt.y = r;
					key.size = 3.0f;
				}
			}
		}
		else
		{
			for (r = 0; r < img.rows; r += stepSize)
			{
				for (c = 0; c < img.cols; c += stepSize)
				{
					cv::KeyPoint& key = keypoints[index];
					key.pt.x = c;
					key.pt.y = r;
					key.size = scaleMap[index];

					++index;
				}
			}
		}

		cv::Mat desc;

		cv::Mat bimg;
		img.convertTo(bimg, CV_8U);
		siftExtractor->compute(bimg, keypoints, desc);

		// Output sift image
		siftImg.create(img.rows / stepSize, img.cols / stepSize, CV_32FC(128));
		memcpy(siftImg.data, desc.data, desc.total()*sizeof(float));
	}

	void SiftFlow::createPyramids(const cv::Mat& img, std::vector<cv::Mat>& pyramids, 
		int levels)
	{
		cv::Mat tmp;
		int w, h;

		// Initialize
		pyramids.resize(levels);
		pyramids[0] = img;

		// For each pyramid level starting from level 1
		for (size_t i = 1; i < nlevels; ++i)
		{
			// Apply gaussian filter
			cv::GaussianBlur(pyramids[i - 1], tmp, cv::Size(5, 5), 0.67, 0,
				cv::BORDER_REPLICATE);

			// Resize image using bicubic interpolation
			w = (int)ceil(pyramids[i - 1].cols / 2.0f);
			h = (int)ceil(pyramids[i - 1].rows / 2.0f);
			cv::resize(tmp, pyramids[i], cv::Size(w, h), 0, 0, cv::INTER_CUBIC);
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////
	//									  flow2color									//
	//////////////////////////////////////////////////////////////////////////////////////

	bool unknown_flow(float u, float v) {
		const double UNKNOWN_FLOW_THRESH = 1e9;
		return (fabs(u) >  UNKNOWN_FLOW_THRESH)
			|| (fabs(v) >  UNKNOWN_FLOW_THRESH)
			|| isnan(u) || isnan(v);
	}

	void setcols(int r, int g, int b, int k)
	{
		colorwheel[k][0] = r;
		colorwheel[k][1] = g;
		colorwheel[k][2] = b;
	}

	void makeColorWheel()
	{
		// relative lengths of color transitions:
		// these are chosen based on perceptual similarity
		// (e.g. one can distinguish more shades between red and yellow 
		//  than between yellow and green)
		int RY = 15;
		int YG = 6;
		int GC = 4;
		int CB = 11;
		int BM = 13;
		int MR = 6;
		ncols = RY + YG + GC + CB + BM + MR;
		//printf("ncols = %d\n", ncols);
		if (ncols > MAXCOLS)
			exit(1);
		int i;
		int k = 0;
		for (i = 0; i < RY; i++) setcols(255, 255 * i / RY, 0, k++);
		for (i = 0; i < YG; i++) setcols(255 - 255 * i / YG, 255, 0, k++);
		for (i = 0; i < GC; i++) setcols(0, 255, 255 * i / GC, k++);
		for (i = 0; i < CB; i++) setcols(0, 255 - 255 * i / CB, 255, k++);
		for (i = 0; i < BM; i++) setcols(255 * i / BM, 0, 255, k++);
		for (i = 0; i < MR; i++) setcols(255, 0, 255 - 255 * i / MR, k++);
	}

	void computeColor(double* u, double* v, uchar* imgData, int npixels)
	{
		if (ncols == 0)
			makeColorWheel();

		float rad, a, fl, f, fk;
		int k0, k1;
		uchar* pImg = imgData;

		for (int i = 0; i < npixels; ++i)
		{
			rad = sqrt(u[i] * u[i] + v[i] * v[i]);
			a = atan2(-v[i], -u[i]) / M_PI;
			fk = (a + 1.0) / 2.0 * (ncols - 1);
			k0 = (int)fk;
			k1 = (k0 + 1) % ncols;
			f = fk - k0;
			//f = 0; // uncomment to see original color wheel
			for (int b = 0; b < 3; b++) {
				float col0 = colorwheel[k0][b] / 255.0;
				float col1 = colorwheel[k1][b] / 255.0;
				float col = (1 - f) * col0 + f * col1;
				if (rad <= 1)
					col = 1 - rad * (1 - col); // increase saturation with radius
				else
					col *= .75; // out of range
				pImg[2 - b] = (int)(255.0 * col);
			}
			pImg += 3;
		}

	}

	// [u v u v u v ...]
	cv::Mat SiftFlow::flow2Color(const DImage& flow, double maxFlow)
	{

		const double EPS = 1e-6;
		double maxu = -9999, maxv = -9999;
		double minu = 9999, minv = 9999;
		double maxrad = -1;

		// Extract flow parameters
		int npixels = flow.npixels();
		double* u = new double[npixels];
		double* v = new double[npixels];
		double* rad = new double[npixels];
		for (int i = 0; i < npixels; ++i)
		{
			u[i] = flow.data()[i * 2];
			v[i] = flow.data()[i * 2 + 1];

			// Fix unknown flow
			if (unknown_flow(u[i], v[i]))
				u[i] = v[i] = 0;

			// Find min and max values for both channels
			maxu = max(maxu, u[i]);
			minu = min(minu, u[i]);
			maxv = max(maxv, v[i]);
			minv = min(minv, v[i]);

			// Calculate flow vectors lengths and max flow vectors length
			rad[i] = sqrt(u[i] * u[i] + v[i] * v[i]);
			maxrad = max(maxrad, rad[i]);
		}

		//printf("max flow: %.4f flow range: u = %.3f .. %.3f; v = %.3f .. %.3f\n", maxrad, minu, maxu, minv, maxv);

		// If max flow is specified than use it instead
		if (maxFlow > 0) maxrad = maxFlow;

		// Normalize flow vectors
		double normFactor = maxrad + EPS;
		for (int i = 0; i < npixels; ++i)
		{
			u[i] /= normFactor;
			v[i] /= normFactor;
		}

		// Compute color
		cv::Mat colorImg(flow.height(), flow.width(), CV_8UC3);
		computeColor(u, v, colorImg.data, npixels);

		// Release allocated memory
		delete[] u;
		delete[] v;
		delete[] rad;

		return colorImg;
	}

	void SiftFlow::discreteFlow(const cv::Mat& siftImg1, const cv::Mat& siftImg2,
		DImage& flow, double gamma, int nIterations, int nHierarchy, int wsize,
		const cv::Mat& OffsetX, const cv::Mat& OffsetY,
		const IntImage& WinSizeX, const IntImage& WinSizeY)
	{
		BPFlow bpflow;
		bpflow.LoadImages(siftImg1.cols, siftImg1.rows, siftImg1.channels(),
			(float*)siftImg1.data, siftImg2.cols, siftImg2.rows, (float*)siftImg2.data);

		double* pEnergyList = NULL;

		//		if (false)
		//			bpflow.setPara(Im_s, Im_d);
		//		else
		bpflow.setPara(alpha, d);

		bpflow.setHomogeneousMRF(wsize); // first assume homogeneous setup

		if (!(OffsetX.empty() > 0 || OffsetY.empty()))
			bpflow.LoadOffset((float*)OffsetX.data, (float*)OffsetY.data);

		if (WinSizeX.nelements() > 0 && WinSizeY.nelements())
			bpflow.LoadWinSize(WinSizeX.data(), WinSizeY.data());

		// Stop bpFlow from printing to console
		bpflow.setDisplay(false);

		bpflow.ComputeDataTerm();
		bpflow.ComputeRangeTerm(gamma);

		bpflow.MessagePassing(nIterations, nHierarchy, pEnergyList);
		bpflow.ComputeVelocity();

		// Show flow
		//Mat colorImg = flow2Color(bpflow.flow());
		//cv::imshow("Flow", colorImg);
		//cv::waitKey(0);

		// Return flow
		flow = bpflow.flow();
	}

	//////////////////////////////////////////////////////////////////////////////////////
	//									coarse2fineFlow									//
	//////////////////////////////////////////////////////////////////////////////////////

	void BiImage2DImage(const BiImage& src, DImage& dst)
	{
		int width = src.width();
		int height = src.height();
		int channels = src.nchannels();
		dst.allocate(width, height, channels);

		unsigned char* src_data = src.pData;
		double* dst_data = dst.pData;
		for (int i = 0; i<height; i++)
		for (int j = 0; j<width; j++)
		for (int k = 0; k < channels; k++)
			*dst_data++ = (double)(*src_data++);
	}

	void DImage2BiImage(const DImage& src, BiImage &dst)
	{
		int width = src.width();
		int height = src.height();
		int channels = src.nchannels();
		dst.allocate(width, height, channels);

		double* src_data = src.pData;
		unsigned char* dst_data = dst.pData;
		for (int i = 0; i<height; i++)
		for (int j = 0; j<width; j++)
		for (int k = 0; k < channels; k++)
			*dst_data++ = (unsigned char)round(*src_data++);
	}

	void DImage2IntImage(const DImage& src, IntImage &dst)
	{
		int width = src.width();
		int height = src.height();
		int channels = src.nchannels();
		dst.allocate(width, height, channels);

		double* src_data = src.pData;
		short* dst_data = dst.pData;
		for (int i = 0; i<height; i++)
		for (int j = 0; j<width; j++)
		for (int k = 0; k < channels; k++)
			*dst_data++ = (short)round(*src_data++);
	}

	void downOffset(cv::Mat& offset, int width, int height)
	{
		cv::resize(offset, offset, cv::Size(width, height), 0, 0, cv::INTER_CUBIC);
		float* off_data = (float*)offset.data;
		int pixels = offset.total();
		float val;
		for (int i = 0; i < pixels; ++i)
		{
			val = *off_data;
			*off_data++ = std::round(val * 2);
		}
	}

	void SiftFlow::coarse2fineFlow(const cv::Mat& siftImg1, const cv::Mat& siftImg2,
		cv::Mat& vx, cv::Mat& vy)
	{
		int gammaFact = 1;
		int width, height, channels, width2, height2, pixels;
		int r, c;
		DImage flow;

		// Build pyramids
		vector<cv::Mat> pyrd1(nlevels), pyrd2(nlevels);
		createPyramids(siftImg1, pyrd1, nlevels);
		createPyramids(siftImg2, pyrd2, nlevels);

		// For each level
		for (int i = nlevels - 1; i >= 0; --i)
		{
			gammaFact = 1 << i;	// gammaFact = 2^i;
			width = pyrd1[i].cols;
			height = pyrd1[i].rows;
			channels = pyrd1[i].channels();
			width2 = pyrd2[i].cols;
			height2 = pyrd2[i].rows;
			pixels = pyrd1[i].total();

			if (i == (nlevels - 1))
			{
				IntImage WinSizeX(topwsize, width, height, 1);
				IntImage WinSizeY(topwsize, width, height, 1);

				vx = cv::Mat(height, width, CV_32F, 0.0f);
				vy = cv::Mat(height, width, CV_32F, 0.0f);

				discreteFlow(pyrd1[i], pyrd2[i], flow, gamma*gammaFact,
					nTopIterations, 2, topwsize, vx, vy, WinSizeX, WinSizeY);
			}
			else
			{
				IntImage WinSizeX(wsize + i, width, height, 1);
				IntImage WinSizeY(wsize + i, width, height, 1);

				// Calculate vx, vy
				downOffset(vx, width, height);
				downOffset(vy, width, height);			

				discreteFlow(pyrd1[i], pyrd2[i], flow, gamma*gammaFact,
					nIterations, 2, wsize, vx, vy, WinSizeX, WinSizeY);
			}

			// Set vx vy
			float *vx_data = (float*)vx.data, *vy_data = (float*)vy.data;
			for (int j = 0; j < pixels; ++j)
			{
				*vx_data++ = (float)flow.data()[j * 2];
				*vy_data++ = (float)flow.data()[j * 2 + 1];
			}

			/*/// Debug ///
			cv:Mat_<double> vx_debug(height, width, vx.pData), vy_debug(height, width, vy.pData);
			debug << "vx_c{" << i + 1 << "} = " << vx_debug << ";" << endl;
			debug << "vy_c{" << i + 1 << "} = " << vy_debug << ";" << endl;
			/////////////*/
		}
	}

}	// namespace dc