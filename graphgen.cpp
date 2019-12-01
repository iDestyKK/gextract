/*
 * ECE 572 - Final Project: Graph Generate (graphgen)
 *
 * Description:
 *     Takes a valid PGM (Binary/P5) file and attempts to generate a graph with
 *     it. The program should be able to figure out nodes, edges, and how they
 *     all connect. The data is then dumped to a directory (an empty one...) of
 *     the user's choice to be packaged by "graphpak", which is also included
 *     in this project.
 *
 * Author:
 *     Clara Nguyen
 */

//C++ Includes
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <set>

//C Includes
#include <stdio.h>
#include <string.h>

//Our "secret weapon"
#include "cn_image/image_processor.hpp"
#include "cn_image/basics.hpp"
#include "cn_image/morphology.hpp"
#include "cn_image/connected_components.hpp"

//Hardcoded dilation radius for checking what nodes belong to what edges
#define EDGE_BOOST 10

using namespace std;

int main(int argc, char **argv) {
	//Argument Check
	if (argc < 6) {
		fprintf(
			stderr,
			"usage: %s [-i] radius node_min_vol edge_min_vol "
			"input_ppm output_dir\n",
			argv[0]
		);
		return 1;
	}

	int i, j, u, v;
	int option_invert = false;
	int option_radius = 0;
	int option_node_min_vol = 0;
	int option_edge_min_vol = 0;

	//Parse arguments
	if (argc > 6) {
		for (i = 1; i < argc - 4; i++) {
			if (strcmp(argv[i], "-i") == 0)
				option_invert = true;
		}
	}

	option_radius       = atoi(argv[argc - 5]);
	option_node_min_vol = atoi(argv[argc - 4]);
	option_edge_min_vol = atoi(argv[argc - 3]);

	//Setup the image and read from file
	cn_image::image_processor<unsigned char> ip, mask;
	ip.import_pgm_p5(argv[argc - 2]);

	//Dump a copy of the image to the output directory
	ostringstream oss_img;
	oss_img << argv[argc - 1] << "/master.pgm";
	ip.export_pgm_p5(oss_img.str().c_str());

	//Binarise the image and act accordingly
	cn_image::binarise(ip);

	if (option_invert)
		cn_image::invert(ip);

	//Store the mask for later. We'll need this.
	mask = ip;

	//Perform opening to extract the nodes
	cn_image::open(ip, option_radius);

	//Let's extract nodes and edges
	vector< cn_image::image_processor<unsigned char> > nodes, edges, ee;
	cn_image::subtract(mask, ip);

	//Split edges into images
	cn_image::split_by_connected_components(mask, edges, option_edge_min_vol);
	ee = edges;

	//Split nodes into images
	cn_image::split_by_connected_components(ip, nodes, option_node_min_vol);

	//Dump edges to files
	char fname[16];
	for (i = 0; i < edges.size(); i++) {
		ostringstream oss;
		oss << argv[argc - 1] << "/edge"  << i << ".pgm";

		edges[i].export_pgm_p5(oss.str().c_str());

		//Dilate for later
		cn_image::dilate(ee[i], EDGE_BOOST);
	}

	//Dump nodes to files
	for (i = 0; i < nodes.size(); i++) {
		ostringstream oss;
		oss << argv[argc - 1] << "/node" << i << ".pgm";

		nodes[i].export_pgm_p5(oss.str().c_str());
	}

	//Take advantage of how cn_image::connected_components labels is still
	//filled... This is pretty bad but hey I'm not to complain.

	//Scan through every edge and try to find 2 nodes connected
	ostringstream oss_graph;
	oss_graph << argv[argc - 1] << "/graph.csv";

	ofstream op;
	op.open(oss_graph.str().c_str());

	for (i = 0; i < edges.size(); i++) {
		set<int> connected_edges;
		int volume = 0;
		cn_image::image_processor<unsigned char> &ee_img = ee[i];
		cn_image::image_processor<unsigned char> &edge_img = edges[i];

		for (v = 0; v < ip.get_height(); v++) {
			for (u = 0; u < ip.get_width(); u++) {
				//Only process white pixels
				if (ee_img.get_pixel(u, v) != 255)
					continue;

				if (edge_img.get_pixel(u, v) == 255)
					volume++;

				int Luv = cn_image::findLabel(cn_image::label[u][v]);
				int Lid = cn_image::setLabel[Luv];

				//Insert if not the background
				if (Lid != 0)
					connected_edges.insert(Lid - 1);
			}
		}

		if (connected_edges.size() == 2) {
			set<int>::iterator ii;
			for (
				ii  = connected_edges.begin();
				ii != connected_edges.end();
				ii++
			) {
				op << *ii << ",";
			}
			op << i << "," << volume << endl;
		}
	}

	op.close();

	//Export "collision" data to a file as well
	ostringstream oss_collision;
	oss_collision << argv[argc - 1] << "/collision.dat";

	op.open(oss_collision.str().c_str());

	for (v = 0; v < ip.get_height(); v++) {
		for (u = 0; u < ip.get_width(); u++) {
			int Luv = cn_image::findLabel(cn_image::label[u][v]);
			int Lid = cn_image::setLabel[Luv];

			op << Lid;

			if (u == ip.get_width() - 1)
				op << endl;
			else
				op << " ";
		}
	}

	op.close();

	//ip.export_pgm_p5(argv[argc - 1]);

	return 0;
}
