
#include <iostream>
#include <string>

#include "problem.h"
#include "cws.h"
#include "ga.h"


int maxiter;
void init()
{
#ifdef _DEBUG
	freopen("in.txt", "r", stdin);
	//freopen("out.txt", "w", stdout);
#endif
	using std::cin;
	using namespace VRP;

	std::ios::sync_with_stdio(false);
	cin.tie(nullptr);

	// customer nodes
	cin >> problem.nCustomer;
	problem.customers.reserve(problem.nCustomer);
	for (int i = 0; i < problem.nCustomer; i++)
	{
		int id;
		cin >> id;
		problem.nodeID[i] = id;
		problem.nodeID[MAXN + id] = i;
		Customer customer;
		cin >> customer.demand >> customer.serviceTime >> customer.twbeg >> customer.twend;
		problem.customers.push_back(customer);
	}

	// depot nodes
	cin >> problem.nDepot;
	for (int i = problem.nCustomer; i < problem.nCustomer + problem.nDepot; i++)
	{
		int id;
		cin >> id;
		problem.nodeID[i] = id;
		problem.nodeID[MAXN + id] = i;
	}

	// other nodes
	cin >> problem.nOther;
	for (
		int i = problem.nCustomer + problem.nDepot;
		i < problem.nCustomer + problem.nDepot + problem.nOther; 
		i++)
	{
		int id;
		cin >> id;
		problem.nodeID[i] = id;
		problem.nodeID[MAXN + id] = i;
	}

	problem.nNode = problem.nCustomer + problem.nDepot + problem.nOther;
	for (int i = 0; i < problem.nNode; i++)
	{
		std::fill_n(problem.dis[i].begin(), problem.nNode, INF);
		std::fill_n(problem.via[i].begin(), problem.nNode, -1);
		problem.dis[i][i] = 0;
	}

	// edges
	int nEdge;
	cin >> nEdge;
	for (int i = 0; i < nEdge; i++)
	{
		int u, v;
		double w;
		cin >> u >> v >> w;
		if (w < problem.dis[problem.nodeID[MAXN + u]][problem.nodeID[MAXN + v]])
		{
			problem.dis[problem.nodeID[MAXN + u]][problem.nodeID[MAXN + v]]
				= problem.dis[problem.nodeID[MAXN + v]][problem.nodeID[MAXN + u]] = w;
		}
	}
	problem.floyd();

	// vehicles
	cin >> problem.nVehicle;
	cin >> Vehicle::speed >> Vehicle::workTime;
	problem.vehicles.reserve(problem.nVehicle);
	for (int i = 0; i < problem.nVehicle; i++)
	{
		std::string id;
		cin >> id;
		problem.vehicleID[i] = id;
		//problem.vehicleID[MAXN + id] = i;
		Vehicle veh;
		cin >> veh.depot >> veh.maxLoad >> veh.maxMileage >> veh.count;
		veh.depot = problem.nodeID[MAXN + veh.depot];
		problem.vehicles.push_back(veh);
	}

	// the priority of distance, time, and load factor
	cin >> problem.distancePrior >> problem.timePrior >> problem.loadPrior;

	int npop, popsize;
	cin >> npop >> popsize >> maxiter; // max iterations
}

int main()
{
	init();
#ifdef _DEBUG
	VRP::HGA(maxiter, true);
#else
	VRP::HGA(maxiter, false);
#endif
	return 0;
}