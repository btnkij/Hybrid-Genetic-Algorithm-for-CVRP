
#include "ga.h"

#include <iostream>
#include <ctime>
#include <cassert>
#include <memory>
#include <algorithm>
#include <numeric>
#include <thread>
#include <unordered_set>

#include "problem.h"
#include "loss_metrics.h"
#include "genetic_coding.h"
#include "cws.h"
#include "util.h"


namespace VRP
{
	LossMetrics EvaluateTrip(
		Plan::const_iterator& entry, const Plan::const_iterator& end,
		const Vehicle& veh, const bool verbose)
	{
		assert(entry->type == 0);
		static std::vector<int> path;
		if (verbose)
		{
			std::cout << "{"
				<< R"("serve":[)";
			path.clear();
		}
		LossMetrics loss;
		int pre = veh.depot;
		for (; ; ++entry)
		{
			int cur = entry->cid;
			if (verbose)
			{
				std::cout << problem.nodeID[cur];
				problem.getPath(pre, cur, path);
				path.pop_back();
			}
			loss.dist += problem.dis[pre][cur];
			loss.time += problem.dis[pre][cur] / veh.speed;
			loss.load += problem.customers[cur].demand;
			pre = cur;
			if (problem.customers[cur].twbeg > 0)
			{
				loss.time = std::max(loss.time, problem.customers[cur].twbeg);
			}
			if (problem.customers[cur].twend > 0)
			{
				double twViolation = 0;
				twViolation = std::max(twViolation, problem.customers[cur].twbeg - loss.time);
				twViolation = std::max(twViolation, loss.time - problem.customers[cur].twend);
				loss.penalty += twViolation;
			}
			loss.time += problem.customers[cur].serviceTime;
			if (std::next(entry) == end 
				|| std::next(entry)->type != 0 
				//|| entry->back
				|| loss.load + problem.customers[std::next(entry)->cid].demand > veh.maxLoad
				)
			{
				loss.maxLoad += veh.maxLoad;
				loss.dist += problem.dis[cur][veh.depot];
				loss.time += problem.dis[cur][veh.depot] / veh.speed;
				loss.penalty += std::max(0., loss.load - veh.maxLoad);
				break;
			}
			if (verbose)
			{
				std::cout << ",";
			}
		}
		++entry;
		if (verbose)
		{
			problem.getPath(pre, veh.depot, path);
			std::cout << "],"
				<< R"("route":[)";
			for (auto it = path.begin(); it != path.end(); ++it)
			{
				if (it != path.begin())std::cout << ",";
				std::cout << problem.nodeID[*it];
			}
			std::cout << "],"
				<< R"("distance":)" << loss.dist
				<< R"(,"time":)" << loss.time
				<< R"(,"load":)" << loss.load
				<< "}";
		}
		return loss;
	}

	LossMetrics EvaluateJourney(
		Plan::const_iterator& entry, const Plan::const_iterator& end,
		const bool verbose)
	{
		assert(entry->type == 1);
		if (verbose)
		{
			std::cout << "{"
				<< R"("vid":)" << problem.vehicleID[entry->vtype]
				<< R"(,"trips":[)";
		}
		LossMetrics loss;
		const Vehicle& veh = problem.vehicles[entry->vtype];
		auto it = std::next(entry);
		while (it != end && it->type == 0)
		{
			if (verbose && it != std::next(entry))
			{
				std::cout << ",";
			}
			loss.Update(EvaluateTrip(it, end, veh, verbose));
		}
		if (veh.maxMileage > 0)
		{
			loss.penalty += std::max(0., loss.dist - veh.maxMileage);
		}
		if (Vehicle::workTime > 0)
		{
			loss.penalty += std::max(0., loss.time - Vehicle::workTime);
		}
		if (verbose)
		{
			std::cout << "]"
				<< R"(,"distance":)" << loss.dist
				<< R"(,"time":)" << loss.time
				<< R"(,"loadFactor":)" << loss.LoadFactor()
				<< "}";
		}
		entry = it;
		return loss;
	}

	LossMetrics EvaluatePlan(const Plan& plan, const bool verbose)
	{
		assert(plan.begin()->type == 1);
		LossMetrics loss;
		if (verbose)
		{
			std::cout << R"({"plan":[)";
		}
		auto entry = plan.begin();
		while (entry != plan.end())
		{
			if (verbose && entry != plan.begin())
			{
				std::cout << ",";
			}
			loss.Merge(EvaluateJourney(entry, plan.end(), verbose));
		}
		if (verbose)
		{
			std::cout << R"(],"distance":)" << loss.dist
				<< R"(,"time":)" << loss.time
				<< R"(,"loadFactor":)" << loss.LoadFactor()
				<< "}" << std::endl;
		}
		return loss;
	}

