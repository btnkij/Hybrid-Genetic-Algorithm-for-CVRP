
#include <iostream>

#include "problem.h"
#include "cws.h"
#include "ga.h"


int npop, popsize, maxiter;
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

	cin >> problem.nDepot;
	for (int i = problem.nCustomer; i < problem.nCustomer + problem.nDepot; i++)
	{
		int id;
		cin >> id;
		problem.nodeID[i] = id;
		problem.nodeID[MAXN + id] = i;
	}

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

	int nEdge;
	cin >> nEdge;
	for (int i = 0; i < nEdge; i++)
	{
		int u, v;
		double w;
		cin >> u >> v >> w; // 连接u、v长度为w的双向道路
		problem.dis[problem.nodeID[MAXN + u]][problem.nodeID[MAXN + v]] 
			= problem.dis[problem.nodeID[MAXN + v]][problem.nodeID[MAXN + u]] = w;
	}
	problem.floyd();

	cin >> problem.nVehicle; // 货车数量
	cin >> Vehicle::speed >> Vehicle::workTime;
	problem.vehicles.reserve(problem.nVehicle);
	for (int i = 0; i < problem.nVehicle; i++)
	{
		int id;
		cin >> id;
		problem.vehicleID[i] = id;
		problem.vehicleID[MAXN + id] = i;
		Vehicle veh;
		cin >> veh.depot >> veh.maxLoad >> veh.maxMileage >> veh.count;
		veh.depot = problem.nodeID[MAXN + veh.depot];
		problem.vehicles.push_back(veh);
	}

	cin >> problem.distancePrior >> problem.timePrior >> problem.loadPrior; // 路程加权、用时加权、满载率加权

	cin >> npop >> popsize >> maxiter; // 种群数量、种群大小、迭代次数
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