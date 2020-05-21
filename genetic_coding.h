#pragma once

#include <utility>
#include <list>
#include <vector>
#include <cassert>


namespace VRP
{
	enum GeneType
	{
		CUSTOMER, VEHICLE
	};

	struct Gene
	{
		GeneType type;
		union
		{
			struct // if type = CUSTOMER
			{
				int cid; // customer id
				//int back; // whether to return to depot after serving cid
			};
			struct // if type = VEHICLE
			{
				int vtype; // type number of the vehicle
			};
		};
		static Gene CustomerGene(int cid)
		{
			Gene g;
			g.type = CUSTOMER;
			g.cid = cid;
			return g;
		}
		static Gene VehicleGene(int vtype)
		{
			Gene g;
			g.type = VEHICLE;
			g.vtype = vtype;
			return g;
		}
	};

	using Plan = std::list<Gene>;

	struct Genome
	{
		Plan plan; // describe the whole routing plan
		std::vector<int> num; // the number of vehicles used in each type
		std::pair<double, double> loss; // {penalty, weighted cost}
		bool operator<(const Genome& rhs)const
		{
			return loss < rhs.loss;
		}
		bool operator>(const Genome& rhs)const
		{
			return loss > rhs.loss;
		}

		inline void FindEntry()
		{
			if (plan.begin()->type == 1)
			{
				return;
			}
			auto entry = std::find_if(plan.begin(), plan.end(), [](const auto& g) {
				return g.type == 1;
				});
			assert(entry != plan.end());
			std::rotate(plan.begin(), entry, plan.end());
		}

		inline void RemoveSpareVehicles()
		{
			for (auto it = plan.begin(); it != plan.end(); )
			{
				auto nxt = std::next(it);
				if (it->type == 1)
				{
					if (nxt != plan.end())
					{
						if (nxt->type == 1)
						{
							num[it->vtype]--;
							plan.erase(it);
						}
					}
					else if (plan.begin()->type == 1)
					{
						num[it->vtype]--;
						plan.erase(it);
					}
				}
				it = nxt;
			}
		}

		inline int Hash()const
		{
			if (loss.first > EPS)
				return -(int)(loss.second * 10 + 0.5);
			else
				return (int)(loss.second * 10 + 0.5);
		}

		void Evaluate();
		friend void MicrobeCrossover(Genome& parent, Genome& child);
		friend Genome Crossover(const Genome& parent1, const Genome& parent2);
		friend Genome MutateRoute(const Genome& genome);
		friend Genome MutateVehicle(const Genome& genome);
		friend void Finetune(Genome& genome);
	};
}
