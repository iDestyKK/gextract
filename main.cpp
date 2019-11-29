#include <iostream>
#include <string>

#include <stdio.h>

#include "cn_image/image_processor.hpp"
#include "cn_image/basics.hpp"

using namespace std;

int main(int argc, char **argv) {
	//Argument Check
	if (argc != 3) {
		fprintf(stderr, "usage: %s input_ppm output_dfspak\n", argv[0]);
		return 1;
	}

	cn_image::image_processor<unsigned char> ip;

	ip.import_pgm_p5(argv[1]);

	vector<unsigned int> histogram;
	cn_image::get_histogram(ip, histogram);

	cn_image::binarise(ip);

	ip.export_pgm_p5(argv[2]);

	return 0;
}
