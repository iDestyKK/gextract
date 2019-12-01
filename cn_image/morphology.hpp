/*
 * Morphology
 *
 * Description:
 *     Implements the Morphology plugin given in "morphology_.java". This is
 *     simply a rewrite of it in C++.
 *
 * Author:
 *     Clara Nguyen
 */

#ifndef __CN_IMG_MORPHOLOGY_HPP__
#define __CN_IMG_MORPHOLOGY_HPP__

#include <iostream>
#include <vector>
#include <cmath>

#include "image_processor.hpp"
#include "basics.hpp"

using namespace std;

namespace cn_image {

	/*
	 * dilate
	 *
	 * Performs Dilation on a given image. This assumes the image has been run
	 * through "binarise" already.
	 */

	template <typename T>
	void dilate(image_processor<T> &img, int rad) {
		image_processor<T> tmp;
		tmp.resize(img.get_width(), img.get_height());

		vector< vector<T> > &pixels = img.ext_get_pixels();
		vector< vector<T> > &tmp_pixels = tmp.ext_get_pixels();

		int n, m, i, j, r, r2, tn, tm;
		r2 = rad * rad;

		//For every pixel in the image
		for (j = 0; j < img.get_height(); j++) {
			for (i = 0; i < img.get_width(); i++) {
				/*
				 * Loop through surrounding pixels to see if any match. If
				 * so, set. Otherwise, don't.
				 */

				for (n = max(j - rad, 0); n <= min(j + rad, (int) img.get_height() - 1); n++) {
					for (m = max(i - rad, 0); m <= min(i + rad, (int) img.get_width() - 1); m++) {
						tn = n - j;
						tm = m - i;

						//Check if inside circle (H_DISK)
						if (((tn * tn) + (tm * tm)) > r2)
							continue;

						if (pixels[n][m] == 255)
							tmp_pixels[j][i] = 255;
					}
				}
			}
		}

		pixels = tmp_pixels;
	}

	/*
	 * erode
	 *
	 * Performs Erosion on a given image.
	 */

	template <typename T>
	void erode(image_processor<T> &img, int rad) {
		invert(img);
		dilate(img, rad);
		invert(img);
	}

	/*
	 * open
	 *
	 * Performs an erosion followed by a dilation
	 */

	template <typename T>
	void open(image_processor<T> &img, int rad) {
		erode(img, rad);
		dilate(img, rad);
	}

	/*
	 * close
	 *
	 * Performs a dilation followed by an erosion
	 */

	template <typename T>
	void close(image_processor<T> &img, int rad) {
		dilate(img, rad);
		erode(img, rad);
	}
}

#endif
