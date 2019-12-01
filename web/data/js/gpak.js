/*
 * ECE 572 - Final Project: Graph Simulate
 *
 * Description:
 *     Interface for a web simulator that can parse gpak files. This file will
 *     provide all needed functionality for a simulator to take a gpak file,
 *     parse it, generate all bitmaps, do path detection, and screen drawing to
 *     a canvas.
 *
 * Author:
 *     Clara Nguyen
 */

// ----------------------------------------------------------------------------
// GLOBALS                                                                 {{{1
// ----------------------------------------------------------------------------

var files;
var reader;
var gpak_bytes;

var master_image;
var node_image = [];
var edge_image = [];

var master_bitmap;
var node_bitmap = [];
var edge_bitmap = [];

var collision_map;
var graph;
var ready = false;

const offset = {
	BEGIN     : 0,
	MASTER    : 1,
	NODES     : 2,
	EDGES     : 3,
	COLLISION : 4,
	GRAPH     : 5
};

// ----------------------------------------------------------------------------
// HELPER FUNCTIONS                                                        {{{1
// ----------------------------------------------------------------------------

function byte2int(a, start) {
	return (a[start + 0]      ) |
		   (a[start + 1] <<  8) |
		   (a[start + 2] << 16) |
		   (a[start + 3] << 24);
}

// ----------------------------------------------------------------------------
// GPAK FUNCTIONS                                                          {{{1
// ----------------------------------------------------------------------------

function gpak_read(bytes) {
	let bl = bytes.length;
	let w, h, n, e, i, j;
	let collision_arr;
	let graph_csv;
	let graph_lines;
	let token;

	ready = false;

	/*
	 * Get the offsets from the ends of the file (This will make
	 * our lives easier in the long run).
	 */

	offsets = [
		0,                        /* BEGIN     */
		byte2int(bytes, bl - 20), /* MASTER    */
		byte2int(bytes, bl - 16), /* NODES     */
		byte2int(bytes, bl - 12), /* EDGES     */
		byte2int(bytes, bl -  8), /* COLLISION */
		byte2int(bytes, bl -  4)  /* GRAPH     */
	];

	//Image dimensions
	w = byte2int(bytes, 12);
	h = byte2int(bytes, 16);

	//Set the canvas
	let _ctx = document.getElementById("image_canvas").getContext('2d');
	_ctx.canvas.width  = w;
	_ctx.canvas.height = h;

	//Number of nodes and edges
	n = byte2int(bytes, 4);
	e = byte2int(bytes, 8);

	//Load in the collision data
	collision_arr = new TextDecoder("utf-8").decode(
		gpak_bytes.slice(offsets[4], offsets[5])
	).split(' ');

	//Generate the collision map
	collision_map = [];
	for (j = 0; j < h; j++) {
		collision_map.push([]);
		for (i = 0; i < w; i++)
			collision_map[j].push(collision_arr[i + (j * w)]);
	}

	//Load in the graph CSV
	graph_csv = new TextDecoder("utf-8").decode(
		gpak_bytes.slice(offsets[5], gpak_bytes.length - 20)
	);

	//Parse graph CSV into an actual graph...
	graph = {};
	graph_lines = graph_csv.split('\n');

	for (i = 0; i < graph_lines.length; i++) {
		//CSV FORMAT: FROM, TO, EDGE_ID, WEIGHT
		token = graph_lines[i].split(',');
		let a   = +token[0];
		let b   = +token[1];
		let eid = +token[2];
		let w   = +token[3];

		if (!(a in graph)) graph[a] = {};
		if (!(b in graph)) graph[b] = {};

		//Undirected graph edge insertion
		graph[a][b] = { "e": eid, "w": w };
		graph[b][a] = { "e": eid, "w": w };
	}

	//Read the master image pixels in.
	var c = document.getElementById("image_canvas");
	var ctx = c.getContext("2d");

	//Load the master image
	console.log("Loading Master...");
	let img_sz = w * h * 4;
	master_image = new ImageData(w, h);
	for (i = 0; i < img_sz; i++)
		master_image.data[i] = bytes[offsets[offset.MASTER] + i];

	//Load the node images
	node_image = [];
	for (i = offsets[offset.NODES]; i < offsets[offset.EDGES]; i += img_sz) {
		console.log("Loading Node...");
		node_image.push(new ImageData(w, h));
		for (j = 0; j < img_sz; j++)
			node_image[node_image.length - 1].data[j] = bytes[i + j];
	}

	//Load the edge images
	edge_image = [];
	for (i = offsets[offset.EDGES]; i < offsets[offset.COLLISION]; i += img_sz) {
		console.log("Loading Edge...");
		edge_image.push(new ImageData(w, h));
		for (j = 0; j < img_sz; j++)
			edge_image[edge_image.length - 1].data[j] = bytes[i + j];
	}

	//Generate Bitmaps
	bm_p = [];
	bm_p.push(createImageBitmap(master_image));

	for (i = 0; i < node_image.length; i++)
		bm_p.push(createImageBitmap(node_image[i]));
	for (i = 0; i < edge_image.length; i++)
		bm_p.push(createImageBitmap(edge_image[i]));

	Promise.all( bm_p ).then(function(r) {
		//Wipe out any existing bitmaps
		node_bitmap = [];
		edge_bitmap = [];

		//Fill in all bitmaps
		master_bitmap = r[0];

		for (i = 0; i < node_image.length; i++)
			node_bitmap.push(r[i + 1]);

		for (i = 0; i < edge_image.length; i++)
			edge_bitmap.push(r[node_image.length + i + 1]);

		ready = true;
	});

	ctx.putImageData(master_image, 0, 0);
}

