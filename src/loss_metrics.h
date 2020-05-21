#pragma once

#include <utility>

#include "problem.h"


namespace VRP
{
	struct LossMetrics
	{
		double dist;
		double time;
		double load, maxLoad;
		double penalty;

		LossMetrics() :dist(0), time(0), load(0), maxLoad(0), penalty(0) {}

		inline void Update(const LossMetrics& rhs)
		{
			dist += rhs.dist;
			time += rhs.time;
			load += rhs.load, maxLoad += rhs.maxLoad;
			penalty += rhs.penalty;
		}

		inline void Merge(const LossMetrics& rhs)
		{
			dist += rhs.dist;
			time = std::max(time, rhs.time);
			load += rhs.load, maxLoad += rhs.maxLoad;
			penalty += rhs.penalty;
		}

		double LoadFactor() const
		{
			return load / maxLoad;
		}

		std::pair<double, double> Result(int nVeh)
		{
			double norms = this->dist / Vehicle::speed;
			double normt = this->time * nVeh;
			double norml = (1. - this->LoadFactor()) * Vehicle::workTime * nVeh;
			double gcost = norms * problem.distancePrior + normt * problem.timePrior + norml * problem.loadPrior;
			return std::make_pair(this->penalty, gcost);
		}
	};
}
