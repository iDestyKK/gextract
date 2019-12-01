/*
 * Basics
 *
 * Description:
 *     Includes basic functions to interact with "image_processor".
 *
 * Author:
 *     Clara Nguyen
 */

#ifndef __CN_IMG_EFFECTS_BASIC_HPP__
#define __CN_IMG_EFFECTS_BASIC_HPP__

#include <iostream>
#include <vector>
#include <limits>
#include "image_processor.hpp"

using namespace std;

namespace cn_image {
	/*
	 * get_histogram
	 *
	 * Returns a vector histogram of an "image_processor". Two Variants, just
	 * in case you want either convenience or efficiency...
	 */

	template <typename T>
	vector<unsigned int> get_histogram(const image_processor<T> &img) {
		vector<unsigned int> histogram;
		get_histogram<T>(img, histogram);

		return histogram;
	}

	template <typename T>
	void get_histogram(
		const image_processor<T> &img,
		vector<unsigned int>     &histogram
	) {
		const vector< vector<T> >& pixels = img.get_pixels();
		unsigned int i, j;

		//Setup with default values and assume it's 8-bit colour (for now)
		histogram.resize(256, 0);

		for (j = 0; j < img.get_height(); j++)
			for (i = 0; i < img.get_width(); i++)
				histogram[pixels[j][i]]++;
	}

	/*
	 * invert
	 *
	 * Inverts an image
	 */

	template <typename T>
	void invert(image_processor<T> &img) {
		unsigned int i, j;
		vector< vector<T> > &pixels = img.ext_get_pixels();

		for (j = 0; j < img.get_height(); j++) {
			for (i = 0; i < img.get_width(); i++) {
				pixels[j][i] = 255 - pixels[j][i];
			}
		}
	}

	/*
	 * subtract
	 *
	 * Subtracts "sub" from "img".
	 */

	template <typename T>
	void subtract(image_processor<T> &img, const image_processor<T> &sub) {
		vector< vector<T> > &a = img.ext_get_pixels();
		const vector< vector<T> > &b = sub.get_pixels();

		int i, j, w, h;
		w = img.get_width();
		h = img.get_height();

		for (j = 0; j < h; j++) {
			for (i = 0; i < w; i++) {
				if (a[j][i] > b[j][i])
					a[j][i] -= b[j][i];
				else
					a[j][i] = 0;
			}
		}
	}

	/*
	 * binarise
	 *
	 * Binarises the image via Otsu's Method
	 */

	template <typename T>
	void binarise(image_processor<T> &img) {
		vector<unsigned int> histogram;

		double N, max_intensity, sum, sumB, q1, q2, mu1, mu2, i, j;
		double threshold, var_max, theta;

		//Get writable pixels of image & histogram
		vector< vector<T> > &pixels = img.ext_get_pixels();
		get_histogram(img, histogram);

		//Get other data
		N = img.get_width() * img.get_height();
		max_intensity = 255;
		threshold = var_max = sum = sumB = q1 = q2 = mu1 = mu2 = 0;

		
		for (i = 0; i <= max_intensity; i++)
			sum += i * histogram[i];

		for (i = 0; i <= max_intensity; i++) {
			q1 += histogram[i];

			if (q1 == 0)
				continue;

			q2 = N - q1;

			if (q2 == 0)
				continue;

			sumB += i * histogram[i];
			mu1 = sumB / q1;
			mu2 = (sum - sumB) / q2;

			theta = q1 * q2 * ((mu1 - mu2) * (mu1 - mu2));

			if (theta > var_max) {
				threshold = i;
				var_max = theta;
			}
		}

		//Binarise
		for (j = 0; j < img.get_height(); j++)
			for (i = 0; i < img.get_width(); i++)
				pixels[j][i] = (pixels[j][i] > threshold) ? 255 : 0;
	}
}

#endif
