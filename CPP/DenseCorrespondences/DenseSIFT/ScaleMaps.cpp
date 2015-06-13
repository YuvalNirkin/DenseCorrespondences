/************************************************************************************
*									   Includes										*
************************************************************************************/
#include "ScaleMaps.h"
#include <numeric>
#include <algorithm>

// OpenCV (for handling images)
#include <opencv2\core.hpp>

// Eigen (for solving the sparse linear system)
#include <Eigen/SparseCore>
#include <Eigen/Sparse>

/************************************************************************************
*									  Namespaces									*
************************************************************************************/
using std::vector;

/************************************************************************************
*									 Declarations									*
************************************************************************************/
const float EPS = 1e-7;

/************************************************************************************
*									Implementation									*
************************************************************************************/
namespace dc
{
	// Values must be of size 2 or greater
	template <typename T>
	void getStat(const vector<T>& values, size_t size, T& mean, T& var)
	{
		T sum = std::accumulate(values.begin(), values.begin() + size, 0.0);
		mean = sum / size;

		T accum = 0.0;
		std::for_each(values.begin(), values.begin() + size, [&](const T d) {
			accum += (d - mean) * (d - mean);
		});

		var = accum / (size - 1);
	}

	// weights(1:n) = 1 + (1 / var) * (weights(1:n) - mean) * (image(i, j) - mean)
	void weight_linear(vector<float>& weights, size_t weightSize, float center, float mean, float var)
	{
		float ivar = (1 / var);
		for (size_t i = 0; i < weightSize; ++i)
		{
			weights[i] = 1 + ivar * (weights[i] - mean) * (center - mean);
		}
	}

	// weights(1:n) = exp(-(weights(1:n) - image(i, j)) .^ 2 / (0.6 * var))
	void weight_exp(vector<float>& weights, size_t weightSize, float center, float mean, float var)
	{
		float diff;
		for (size_t i = 0; i < weightSize; ++i)
		{
			diff = weights[i] - center;
			weights[i] = exp(-diff*diff / (0.6f*var));
		}
	}	

	ScaleMaps::ScaleMaps() : mWeightFunc(&weight_linear)
	{
	}

	void ScaleMaps::setWeightFunc(bool exponential)
	{
		if (exponential) mWeightFunc = &weight_exp;
		else mWeightFunc = &weight_linear;
	}

	void ScaleMaps::run(const cv::Mat& img, const vector<FeatureType>& features,
		vector<float>& scaleMap)
	{
		size_t r, c, i, j, nr, nc, min_row, max_row, min_col, max_col;
		size_t neighborCount, coefficientCount = 0;
		std::vector<Eigen::Triplet<float>> coefficients;	// list of non-zeros coefficients
		vector<float> weights(9);
		float m, v, sum;
		size_t pixels = img.total();
		int index = -1, nindex;


		// Initialization
		vector<bool> scale(pixels, false);
		for (i = 0; i < features.size(); ++i)
		{
			const FeatureType& feat = features[i];
			scale[((int)feat.y)*img.cols + (int)feat.x] = true;
		}

		// Adjust image values
		cv::Mat scaledImg = img.clone();
		scaledImg += 1;
		scaledImg *= (1 / 32.0f);

		// For each pixel in the image
		for (r = 0; r < scaledImg.rows; ++r)
		{
			min_row = (size_t)std::max(int(r - 1), 0);
			max_row = (size_t)std::min(scaledImg.rows - 1, int(r + 1));
			for (c = 0; c < scaledImg.cols; ++c)
			{
				// Increment pixel index
				++index;

				// If this is not a feature point
				if (!scale[index])
				{
					min_col = (size_t)std::max(int(c - 1), 0);
					max_col = (size_t)std::min(scaledImg.cols - 1, int(c + 1));
					neighborCount = 0;

					// Loop over 3x3 neighborhoods matrix
					// and calculate the variance of the intensities
					for (nr = min_row; nr <= max_row; ++nr)
					{
						for (nc = min_col; nc <= max_col; ++nc)
						{
							if (nr == r && nc == c) continue;
							weights[neighborCount++] = scaledImg.at<float>(nr, nc);
						}
					}
					weights[neighborCount] = scaledImg.at<float>(r, c);

					// Calculate the weights statistics
					getStat(weights, neighborCount + 1, m, v);
					m *= 0.6;
					if (v < EPS) v = EPS;	// Avoid division by 0

					// Apply weight function
					mWeightFunc(weights, neighborCount, scaledImg.at<float>(r, c), m, v);

					// Normalize the weights and set to coefficients
					sum = std::accumulate(weights.begin(), weights.begin() + neighborCount, 0.0f);
					i = 0;
					for (nr = min_row; nr <= max_row; ++nr)
					{
						for (nc = min_col; nc <= max_col; ++nc)
						{
							if (nr == r && nc == c) continue;
							nindex = nr*scaledImg.cols + nc;
							coefficients.push_back(Eigen::Triplet<float>(
								index, nindex, -weights[i++] / sum));
						}
					}
				}

				// Add center coefficient
				coefficients.push_back(Eigen::Triplet<float>(index, index, 1));
			}
		}

		// Build right side equation vector
		Eigen::VectorXf b = Eigen::VectorXf::Zero(pixels);
		for (i = 0; i < features.size(); ++i)
		{
			const FeatureType& feat = features[i];
			b[((int)feat.y)*scaledImg.cols + (int)feat.x] = feat.scale;
		}

		// Build left side equation matrix
		Eigen::SparseMatrix<float> A(pixels, pixels);
		A.setFromTriplets(coefficients.begin(), coefficients.end());

		/*/// Debug ///
		std::ofstream file("Output.m");
		cv::Mat_<int> rows(1, coefficients.size()), cols(1, coefficients.size());
		cv::Mat_<float> values(1, coefficients.size());
		for (i = 0; i < coefficients.size(); ++i)
		{
		rows.at<int>(i) = coefficients[i].row();
		cols.at<int>(i) = coefficients[i].col();
		values.at<float>(i) = coefficients[i].value();
		}
		file << "cpp_rows = " << rows << ";" << std::endl;
		file << "cpp_cols = " << cols << ";" << std::endl;
		file << "cpp_values = " << values << ";" << std::endl;
		/////////////*/

		// Solving
		Eigen::SparseLU<Eigen::SparseMatrix<float>> slu(A);
		Eigen::VectorXf x = slu.solve(b);

		// Copy to output
		scaleMap.resize(pixels);
		memcpy(scaleMap.data(), x.data(), pixels*sizeof(float));
	}

}	// namespace dc