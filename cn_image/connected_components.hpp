/*
 * Connected Components
 *
 * Description:
 *     Implements "Connected Components" via Disjoint-sets. This is essentially
 *     a C++ remake of HW11.
 *
 * Author:
 *     Clara Nguyen
 */

#ifndef __CN_IMG_DISJOINT_SETS_HPP__
#define __CN_IMG_DISJOINT_SETS_HPP__

#include <iostream>
#include <vector>
#include <cmath>
#include <map>

#include "image_processor.hpp"
#include "basics.hpp"

namespace cn_image {
	/*
	 * Namespace globals for disjoint-sets
	 */

	int nextLabel, Nobjects;

	vector<int> disjointSetLabel;
	vector<int> disjointSetRank;

	vector< vector<int> > label;
	map<int, int> setLabel;

	/*
	 * findLabel
	 */

	int findLabel(int i) {
		if (disjointSetLabel[i] == -1)
			return i;

		//Recursive path compression
		disjointSetLabel[i] = findLabel(disjointSetLabel[i]);
		return disjointSetLabel[i];
	}

	/*
	 * mergeLabels
	 */

	int mergeLabels(int i1, int i2) {
		i1 = findLabel(i1);
		i2 = findLabel(i2);

		// union-by-rank merging of sets
		if (i1 != i2) {
			if (disjointSetRank[i1] > disjointSetRank[i2])
				disjointSetLabel[i2] = i1;
			else
			if (disjointSetRank[i1] < disjointSetRank[i1])
				disjointSetLabel[i1] = i2;
			else {
				disjointSetLabel[i2] = i1;
				disjointSetRank[i1] += 1;
			}
		}

		return findLabel(i1);
	}

	/*
	 * split_by_connected_components
	 *
	 * Performs connected_components on "img", extracts all objects that result
	 * from the disjoint-set and places them into a vector of images "imgs".
	 *
	 * In other words, each object detected by disjoint-sets gets its own image
	 * to live in. Neat.
	 *
	 * Also, a threshold can be specified so a minimum number of pixels must be
	 * set for the set to be valid. This is to prevent sets of single pixels,
	 * or the like.
	 */

	template <typename T>
	void split_by_connected_components(
		image_processor<T> &img,
		vector< image_processor<T> > &imgs,
		int threshold = 0
	) {
		//Disjoint-set computation (This comes straight out of the HW...)
		int Nrow, Ncol, i, j, v, u;
		vector< vector<T> > &pixels = img.ext_get_pixels();

		nextLabel = 1; // initial object label
		Nobjects  = 0; // num objects created

		Nrow = img.get_height();
		Ncol = img.get_width();

		label.clear();
		label.resize(Ncol, vector<int>(Nrow, 0));

		disjointSetLabel.clear();
		disjointSetLabel.resize(Nrow * Ncol, -1);

		disjointSetRank.clear();
		disjointSetRank.resize(Nrow * Ncol, 0);

		for (v = 0; v < Nrow; v++) {
			for (u = 0; u < Ncol; u++) {
				int Iuv = static_cast<int>(pixels[v][u]);

				label[u][v] = 0; // default label: background
				if (Iuv == 0)
					continue;

				int _S = ((u == 0) ? 0 : findLabel(label[u - 1][v]));
				int _T = ((v == 0) ? 0 : findLabel(label[u][v - 1]));

				if (_S == 0 && _T == 0) {
					label[u][v] = nextLabel++;
					Nobjects++;
				}
				else
				if (_S != 0 && _T == 0) {
					label[u][v] = _S;
				}
				else
				if (_S == 0 && _T != 0) {
					label[u][v] = _T;
				}
				else {
					if (findLabel(_S) == findLabel(_T))
						label[u][v] = _S;
					else {
						label[u][v] = mergeLabels(_S, _T);
						Nobjects -= 1;
					}
				}
			}
		}


		setLabel.clear();
		setLabel[0] = 0;

		//Construct images
		vector< image_processor<T> > tmp_imgs;
		tmp_imgs.resize(Nobjects);

		//By default, make all pixels zero
		for (i = 0; i < Nobjects; i++)
			tmp_imgs[i].resize(Ncol, Nrow);

		//Set volume to 0
		vector<int> volume;
		volume.resize(Nobjects, 0);

		for (v = 0; v < Nrow; v++) {
			for (u = 0; u < Ncol; u++) {
				int Luv = findLabel(label[u][v]);

				if (setLabel.find(Luv) == setLabel.end())
					setLabel.insert(make_pair(Luv, setLabel.size()));

				int Lid = setLabel[Luv];

				if (Lid != 0) {
					tmp_imgs[Lid - 1].set_pixel(u, v, 255);
					volume[Lid - 1]++;
				}
			}
		}

		//Move valid layers into "imgs" and return.
		imgs.clear();

		for (i = 0; i < Nobjects; i++) {
			if (volume[i] > threshold)
				imgs.push_back(tmp_imgs[i]);
		}
	}
}

#endif
