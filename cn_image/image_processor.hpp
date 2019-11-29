/*
 * CN_Image - Image Processor
 *
 * Description:
 *     Kinda inspired by ImageJ's. This will be so we can interface with an 
 *     image.
 *
 * Author:
 *     Clara Nguyen
 */
#ifndef __IMAGE_PROCESSOR_HPP__
#define __IMAGE_PROCESSOR_HPP__

//C Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//C++ Includes
#include <string>
#include <fstream>
#include <vector>

using namespace std;

namespace cn_image {

// ----------------------------------------------------------------------------
// Setup                                                                   {{{1
// ----------------------------------------------------------------------------

//Valid Image Pixel Formats
typedef unsigned char  PIX_GREYSCALE_8BIT;
typedef unsigned short PIX_GREYSCALE_16BIT;
typedef float          PIX_GREYSCALE_32BIT_FLOAT;
typedef double         PIX_GREYSCALE_64BIT_FLOAT;
typedef long double    PIX_GREYSCALE_80BIT_FLOAT;

// ----------------------------------------------------------------------------
// Image Processor Class                                                   {{{1
// ----------------------------------------------------------------------------

template <typename T>
class image_processor {
	public:
		//Constructor
		image_processor();
		image_processor(const image_processor &);

		//Readers/Importers
		void import_pgm_p5(istream &);
		void import_pgm_p5(const char *);

		//Writers/Exporters
		void export_pgm_p5(ostream &);
		void export_pgm_p5(const char *);

		//Get Functions
		const T& get_pixel(int, int) const;
		const unsigned int& get_width() const;
		const unsigned int& get_height() const;
		const vector< vector<T> >& get_pixels() const;
		vector< vector<T> >& ext_get_pixels(); /* Allow Extensions to Modify */

		//Set Functions
		void set_pixel(int, int, const T&);

		//Image Modification Functions
		void copy(const image_processor &);
		void resize(int, int);
		void clear();

	private:
		unsigned int width,
		             height;

		vector< vector<T> > pixels;
};

}

//Template Definitions
#include "image_processor.tpp"

#endif