	/**
	* 交叉
	**/
	void MicrobeCrossover(Genome& parent, Genome& child)
	{
		auto x = std::next(parent.plan.begin(), randint(0, parent.plan.size()));
		std::rotate(parent.plan.begin(), x, parent.plan.end());
		x = parent.plan.begin();
		auto y = std::next(x, randint(1, parent.plan.size()));
		std::unordered_set<int> vis;
		for (auto it = x; it != y; ++it)
		{
			if (it->type == 0)
				vis.insert(it->cid);
			else
				child.num[it->vtype]++;
		}
		for (auto it = child.plan.begin(); it != child.plan.end(); )
		{
			auto nxt = std::next(it);
			if (it->type == 0 && vis.count(it->cid))
			{
				child.plan.erase(it);
			}
			else if (it->type == 1)
			{
				if (problem.vehicles[it->vtype].count != -1 && child.num[it->vtype] > problem.vehicles[it->vtype].count)
				{
					child.num[it->vtype]--;
					child.plan.erase(it);
				}
				else if (it != child.plan.begin())
				{
					auto pre = std::prev(it);
					if (pre->type == 1)
					{
						child.num[pre->vtype]--;
						child.plan.erase(pre);
					}
				}
			}
			it = nxt;
		}
		child.plan.insert(child.plan.end(), x, y);
		child.RemoveSpareVehicles();
		child.Evaluate();
	}

	Genome Crossover(const Genome& parent1, const Genome& parent2)
	{
		auto x = std::next(parent1.plan.begin(), randint(0, parent1.plan.size() - 1));
		int y = randint(1, parent1.plan.size());
		std::unordered_set<int> vis;
		Genome child;
		child.num.resize(problem.nVehicle);
		auto it = x;
		for (int i = 0; i < y; i++)
		{
			if (it == parent1.plan.end())
			{
				it = parent1.plan.begin();
			}
			child.plan.push_back(*it);
			if (it->type == 0)
				vis.insert(it->cid);
			else
				child.num[it->vtype]++;
			++it;
		}
		for (it = parent2.plan.begin(); it != parent2.plan.end(); ++it)
		{
			if (it->type == CUSTOMER)
			{
				if (!vis.count(it->cid))
				{
					child.plan.push_back(*it);
				}
			}
			else
			{
				if (std::next(it) == parent2.plan.end() && child.plan.front().type == VEHICLE)
				{
					break;
				}
				if (child.plan.back().type == VEHICLE)
				{
					child.num[child.plan.back().vtype]--;
					child.plan.pop_back();
				}
				if (problem.vehicles[it->vtype].count == -1 
					|| child.num[it->vtype] < problem.vehicles[it->vtype].count)
				{
					child.plan.push_back(*it);
					child.num[it->vtype]++;
				}
			}
		}
		child.RemoveSpareVehicles();
		child.Evaluate();
		return child;
	}

	Genome RemoveVehicle(const Genome& genome)
	{
		auto tmp = genome;
		auto x = std::next(tmp.plan.begin(), randint(0, tmp.plan.size() - 1));
		std::rotate(tmp.plan.begin(), x, tmp.plan.end());
		auto it = std::find_if(tmp.plan.begin(), tmp.plan.end(), [](Gene& g) {
			return g.type == VEHICLE;
			});
		tmp.num[it->vtype]--;
		tmp.plan.erase(it);
		tmp.Evaluate();
		return tmp;
	}

	//Genome AddVehicle(const Genome& genome)
	//{
	//	auto tmp = genome;
	//	int vtype = randint(0, problem.nVehicle - 1);
	//	if (problem.vehicles[vtype].count == -1 || tmp.num[vtype] < problem.vehicles[vtype].count)
	//	{
	//		auto x = std::next(tmp.plan.begin(), randint(0, tmp.plan.size()));
	//		tmp.num[vtype]++;
	//		tmp.plan.insert(x, Gene::VehicleGene(vtype));
	//		tmp.RemoveSpareVehicles();
	//	}
	//}

	/**
	* 变异
	**/
	Genome MutateVehicle(const Genome& genome)
	{
		return RemoveVehicle(genome);
	}

	Genome MutateRoute(const Genome& genome)
	{
		int k = randint(0, 3);
		switch (k)
		{
		case 0:
			return SwapRoute(genome);
		case 1:
			return RotateRoute(genome);
		case 2:
			return ReverseRoute(genome);
		case 3:
			return ReinsertRoute(genome);
		default:
			assert(0);
		}
	}

