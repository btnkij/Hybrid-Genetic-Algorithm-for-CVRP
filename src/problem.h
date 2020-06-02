#pragma once

#include <limits>
#include <array>
#include <vector>
#include <unordered_map>


namespace VRP
{
	static constexpr double EPS = 1e-8;
	static constexpr double INF = std::numeric_limits<double>::max() / 4.;
	static constexpr int MAXN = 500; // the maximum number of nodes the graph can contain

	struct Vehicle
	{
		int depot; // the node where the vehicle starts and ends and reloads goods
		double maxLoad; // maximum capacity, unit = t
		int count; // the number of vehicles
		double maxMileage = -1; // maximum mileage, unit = km
		static double speed; // unit = km/h
		static double workTime; // daily work time, unit = h
	};

	struct Customer
	{
		double demand; // unit = t
		double twbeg = -1, twend = -1; // time window begin and end, unit = h
		double serviceTime = 5.0 / 60; // unit = h
	};

	struct Problem
	{
		int nNode; // total number of nodes in the graph
		int nCustomer, nDepot, nOther; // the number of customer, depot, and irrelevant nodes
		int nVehicle; // the number of different types of vehicles
		std::array<std::array<double, MAXN>, MAXN> dis; // adjacency matrix, denoting distance between two nodes
		std::array<std::array<int, MAXN>, MAXN> via; // record the intermediate node in FLOYD algorithm
		std::vector<Vehicle> vehicles;
		std::vector<Customer> customers;
		std::unordered_map<int, int> nodeID;
		std::unordered_map<int, int> vehicleID;
		double distancePrior;	// priority of distance
		double loadPrior;		// priority of load factor
		double timePrior;		// priority of time consumption

		/// <summary>
		/// FLOYD algorithm, to get the shortest distance between any two nodes
		/// </summary>
		void floyd();

		/// <summary>
		/// append the path to std::vector<int> path between from and to
		/// </summary>
		/// <param name="from"></param>
		/// <param name="to"></param>
		/// <param name="path"></param>
		void getPath(int from, int to, std::vector<int>& path)const;
	};

	// describe the problem to be solved
	extern Problem problem;
}
