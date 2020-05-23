"use strict";

const { spawn } = require('child_process');

function solve(
	problem,			// VRP问题描述
	npop=2,
	popsize=100,		// 种群大小
	maxiter=40,			// 迭代次数
) {
	var solver = spawn("D:\\Application\\cpp\\VehicleRouting\\out\\build\\x64-Release\\VehicleRouting.exe");
	var pipe = (...msg) => {
		solver.stdin.write(msg.join(' ') + '\n');
		// console.log(msg.join(' '));
	};

	var n_customer = 0;
	for (let i in problem.nodes) {
		var node = problem.nodes[i];
		if (node.type == 'customer') {
			n_customer++;
		}
	}
	pipe(n_customer);
	for (let i in problem.nodes) {
		var node = problem.nodes[i];
		if (node.type == 'customer') {
			var demand = node.demand;
			var service_time = node.service_time || 0;
			var tw_beg = node.tw_beg || -1;
			var tw_end = node.tw_end || -1;
			pipe(node.id, demand, service_time, tw_beg, tw_end);
		}
	}

	var n_depot = 0, first_depot = -1;
	for (let i in problem.nodes) {
		var node = problem.nodes[i];
		if (node.type == 'depot') {
			if (first_depot == -1) {
				first_depot = node.id;
			}
			n_depot++;
		}
	}
	if (first_depot == -1) {
		throw 'no depot was found in nodes';
	}
	pipe(n_depot);
	for (let i in problem.nodes) {
		var node = problem.nodes[i];
		if (node.type == 'depot') {
			pipe(node.id);
		}
	}

	var n_other = 0;
	for (let i in problem.nodes) {
		var node = problem.nodes[i];
		if (node.type == 'other') {
			n_other++;
		}
	}
	pipe(n_other);
	for (let i in problem.nodes) {
		var node = problem.nodes[i];
		if (node.type == 'other') {
			pipe(node.id);
		}
	}

	if (typeof(problem.edges) == 'string') {
		if (problem.edges == 'euc2d') {
			var n = problem.nodes.length;
			pipe(Math.round(n * (n - 1) / 2));
			for (var i = 0; i < n; i++) {
				for (var j = 0; j < i; j++) {
					var w = Math.sqrt(
						Math.pow(problem.nodes[i].x - problem.nodes[j].x, 2)
						+ Math.pow(problem.nodes[i].y - problem.nodes[j].y, 2));
					pipe(problem.nodes[i].id, problem.nodes[j].id, w);
				}
			}
		} else {
			throw 'unsupported edge type \"' + problem.edges + '\"';
		}
	} else {
		pipe(problem.edges.length);
		for (var i in problem.edges) {
			var edge = problem.edges[i];
			pipe(edge.u, edge.v, edge.w);
		}
	}

	pipe(problem.vehicles.length);
	var speed = problem.speed || 1;
	var work_time = problem.work_time || -1;
	pipe(speed, work_time);
	for (var i in problem.vehicles) {
		var veh = problem.vehicles[i];
		var depot = veh.depot || first_depot;
		var load = veh.load;
		var mileage = veh.mileage || -1;
		var count = veh.count || -1;
		pipe(veh.id, depot, load, mileage, count);
	}

	pipe(problem.distancePrior, problem.timePrior, problem.loadPrior);
	pipe(npop, popsize, maxiter);

	var out = '';

	solver.stdout.on('data', (buffer) => {
		out += buffer.toString();
	});

	solver.on('exit', () => {
		console.log(out);
		var solution = eval('(' + out + ')');
	});
}

