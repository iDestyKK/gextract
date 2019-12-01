/*
 * ECE 572 - Final Project: Graph Package (graphpak)
 *
 * Description:
 *     Takes a valid director and attempts to generate a "gpak" file with it.
 *     This file will be valid for uploading to a simulator online on my page.
 *
 * Author:
 *     Clara Nguyen
 */

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>

using namespace std;

// ----------------------------------------------------------------------------
// Image File Struct                                                       {{{1
// ----------------------------------------------------------------------------

struct img_file {
	img_file();
	img_file(const char *);
	void open(const char *);
	int w, h, d;
	vector<unsigned char> data;

	void dump(ofstream &, bool = false);
};

img_file::img_file() {

}

img_file::img_file(const char *fpath) {
	open(fpath);
}

void img_file::open(const char *fpath) {
	ifstream fp(fpath);
	string header;

	fp >> header >> w >> h >> d;
	fp.get();

	//Read in all pixels
	data.resize(w * h);
	fp.read((char *) &data[0], w * h);

	fp.close();
}

void img_file::dump(ofstream &op, bool black_is_transparent) {
	//Dump all bytes
	//op.write((char *) &data[0], w * h);

	/*
	 * To optimise for the web application, let's write 3 copies of every value
	 * along with an alpha of 1...
	 */

	int i, j;
	unsigned char a;
	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			if (black_is_transparent && data[i + (j * w)] == 0)
				a = 0x00;
			else
				a = 0xFF;

			op.write((char *) &data[i + (j * w)], 1);
			op.write((char *) &data[i + (j * w)], 1);
			op.write((char *) &data[i + (j * w)], 1);
			op.write((char *) &a                , 1);
		}
	}
}

// ----------------------------------------------------------------------------
// Helper Functions                                                        {{{1
// ----------------------------------------------------------------------------

string cpa(string dir, string fname) {
	return dir + "/" + fname;
}

bool file_exists(const char *fname) {
	struct stat buffer;
	return (stat(fname, &buffer) == 0);
}

// ----------------------------------------------------------------------------
// Main Function                                                           {{{1
// ----------------------------------------------------------------------------

int main(int argc, char **argv) {
	//Argument check
	if (argc != 3) {
		cerr << "usage: " << argv[0] << " data_dir out_gpak\n";
		return 1;
	}

	int i;
	string line;

	ifstream fp;
	ofstream gpak;

	//Specify files
	img_file master;
	vector<img_file> nodes;
	vector<img_file> edges;

	gpak.open(argv[2]);

	//Read "master.pgm"
	master.open(cpa(argv[1], "master.pgm").c_str());

	//Read as many nodes as possible
	for (i = 0;; i++) {
		ostringstream oss;
		oss << argv[1] << "/" << "node" << i << ".pgm";

		if (!file_exists(oss.str().c_str()))
			break;

		nodes.push_back(img_file());
		nodes.back().open(oss.str().c_str());
	}

	//Read as many edges as possible
	for (i = 0;; i++) {
		ostringstream oss;
		oss << argv[1] << "/" << "edge" << i << ".pgm";

		if (!file_exists(oss.str().c_str()))
			break;

		edges.push_back(img_file());
		edges.back().open(oss.str().c_str());
	}

	//Dump information
	unsigned int ns = nodes.size();
	unsigned int es = edges.size();

	//Offsets
	unsigned int offset_master,
	             offset_nodes,
	             offset_edges,
	             offset_collision,
	             offset_graph;

	//Header Information
	//GPAK + 4 bytes (node count) + 4 bytes (edge count)
	gpak << "GPAK";
	gpak.write((const char *) &ns      , sizeof(unsigned int));
	gpak.write((const char *) &es      , sizeof(unsigned int));
	gpak.write((const char *) &master.w, sizeof(unsigned int));
	gpak.write((const char *) &master.h, sizeof(unsigned int));
	gpak.write((const char *) &master.d, sizeof(unsigned int));

	//Dump master image
	offset_master = gpak.tellp();

	master.dump(gpak);

	//Dump all nodes (RAW)
	offset_nodes = gpak.tellp();

	for (i = 0; i < nodes.size(); i++)
		nodes[i].dump(gpak, true);

	//Dump all edges (RAW)
	offset_edges = gpak.tellp();

	for (i = 0; i < edges.size(); i++)
		edges[i].dump(gpak, true);

	//Dump collision data
	offset_collision = gpak.tellp();

	fp.open(cpa(argv[1], "collision.dat").c_str());
	bool first = false;

	while (fp >> i) {
		if (!first) {
			first = true;
			gpak << i;
		}
		else
			gpak << " " << i;
	}
	fp.close();

	//Dump graph CSV
	offset_graph = gpak.tellp();

	fp.open(cpa(argv[1], "graph.csv").c_str());

	while (getline(fp, line))
		gpak << line << endl;

	fp.close();

	//Write the offsets
	gpak.write((const char *) &offset_master   , sizeof(unsigned int));
	gpak.write((const char *) &offset_nodes    , sizeof(unsigned int));
	gpak.write((const char *) &offset_edges    , sizeof(unsigned int));
	gpak.write((const char *) &offset_collision, sizeof(unsigned int));
	gpak.write((const char *) &offset_graph    , sizeof(unsigned int));

	return 0;
}
