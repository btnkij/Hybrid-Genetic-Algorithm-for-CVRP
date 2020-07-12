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

		/// <summary>
		/// rotate the chromosome so that it begins with a vehicle gene
		/// </summary>
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

		/// <summary>
		/// remove the vehicles that have no customer to serve
		/// </summary>
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

		/// <summary>
		/// Evaluate `plan` and fill the field `loss`.
		/// </summary>
		void Evaluate();

		/// <summary>
		/// The crossover operator in MGA.
		/// Child, who has greater loss, will be replaced by the crossover of parent and original child
		/// It did not outperform normal crossover in experiment
		/// </summary>
		/// <param name="parent"></param>
		/// <param name="child"></param>
		friend void MicrobeCrossover(Genome& parent, Genome& child);

		friend Genome Crossover(const Genome& parent1, const Genome& parent2);

		friend Genome MutateRoute(const Genome& genome);

		friend Genome MutateVehicle(const Genome& genome);

		friend void Finetune(Genome& genome);
	};
}
