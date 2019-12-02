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

var g_w, g_h;

//Store results
var cache = {};

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

	//Reset the cache
	cache = {};

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

	_ctx = document.getElementById("path_canvas").getContext('2d');
	_ctx.canvas.width  = w;
	_ctx.canvas.height = h;

	g_w = w;
	g_h = h;

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
	let real_edge_count = 0;

	for (i = 0; i < graph_lines.length; i++) {
		//CSV FORMAT: FROM, TO, EDGE_ID, WEIGHT
		token = graph_lines[i].split(',');

		if (token.length != 4)
			continue;

		let a   = +token[0];
		let b   = +token[1];
		let eid = +token[2];
		let w   = +token[3];

		if (!(a in graph)) graph[a] = {};
		if (!(b in graph)) graph[b] = {};

		//Undirected graph edge insertion
		graph[a][b] = { "e": eid, "w": w };
		graph[b][a] = { "e": eid, "w": w };

		real_edge_count++;
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

		//Update graph properties on page
		document.getElementById("num_nodes").innerHTML = node_bitmap.length;
		document.getElementById("num_edges").innerHTML = real_edge_count;

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
 * graph_do_dijkstra
 *
 * Performs Dijkstra's algorithm on the graph to go from source S to sink T.
 * (Gee, it's almost as if this code came from somewhere else...)
 */

function graph_do_dijkstra(S, T) {
	//let visited = {};
	//let stack = [];
	//let estack = [];

	//__graph_do_dfs(S, T, stack, estack, visited);

	let nc = node_bitmap.length;
	let visited = Array(nc).fill(0);
	let vdist   = Array(nc).fill(Infinity);
	let vlink   = Array(nc).fill(0);

	vdist[S] = 0;
	vlink[S] = S;

	while (1) {
		let next_i = -1;
		let mindist = Infinity;

		for (let i = 0; i < nc; i++) {
			if (visited[i] == 0 && mindist > vdist[i]) {
				next_i = i;
				mindist = vdist[i];
			}
		}

		let i = next_i;
		if (i == -1)
			break;

		visited[i] = 1;

		for (let k in graph[i]) {
			let wik = graph[i][k].w;

			if (visited[k] == 0) {
				if (vdist[k] > vdist[i] + wik) {
					vdist[k] = vdist[i] + wik;
					vlink[k] = i;
				}
			}
		}
	}

	let stack = [];

	let ii = T, jj = S;
	while (ii != S) {
		stack.push(ii);
		ii = vlink[ii];
	}
	stack.push(S);

	//Cheat
	stack.reverse();

	//Generate edge stack
	let estack = [];
	for (i = 1; i < stack.length; i++)
		estack.push(graph[stack[i - 1]][stack[i]].e);

	return {
		"n_path": stack, /* Nodes in path */
		"e_path": estack /* Edges in path */
	};
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

	//Clear the path image
	path_c = document.getElementById('path_canvas');
	path_ctx = path_c.getContext('2d');
	path_ctx.clearRect(0, 0, g_w, g_h);

	//Draw the master image
	context.globalCompositeOperation = 'source-over';
	context.drawImage(master_bitmap, 0, 0);

	if (source_id == -1 && !selecting_S)
		return;

	//Get the object we are at
	let obj = graph_get_object(m_coord.x, m_coord.y);

	if (obj == -1 || selecting_S) {
		document.getElementById("info_sink").innerHTML = "?";
		document.getElementById("info_dist").innerHTML = "?";
	}
	else
		document.getElementById("info_sink").innerHTML = "Node " + obj + " @ (" + +m_coord.x + ", " + +m_coord.y + ")";

	path_ctx.globalCompositeOperation = 'lighten';

	let S = source_id;
	let T = obj;

	/*
	context.drawImage(node_bitmap[S], 0, 0);

	if (obj != -1) {
		context.drawImage(node_bitmap[obj], 0, 0);
	}*/

	//If a node was selected, let's do DFS
	if (obj != -1) {
		let distance = 0;

		if (selecting_S) {
			path_ctx.drawImage(node_bitmap[obj], 0, 0);
		}
		else {
			let dfs_res;

			//Depending on algorithm selected, perform.
			let ao = document.getElementById('path_algorithm');
			let op = ao.options[ao.selectedIndex].value;

			//Let's check out the cache...
			if (!(op in cache))
				cache[op] = {};

			if (!(S in cache[op]))
				cache[op][S] = {};

			if (T in cache[op][S])
				dfs_res = cache[op][S][T];
			else {
				switch (op) {
					case "dfs"     : dfs_res = graph_do_dfs     (S, T); break;
					case "dijkstra": dfs_res = graph_do_dijkstra(S, T); break;
				}
				cache[op][S][T] = dfs_res;
			}

			//Start keeping track of distance
			distance = 0;

			//Light up nodes
			for (i = 0; i < dfs_res.n_path.length; i++) {
				path_ctx.drawImage(node_bitmap[dfs_res.n_path[i]], 0, 0);

				if (i != 0)
					distance += graph[dfs_res.n_path[i - 1]][dfs_res.n_path[i]].w;
			}

			//Light up edges
			for (i = 0; i < dfs_res.e_path.length; i++)
				path_ctx.drawImage(edge_bitmap[dfs_res.e_path[i]], 0, 0);
		}

		//Colour the path based on "Path Colour"
		path_ctx.globalCompositeOperation = 'source-in';
		path_ctx.fillStyle = path_colour;
		path_ctx.fillRect(0, 0, g_w, g_h);

		//Update the distance statistic
		if (obj != -1 && !selecting_S)
			document.getElementById("info_dist").innerHTML = +distance;
	}

	path_ctx.globalCompositeOperation = 'source-over';
}
