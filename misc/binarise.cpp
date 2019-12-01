//C++ Includes
#include <iostream>
#include <string>

//C Includes
#include <stdio.h>

//CN_Image
#include "cn_image/image_processor.hpp"
#include "cn_image/basics.hpp"

using namespace std;

int main(int argc, char **argv) {
	//Argument Check
	if (argc != 3) {
		fprintf(stderr, "usage: %s input_ppm output_ppm\n", argv[0]);
		return 1;
	}

	//Import Image
	cn_image::image_processor<unsigned char> ip;
	ip.import_pgm_p5(argv[1]);

	//Binarise and write to file
	cn_image::binarise(ip);
	ip.export_pgm_p5(argv[2]);

	return 0;
}
