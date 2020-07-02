"use strict";

const { spawn } = require('child_process');
const os = require('os');

function onMessageReceived(data) {
	// console.log('msg');
	// console.log(data);
}

function onFinReceived(data) {
	// console.log('fin');
	// console.log(data);
}

function onSolReceived(data) {
	// console.log('sol');
	console.log(data);
}

function solve(
	problem,			// VRP问题描述
	npop=4,				// 种群数量，无效参数，做兼容性保留
	popsize=100,		// 种群大小，无效参数，做兼容性保留
	maxiter=200,		// 迭代次数
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
		while (true) {
			let eol = out.indexOf(os.EOL);
			if (eol == -1) {
				break;
			}
			let data = out.substr(0, eol);
			let [flag, payload] = data.split(' ');
			// payload = eval('(' + payload + ')');
			switch(flag) {
			case 'msg':
				onMessageReceived(payload);
				break;
			case 'fin':
				onFinReceived(payload);
				break;
			case 'sol':
				onSolReceived(payload);
				break;
			default:
				throw 'unknown flag';
			}			
			out = out.substr(eol + os.EOL.length);
		}
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
	// var problem = load_cvrp("D:/Application/cpp/VehicleRouting/benchmark/A-n33-k5.vrp")
	let problem = {
	"routeMode": false,
	"nodes": [{
		"type": "depot",
		"id": 1,
		"demand": 0,
		"x": 82,
		"y": 76
	}, {
		"type": "customer",
		"id": 2,
		"demand": 19,
		"x": 96,
		"y": 44
	}, {
		"type": "customer",
		"id": 3,
		"demand": 21,
		"x": 50,
		"y": 5
	}, {
		"type": "customer",
		"id": 4,
		"demand": 6,
		"x": 49,
		"y": 8
	}, {
		"type": "customer",
		"id": 5,
		"demand": 19,
		"x": 13,
		"y": 7
	}, {
		"type": "customer",
		"id": 6,
		"demand": 7,
		"x": 29,
		"y": 89
	}, {
		"type": "customer",
		"id": 7,
		"demand": 12,
		"x": 58,
		"y": 30
	}, {
		"type": "customer",
		"id": 8,
		"demand": 16,
		"x": 84,
		"y": 39
	}, {
		"type": "customer",
		"id": 9,
		"demand": 6,
		"x": 14,
		"y": 24
	}, {
		"type": "customer",
		"id": 10,
		"demand": 16,
		"x": 2,
		"y": 39
	}, {
		"type": "customer",
		"id": 11,
		"demand": 8,
		"x": 3,
		"y": 82
	}, {
		"type": "customer",
		"id": 12,
		"demand": 14,
		"x": 5,
		"y": 10
	}, {
		"type": "customer",
		"id": 13,
		"demand": 21,
		"x": 98,
		"y": 52
	}, {
		"type": "customer",
		"id": 14,
		"demand": 16,
		"x": 84,
		"y": 25
	}, {
		"type": "customer",
		"id": 15,
		"demand": 3,
		"x": 61,
		"y": 59
	}, {
		"type": "customer",
		"id": 16,
		"demand": 22,
		"x": 1,
		"y": 65
	}, {
		"type": "customer",
		"id": 17,
		"demand": 18,
		"x": 88,
		"y": 51
	}, {
		"type": "customer",
		"id": 18,
		"demand": 19,
		"x": 91,
		"y": 2
	}, {
		"type": "customer",
		"id": 19,
		"demand": 1,
		"x": 19,
		"y": 32
	}, {
		"type": "customer",
		"id": 20,
		"demand": 24,
		"x": 93,
		"y": 3
	}, {
		"type": "customer",
		"id": 21,
		"demand": 8,
		"x": 50,
		"y": 93
	}, {
		"type": "customer",
		"id": 22,
		"demand": 12,
		"x": 98,
		"y": 14
	}, {
		"type": "customer",
		"id": 23,
		"demand": 4,
		"x": 5,
		"y": 42
	}, {
		"type": "customer",
		"id": 24,
		"demand": 8,
		"x": 42,
		"y": 9
	}, {
		"type": "customer",
		"id": 25,
		"demand": 24,
		"x": 61,
		"y": 62
	}, {
		"type": "customer",
		"id": 26,
		"demand": 24,
		"x": 9,
		"y": 97
	}, {
		"type": "customer",
		"id": 27,
		"demand": 2,
		"x": 80,
		"y": 55
	}, {
		"type": "customer",
		"id": 28,
		"demand": 20,
		"x": 57,
		"y": 69
	}, {
		"type": "customer",
		"id": 29,
		"demand": 15,
		"x": 23,
		"y": 15
	}, {
		"type": "customer",
		"id": 30,
		"demand": 2,
		"x": 20,
		"y": 70
	}, {
		"type": "customer",
		"id": 31,
		"demand": 14,
		"x": 85,
		"y": 60
	}, {
		"type": "customer",
		"id": 32,
		"demand": 9,
		"x": 98,
		"y": 5
	}],
	"edges": "euc2d",
	"vehicles": [{
		"id": "东风",
		"depot": 1,
		"load": 100
	}],
	"distancePrior": 5,
	"timePrior": 1,
	"loadPrior": 4,
	"speed": 10,
	"maxiter": 200
};
	solve(problem, -1, -1, 200);
}

main();