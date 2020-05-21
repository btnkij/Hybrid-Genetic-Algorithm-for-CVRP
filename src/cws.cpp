
#include "cws.h"

#include <algorithm>
#include <vector>
#include <list>
#include <cassert>

#include "problem.h"
#include "genetic_coding.h"
#include "loss_metrics.h"


namespace VRP
{
	int FindVehicle(const std::vector<int> veh, const std::vector<int>& num, const LossMetrics& loss)
	{
		int vid = -1;
		for (int i : veh)
		{
			const Vehicle& veh = problem.vehicles[i];
			if ((veh.maxMileage < 0 || veh.maxMileage >= loss.dist) 
				&& veh.maxLoad >= loss.load
				//&& veh.workTime >= loss.time
				&& (veh.count == -1 || num[i] < veh.count)
				&& (vid == -1 || veh.maxLoad < problem.vehicles[vid].maxLoad))
			{
				vid = i;
			}
		}
		return vid;
	}

	void CWS_SingleDepot(
		std::vector<int>& cus, const std::vector<int>& veh, 
		std::vector<int>& num, int depotID, Plan& plan)
	{
		std::vector<CWSNode> nodes;
		nodes.reserve(cus.size());
		for (int cid : cus)
		{
			CWSNode node;
			node.route.push_back(cid);
			node.loss.dist = problem.dis[depotID][cid] * 2;
			node.loss.time = node.loss.dist / Vehicle::speed + problem.customers[cid].serviceTime;
			node.loss.load = problem.customers[cid].demand;
			node.vid = -1;
			nodes.push_back(node);
		}

		std::vector<Saving> savings;
		savings.reserve(cus.size() * (cus.size() - 1) / 2);
		for (int i = 1; i < cus.size(); i++)
		{
			for (int j = 0; j < i; j++)
			{
				int c1 = cus[i], c2 = cus[j];
				savings.emplace_back(i, j, 
					problem.dis[depotID][c1] + problem.dis[depotID][c2] - problem.dis[c1][c2]);
			}
		}
		std::sort(savings.begin(), savings.end()); // descending order

		DSU dsu(cus.size());

		for (const auto& it : savings) // parallel merge
		{
			if (it.d <= 0)break;
			int rx = dsu.FindRoot(it.x), ry = dsu.FindRoot(it.y);
			int c1 = cus[it.x], c2 = cus[it.y];
			if (rx == ry)
				continue;
			if (nodes[rx].route.front() != c1 && nodes[rx].route.back() != c1)
				continue;
			if (nodes[ry].route.front() != c2 && nodes[ry].route.back() != c2)
				continue;
			if (nodes[rx].route.size() > 1 && nodes[ry].route.size() > 1 && (
				nodes[rx].route.front() == c1 && nodes[ry].route.front() == c2
				|| nodes[rx].route.back() == c1 && nodes[ry].route.back() == c2))
			{
				//continue;
				if (nodes[rx].route.size() < nodes[ry].route.size())
					std::swap(rx, ry);
				std::reverse(nodes[ry].route.begin(), nodes[ry].route.end());
			}
			if (nodes[rx].route.front() == c1 && nodes[ry].route.back() == c2)
				std::swap(rx, ry);
			LossMetrics loss = nodes[rx].loss;
			loss.Update(nodes[ry].loss);
			loss.dist -= it.d;
			loss.time -= it.d / Vehicle::speed;
			if (nodes[rx].vid != -1)num[nodes[rx].vid]--;
			if (nodes[ry].vid != -1)num[nodes[ry].vid]--;
			int vid = FindVehicle(veh, num, loss);
			if (vid != -1)
			{
				num[vid]++;
				nodes[rx].route.splice(nodes[rx].route.end(), nodes[ry].route);
				nodes[rx].loss = loss;
				nodes[rx].vid = vid;
				dsu.fa[ry] = rx;
			}
			else
			{
				if (nodes[rx].vid != -1)num[nodes[rx].vid]++;
				if (nodes[ry].vid != -1)num[nodes[ry].vid]++;
			}
		}

		for (int i = 0; i < cus.size(); i++)
		{
			if (dsu.FindRoot(i) != i)continue;
			if (nodes[i].vid == -1)
			{
				nodes[i].vid = FindVehicle(veh, num, nodes[i].loss);
			}
			if (nodes[i].vid != -1)
			{
				plan.push_back(Gene::VehicleGene(nodes[i].vid));
			}
			for (int x : nodes[i].route)
			{
				plan.push_back(Gene::CustomerGene(x));
			}
		}
	}

	Genome CWS()
	{
		std::vector<int> neighbor(problem.nCustomer);
		for (int i = 0; i < problem.nCustomer; i++)
		{
			auto argmin = std::min_element(
				problem.dis[i].begin() + problem.nCustomer,
				problem.dis[i].begin() + problem.nCustomer + problem.nDepot);
			neighbor[i] = std::distance(problem.dis[i].begin(), argmin);
		}
		std::vector<int> cus, veh, num;
		cus.reserve(problem.nCustomer);
		veh.reserve(problem.nVehicle);
		num.resize(problem.nVehicle, 0);
		Genome genome;
		for (int i = problem.nCustomer; i < problem.nCustomer + problem.nDepot; i++)
		{
			cus.clear();
			for (int j = 0; j < problem.nCustomer; j++)
			{
				if (neighbor[j] == i)
				{
					cus.push_back(j);
				}
			}
			veh.clear();
			for (int j = 0; j < problem.nVehicle; j++)
			{
				if (problem.vehicles[j].depot == i)
				{
					veh.push_back(j);
				}
			}
			CWS_SingleDepot(cus, veh, num, i, genome.plan);
		}
		genome.num.resize(problem.nVehicle, 0);
		for (auto& g : genome.plan)
		{
			if (g.type == VEHICLE)
			{
				genome.num[g.vtype]++;
			}
		}
		genome.Evaluate();
		return genome;
	}
}