#pragma once

#include <iostream>
#include <numeric>
#include <algorithm>
#include <vector>

#include "problem.h"
#include "loss_metrics.h"
#include "genetic_coding.h"


namespace VRP
{
	struct CWSNode
	{
		int vid;
		std::list<int> route;
		LossMetrics loss;
	};

	struct DSU // disjoint set union
	{
		std::vector<int> fa;
		DSU(size_t size) :fa(size)
		{
			std::iota(fa.begin(), fa.end(), 0);
		}
		inline int FindRoot(int x)
		{
			return x == fa[x] ? x : fa[x] = FindRoot(fa[x]);
		}
	};

	struct Saving
	{
		int x, y; // the tow customers to be merged
		double d; // saved distance
		Saving(int _x, int _y, double _d) :x(_x), y(_y), d(_d) {}
		bool operator<(const Saving& rhs)const
		{
			return d > rhs.d;
		}
	};

	int FindVehicle(
		const std::vector<Vehicle*> veh, const std::vector<int>& num, 
		const LossMetrics& loss);
	void CWS_SingleDepot(
		std::vector<int>& cus, const std::vector<int>& veh,
		std::vector<int>& num, int depotID, Plan& plan);
	Genome CWS();
}