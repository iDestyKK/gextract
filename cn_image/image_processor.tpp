#ifndef __IMAGE_PROCESSOR_TPP__
#define __IMAGE_PROCESSOR_TPP__

#include "image_processor.hpp"

namespace cn_image {

// ----------------------------------------------------------------------------
// Constructors                                                            {{{1
// ----------------------------------------------------------------------------

/*
 * Standard Constructor
 */

template <typename T>
image_processor<T>::image_processor() {
	width  = 0;
	height = 0;
}

/*
 * Copy from another image_processor of same type
 */

template <typename T>
image_processor<T>::image_processor(const image_processor &from) {
	copy(from);
}

// ----------------------------------------------------------------------------
// Readers/Importers                                                       {{{1
// ----------------------------------------------------------------------------

template <typename T>
void image_processor<T>::import_pgm_p5(istream &is) {
	string header;
	int tw, th, td, i, j;
	unsigned char* buffer;

	getline(is, header);

	//Header Check
	if (header != "P5")
		return;

	//Image Dimensions & Depth
	is >> tw >> th >> td;

	//Check Dimensions for invalid values
	if (tw < 0 || th < 0)
		return;

	//Force depth to only be 255
	if (td != 255)
		return;

	//Skip newline
	is.get();

	/*
	 * Read in every pixel. Since this is templated, we have to cast every
	 * value after reading in. Use a buffer to read data raw, then go through
	 * and cast everything.
	 */

	buffer = (unsigned char *) malloc(sizeof(unsigned char) * tw * th);
	is.read((char *) buffer, tw * th);

	pixels.resize(th, vector<T>(tw));

	//Cast from buffer to pixels vector.
	for (j = 0; j < th; j++)
		for (i = 0; i < tw; i++)
			pixels[j][i] = static_cast<T>(buffer[(j * tw) + i]);

	//Set dimensions
	width  = tw;
	height = th;

	//Free the buffer
	free(buffer);
}

template <typename T>
void image_processor<T>::import_pgm_p5(const char *fname) {
	ifstream fp;
	fp.open(fname);

	//Failure to open file results in nothing happening
	if (fp.fail())
		return;

	import_pgm_p5(fp);

	fp.close();
}

// ----------------------------------------------------------------------------
// Writers/Exporters                                                       {{{1
// ----------------------------------------------------------------------------

template <typename T>
void image_processor<T>::export_pgm_p5(ostream &os) {
	unsigned int i;

	//Header Information
	os << "P5\n" << width << " " << height << "\n" << 255 << "\n";

	//Write each row
	for (i = 0; i < height; i++)
		os.write((const char *) pixels[i].data(), width);
}

template <typename T>
void image_processor<T>::export_pgm_p5(const char *fname) {
	ofstream op;
	op.open(fname);

	if (op.fail())
		return;

	export_pgm_p5(op);
	op.close();
}

// ----------------------------------------------------------------------------
// Get Functions                                                           {{{1
// ----------------------------------------------------------------------------

template <typename T>
const T& image_processor<T>::get_pixel(int x, int y) const {
	return pixels[y][x];
}

template <typename T>
const unsigned int& image_processor<T>::get_width() const {
	return width;
}

template <typename T>
const unsigned int& image_processor<T>::get_height() const {
	return height;
}

template <typename T>
const vector< vector<T> >& image_processor<T>::get_pixels() const {
	return pixels;
}

template <typename T>
vector< vector<T> >& image_processor<T>::ext_get_pixels() {
	return pixels;
}

// ----------------------------------------------------------------------------
// Set Functions                                                           {{{1
// ----------------------------------------------------------------------------

template <typename T>
void image_processor<T>::set_pixel(int x, int y, const T& v) {
	pixels[y][x] = v;
}

// ----------------------------------------------------------------------------
// Image Modification Functions                                            {{{1
// ----------------------------------------------------------------------------

template <typename T>
void image_processor<T>::copy(const image_processor &from) {
	unsigned int i, j;

	//Copy Data
	width  = from.get_width ();
	height = from.get_height();

	//Setup Pixels
	pixels.resize(height, vector<T>(width));

	//Efficiently copy "by-row"
	const vector< vector<T> > &ref = from.get_pixels();

	for (i = 0; i < height; i++)
		memcpy(&pixels[i][0], &ref[i][0], sizeof(T) * width);
}

template <typename T>
void image_processor<T>::resize(int nw, int nh) {
	pixels.resize(nh, vector<T>(nw, T()));
	width = nw;
	height = nh;
}

template <typename T>
void image_processor<T>::clear() {
	pixels.clear();
}

}

#endif