	void Finetune(Genome& genome)
	{
		auto best = genome;
		// re-insert
		for (int k = 0; k < 3; k++)
		{
			auto tmp = ReinsertRoute(genome);
			if (tmp < best)best = tmp;
		}
		// swap
		for (int k = 0; k < 3; k++)
		{
			auto tmp = SwapRoute(genome);
			if (tmp < best)best = tmp;
		}
		// reverse
		for (int k = 0; k < 3; k++)
		{
			auto tmp = ReverseRoute(genome);
			if (tmp < best)best = tmp;
		}
		// change vehicle
		/*for (int k = 0; k < 3; k++)
		{
			auto tmp = *this;
			auto x = std::next(tmp.plan.begin(), randint(0, tmp.plan.size() - 1));
			std::rotate(tmp.plan.begin(), x, tmp.plan.end());
			auto v = std::find_if(tmp.plan.begin(), tmp.plan.end(), [](Gene& g) {
				return g.type == 1;
				});
			std::rotate(tmp.plan.begin(), v1, tmp.plan.end());
			tmp.Evaluate();
			if (tmp < best)best = tmp;
		}*/
		if (best < genome)
			genome = best;
	}

	inline void Genome::Evaluate()
	{
		FindEntry();
		auto loss = EvaluatePlan(plan);
		this->loss = loss.Result(std::accumulate(num.begin(), num.end(), 0));
	}

	Genome RandomGenome()
	{
		std::vector<int> route(problem.nCustomer);
		std::iota(route.begin(), route.end(), 0);
		randshuffle(route.begin(), route.end());
		Genome genome;
		genome.num.assign(problem.nVehicle, 0);
		int vtype = randint(0, problem.nVehicle - 1);
		genome.plan.push_back(Gene::VehicleGene(vtype));
		genome.num[vtype]++;
		for (int cid : route)
		{
			if (!genome.plan.empty() && genome.plan.rbegin()->type != 1 && randprob() < 0.1)
			{
				vtype = randint(0, problem.nVehicle - 1);
				if (problem.vehicles[vtype].count == -1 || genome.num[vtype] < problem.vehicles[vtype].count)
				{
					genome.plan.push_back(Gene::VehicleGene(vtype));
					genome.num[vtype]++;
				}
			}
			genome.plan.push_back(Gene::CustomerGene(cid));
		}
		genome.Evaluate();
		//genome.Finetune();
		return genome;
	}

	Genome ReinsertRoute(const Genome genome)
	{
		auto tmp = genome;
		int cid = randint(0, problem.nCustomer - 1);
		auto g = std::find_if(tmp.plan.begin(), tmp.plan.end(), [cid](Gene& g) {
			return g.type == 0 && g.cid == cid;
			});
		auto x = std::next(tmp.plan.begin(), randint(0, tmp.plan.size()));
		tmp.plan.insert(x, *g);
		tmp.plan.erase(g);
		tmp.RemoveSpareVehicles();
		tmp.Evaluate();
		return tmp;
	}

	Genome SwapRoute(const Genome& genome)
	{
		auto tmp = genome;
		auto p = randpair(0, problem.nCustomer - 1);
		auto c1 = std::find_if(tmp.plan.begin(), tmp.plan.end(), [p](Gene& g) {
			return g.type == 0 && g.cid == p.first;
			});
		auto c2 = std::find_if(tmp.plan.begin(), tmp.plan.end(), [p](Gene& g) {
			return g.type == 0 && g.cid == p.second;
			});
		std::swap(c1->cid, c2->cid);
		tmp.Evaluate();
		return tmp;
	}

	Genome RotateRoute(const Genome& genome)
	{
		auto tmp = genome;
		auto x = std::next(tmp.plan.begin(), randint(0, tmp.plan.size() - 1));
		std::rotate(tmp.plan.begin(), x, tmp.plan.end());
		auto p = randpair(0, tmp.plan.size() - 1);
		int m = randint(p.first, p.second);
		x = std::next(tmp.plan.begin(), p.first);
		auto y = std::next(x, m - p.first);
		auto z = std::next(y, p.second - m);
		std::rotate(x, y, z);
		tmp.RemoveSpareVehicles();
		tmp.Evaluate();
		return tmp;
	}

	Genome ReverseRoute(const Genome& genome)
	{
		auto tmp = genome;
		auto x = std::next(tmp.plan.begin(), randint(0, tmp.plan.size() - 1));
		std::rotate(tmp.plan.begin(), x, tmp.plan.end());
		x = tmp.plan.begin();
		auto y = std::next(x, randint(1, tmp.plan.size()));
		std::reverse(x, y);
		tmp.RemoveSpareVehicles();
		tmp.Evaluate();
		return tmp;
	}

	Population::Population()
	{
		population.reserve(popsize * 4);
		for (int i = 0; i < popsize; i++)
		{
			population.push_back(RandomGenome());
		}
		bestIndiv = *std::min_element(population.begin(), population.end());
	}

