
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
	/// <summary>
	/// Get the loss of one vehicle in one trip,
	/// where a trip is a route begins and ends at a depot
	/// </summary>
	/// <param name="entry">at which gene the trip begins</param>
	/// <param name="end">the end iterator of plan</param>
	/// <param name="veh">the vehicle responsible for this trip</param>
	/// <param name="verbose">whether to print the detail loss</param>
	/// <returns></returns>
	LossMetrics EvaluateTrip(
		Plan::const_iterator& entry, const Plan::const_iterator& end,
		const Vehicle& veh, const bool verbose = false);

	/// <summary>
	/// Get the loss of one vehicle in its journey,
	/// where a journey is a set of all its trips
	/// </summary>
	/// <param name="entry">at which gene the journey begins</param>
	/// <param name="end">the end iterator of plan</param>
	/// <param name="verbose">whether to print the detail loss</param>
	/// <returns></returns>
	LossMetrics EvaluateJourney(
		Plan::const_iterator& entry, const Plan::const_iterator& end, 
		const bool verbose = false);

	/// <summary>
	/// Get the loss of the whole plan,
	/// where a plan is a set of all journeys
	/// </summary>
	/// <param name="plan"></param>
	/// <param name="verbose">whether to print the detail loss</param>
	/// <returns></returns>
	LossMetrics EvaluatePlan(const Plan& plan, const bool verbose = false);

	/// <summary>
	/// randomly construct a genome
	/// </summary>
	/// <returns></returns>
	Genome RandomGenome();

	void ReinsertRoute(Genome& genome);

	void SwapRoute(Genome& genome);

	void RotateRoute(Genome& genome);

	void ReverseRoute(Genome& genome);

	void RemoveVehicle(Genome& genome);

	void InsertVehicle(Genome& genome);

	Genome MutateRoute(const Genome& genome);

	Genome MutateVehicle(const Genome& genome);

	struct Population
	{
		Genome bestIndiv;
		std::vector<Genome> population;

		Population();
		void Select();
		const Genome& BestIndiv()const;
	};

	/// <summary>
	/// The entry of Hybrid Genetic Algorithm
	/// </summary>
	/// <param name="maxiter"></param>
	/// <param name="verbose">whether to print the optimizing process</param>
	void HGA(int maxiter, const bool verbose = false);
}