// ----------------------------------------------------------------------------
// GRAPH FUNCTIONS                                                         {{{1
// ----------------------------------------------------------------------------

/*
 * graph_get_object
 *
 * Returns the ID of an object at (x, y), if possible. Returns -1 if there is
 * no object at that point.
 */

function graph_get_object(x, y) {
	//Just cheat and use the collision map... too easy.
	return collision_map[y][x] - 1;
}

/*
 * graph_do_dfs
 *
 * Performs DFS on the graph to go from source S to sink T.
 */

function graph_do_dfs(S, T) {
	let visited = {};
	let stack = [];
	let estack = [];

	__graph_do_dfs(S, T, stack, estack, visited);

	return {
		"n_path": stack, /* Nodes in path */
		"e_path": estack /* Edges in path */
	};
}

function __graph_do_dfs(S, T, stack, estack, visited) {
	stack.push(S);
	visited[S] = true;

	if (S == T)
		return true;

	for (let to in graph[S]) {
		if (!(to in visited)) {
			estack.push(graph[S][to].e);

			if (__graph_do_dfs(+to, T, stack, estack, visited))
				return true;

			estack.pop();
		}
	}

	//Assume failure
	delete visited[S];
	stack.pop();
}

/*
 * graph_draw
 *
 * Draw handler for the graph.
 */

function graph_draw(canvas, context, m_coord) {
	//Do nothing. We can't draw anything anyways.
	if (ready === false)
		return;

	//Draw the master image
	context.globalCompositeOperation = 'source-over';
	context.drawImage(master_bitmap, 0, 0);

	//Get the object we are at
	let obj = graph_get_object(m_coord.x, m_coord.y);

	context.globalCompositeOperation = 'screen';

	let S = 0;
	let T = obj;

	/*
	context.drawImage(node_bitmap[S], 0, 0);

	if (obj != -1) {
		context.drawImage(node_bitmap[obj], 0, 0);
	}*/

	//If a node was selected, let's do DFS
	if (obj != -1) {
		let dfs_res = graph_do_dfs(S, T);

		for (i = 0; i < dfs_res.n_path.length; i++)
			context.drawImage(node_bitmap[dfs_res.n_path[i]], 0, 0);

		for (i = 0; i < dfs_res.e_path.length; i++)
			context.drawImage(edge_bitmap[dfs_res.e_path[i]], 0, 0);
	}

	context.globalCompositeOperation = 'source-over';
}