function load_test() {
	var problem = {
		"nodes": [
			{"id":0, "type":"depot"},
			{"id":1, "type":"customer", "demand":1.7, "service_time":5./60},
			{"id":2, "type":"customer", "demand":0.8, "service_time":5./60},
			{"id":3, "type":"customer", "demand":1.3, "service_time":5./60},
			{"id":4, "type":"customer", "demand":2.8, "service_time":5./60},
			{"id":5, "type":"customer", "demand":1.9, "service_time":5./60},
			{"id":6, "type":"customer", "demand":3.5, "service_time":5./60},
			{"id":7, "type":"customer", "demand":0.9, "service_time":5./60},
			{"id":8, "type":"customer", "demand":0.3, "service_time":5./60},
			{"id":9, "type":"customer", "demand":1.2, "service_time":5./60},
			{"id":10, "type":"other"}
		],
		"edges": [
			{"u": 0, "v": 1, "w": 5},	
			{"u": 0, "v": 2, "w": 8},	
			{"u": 0, "v": 3, "w": 7},	
			{"u": 0, "v": 10, "w":5},	
			{"u": 0, "v": 5, "w": 4},
			{"u": 0, "v": 6, "w": 12},
			{"u": 0, "v": 7, "w": 9},
			{"u": 0, "v": 8, "w": 12},
			{"u": 0, "v": 9, "w": 6},
			{"u": 1, "v": 2, "w": 4},
			{"u": 1, "v": 9, "w": 3},
			{"u": 2, "v": 3, "w": 3},
			{"u": 3, "v": 4, "w": 4},
			{"u": 3, "v": 10, "w": 5},
			{"u": 4, "v": 5, "w": 3},
			{"u": 4, "v": 10, "w": 2},
			{"u": 5, "v": 6, "w": 10},
			{"u": 5, "v": 10, "w": 2},
			{"u": 6, "v": 7, "w": 4},
			{"u": 6, "v": 8, "w": 7},
			{"u": 7, "v": 8, "w": 5},
			{"u": 8, "v": 9, "w": 9}
		],
		"speed": 10,
		"work_time": 8,
		"vehicles": [
			{"id": 1, "depot": 0, "load": 2, "mileage": 35},
			{"id": 2, "depot": 0, "load": 5, "mileage": 35},
		],
		"distancePrior": 5,
		"timePrior": 1,
		"loadPrior": 4
	}
	return problem;
}

function load_cvrp(path) {
	const { readFileSync } = require('fs');
	var data = readFileSync(path, 'utf8').split('\n');
	var state = 0;
	var n_veh = 0, optimum = 0, capacity = 0;
	var problem = {
		"nodes": [], 
		"edges": 'euc2d', 
		"vehicles": [],
		"distancePrior": 1,
		"timePrior": 0,
		"loadPrior": 0,
	};
	for (var i = 0; i < data.length; i++) {
		var line = data[i].trim();
		switch (state) {
			case 0: // header
			var res = line.match(/^COMMENT.*trucks\:\s*(\d+),.*value\:\s*(\d+)/);
			if (res != null) {
				n_veh = parseInt(res[1]);
				optimum = parseInt(res[2]);
				// console.log(n_veh, optimum);
				break;
			}
			res = line.match(/^CAPACITY\s*\:\s*(\d+)/)
			if (res != null) {
				capacity = parseInt(res[1]);
				problem.vehicles.push({"id": 1, "load": capacity, "count": n_veh});
				// console.log(capacity);
				break;
			}
			if (line == 'NODE_COORD_SECTION') {
				state = 1;
			}
			break;

			case 1: // nodes
			if (line == 'DEMAND_SECTION') {
				state = 2;
				break;
			}
			var res = line.split(' ');
			var id = parseInt(res[0]);
			var x = parseInt(res[1]);
			var y = parseInt(res[2]);
			problem.nodes.push({"type": 'customer', "id": id, "x": x, "y": y});
			// console.log(id, x, y);
			break;

			case 2: // demand
			if (line == 'DEPOT_SECTION') {
				state = 3;
				break;
			}
			var res = line.split(' ');
			var id = parseInt(res[0]);
			var demand = parseInt(res[1]);
			problem.nodes[id - 1].demand = demand;
			// console.log(id, demand);
			break;

			case 3: // depot
			var id = parseInt(line);
			if (id == -1) {
				break;
			}
			problem.nodes[id-1].type = 'depot';
			// console.log(id);
			state = 4;
			break;
		}
	}
	console.log('optimum', optimum);
	return problem;
}

function main() {
	// var problem = load_test();
	var problem = load_cvrp("D:\\wkdir\\vrp\\benchmark\\M\\M-n200-k16.vrp");
	// var problem = {
	// 	"nodes":[
	// 		{"id":3, "type":"depot"},
	// 		{"id":5, "type":"customer", "demand":5},
	// 		{"id":4, "type":"customer", "demand":4},
	// 	],
	// 	"edges":[
	// 		{"u":3,"v":4,"w":2},
	// 		{"u":3,"v":5,"w":3},
	// 		{"u":5,"v":4,"w":1},
	// 	],
	// 	"speed":1,
	// 	"vehicles":[
	// 		{"id":11,"load":5,"mileage":10,"count":1},
	// 		{"id":12,"load":4,"mileage":10,"count":1},
	// 	],
	// 	"distancePrior": 0,
	// 	"timePrior": 0,
	// 	"loadPrior": 1
	// };
	solve(problem, -1, -1, 200);
}

main();