	void Population::Select()
	{
		int nElite = int(popsize * eliteRate);
		auto mid = population.begin() + nElite;
		auto end = population.begin() + popsize;
		std::nth_element(population.begin(), mid, population.end());
		for (int i = int(population.size() - 1); i >= popsize; i--)
		{
			int j = randint(nElite, popsize);
			if (population[i] < population[j] || randprob() < 0.2)
			{
				population[j] = population[i];
			}
		}
		population.erase(end, population.end());
	}

	const Genome& Population::BestIndiv()const
	{
		return bestIndiv;
	}

	void HGA(int maxiter, const bool verbose)
	{
		constexpr int n = 4;
		constexpr double migrationRate = 0.2;
		int nMigration = int(Population::popsize * migrationRate);

		clock_t startTime, endTime;
		if (verbose)
		{
			std::cout << "begin optimizing" << "\n\n";
			startTime = clock();
		}
		
		srand(std::time(nullptr));
		Genome best;
		best.loss.first = INF;
		std::unique_ptr<Population[]> pops(new Population[n]);
		pops[0].population.push_back(CWS());
		std::unique_ptr<std::thread[]> threads(new std::thread[n]);

		for (int step = 0; step < maxiter; )
		{
			int stepsize = std::min(15, maxiter - step);
			for (int k = 0; k < n; k++)
			{
				threads[k] = std::thread([](Population* cur, int nGeneration) {

					srand((unsigned)std::time(nullptr) + (unsigned)cur * 23333U);
					//std::unordered_map<int, std::list<int> > niche;
					//int maxn = Population::popsize * Population::maxConcentration;
					for (int step = 0; step < nGeneration; ++step)
					{
						//niche.clear();
						//for (int i = 0; i < cur->population.size(); i++)
						//{
						//	if (randprob() < 0.5)
						//	{
						//		niche[cur->population[i].Hash()].push_front(i);
						//	}
						//	else
						//	{
						//		niche[cur->population[i].Hash()].push_back(i);
						//	}
						//}

						for (int i = 0; i < Population::popsize; i++)
						{
							if (randprob() > Population::mutationRate)
								continue;
							int j = randint(0, Population::popsize - 1);
							auto child = Crossover(cur->population[i], cur->population[j]);
							while (randprob() < Population::mutationRate)
							{
								cur->population.push_back(MutateRoute(child));
							}
							if (child.plan.size() - problem.nCustomer > 1)
							{
								while (randprob() < Population::mutationRate)
								{
									cur->population.push_back(MutateVehicle(child));
								}
							}
							//int h = child.Hash();
							//if (niche[h].size() >= maxn)
							//{
							//	int t = niche[h].front();
							//	niche[h].pop_front();
							//	cur->population[t] = child;
							//	niche[h].push_back(t);
							//}
							//else
							//{
								//niche[h].push_back(cur->population.size());
								cur->population.push_back(child);
							//}
						}
						
						cur->Select();

						for (int i = 0; i < Population::popsize; i++)
						{
							if (randprob() < Population::finetuneRate)
							{
								Finetune(cur->population[i]);
							}
						}
					}
					
					}, &pops[k], stepsize);
			}
			step += stepsize;
			for (int k = 0; k < n; k++)
			{
				threads[k].join();
			}

			// migrate
			if (step % 15 == 0)
			{
				for (int i = 0; i < n; i++)
				{
					auto& x = pops[i].population;
					auto midx = x.begin() + nMigration;
					std::nth_element(x.begin(), midx, x.end());
					int j = 0;
					if (j >= 2)j = randint(2, n - 2);
					if (j >= i)j++;
					auto& y = pops[j].population;
					y.insert(y.end(), x.begin(), midx);
				}
				for (int i = 2; i < n; i++)
				{
					pops[i].Select();
					int j = i % 2;
					auto beg = pops[i].population.begin();
					pops[j].population.insert(pops[j].population.end(), beg, beg + nMigration);
				}
				for (int i = 0; i < 2; i++)
				{
					pops[i].Select();
				}
			}

			// update solution
			for (int k = 0; k < n; k++)
			{
				auto cur = &pops[k];
				cur->bestIndiv = *std::min_element(cur->population.begin(), cur->population.end());
			}

			if (verbose)
			{
				std::cout << "step " << step << ":\n";
				for (int i = 0; i < n; i++)
				{
					std::cout << "pop#" << i << ": "
						<< "penalty=" << pops[i].BestIndiv().loss.first
						<< ", cost=" << pops[i].BestIndiv().loss.second
						<< "\n";
				}
			}
		}

		for (int i = 0; i < n; i++)
		{
			if (pops[i].BestIndiv() < best)
			{
				best = pops[i].BestIndiv();
			}
		}
		
		if (verbose)
		{
			endTime = clock();
			double runTime = (double(endTime) - startTime) / CLOCKS_PER_SEC;
			std::cout << "\ntime consumption: " << runTime << " sec\n" << std::endl;
		}

		best.FindEntry();
		EvaluatePlan(best.plan, true);
	}
}