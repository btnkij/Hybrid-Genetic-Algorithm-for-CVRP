
#include <limits>
#include <random>
#include <mutex>
#include <vector>
#include <list>
#include <unordered_map>
#include <array>

#include "problem.h"
#include "loss_metrics.h"
#include "genetic_coding.h"
#include "util.h"


namespace VRP
{
	LossMetrics EvaluateTrip(
		Plan::const_iterator& entry, const Plan::const_iterator& end,
		const Vehicle& veh, const bool verbose = false);

	LossMetrics EvaluateJourney(
		Plan::const_iterator& entry, const Plan::const_iterator& end, 
		const bool verbose = false);

	LossMetrics EvaluatePlan(const Plan& plan, const bool verbose = false);

	Genome RandomGenome();

	//std::vector<Plan::iterator> GetVehicleIndex(Genome& genome)
	//{
	//	genome.FindEntry();
	//	std::vector<Plan::iterator> vehs;
	//	for (auto it = genome.plan.begin(); it != genome.plan.end(); ++it)
	//	{
	//		if (it->type == VEHICLE)
	//		{
	//			vehs.push_back(it);
	//		}
	//	}
	//	vehs.push_back(genome.plan.end());
	//	return vehs;
	//}

	Genome ReinsertRoute(const Genome genome);

	Genome SwapRoute(const Genome& genome);

	Genome RotateRoute(const Genome& genome);

	Genome ReverseRoute(const Genome& genome);

	Genome RemoveVehicle(const Genome& genome);

	//Genome AddVehicle(const Genome& genome);

	struct Population
	{
		static constexpr int popsize = 100;
		static constexpr double eliteRate = 0.5;
		static constexpr double crossoverRate = 0.6;
		static constexpr double mutationRate = 0.05;
		static constexpr double finetuneRate = 0.1;
		//static constexpr double maxConcentration = 0.07;

		Genome bestIndiv;
		std::vector<Genome> population;

		Population();
		void Select();
		const Genome& BestIndiv()const;
	};

	void HGA(int maxiter, const bool verbose = false);
}