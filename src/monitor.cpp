
#include <iostream>
#include <tuple>

#include "monitor.h"

namespace VRP
{
	void Monitor::Update(int step, const LossMetrics& loss)
	{
		std::cout << "msg {"
			<< R"("step":)" << step
			<< R"(,"penalty":)" << loss.penalty
			<< R"(,"cost":)" << loss.cost
			<< R"(,"distance":)" << loss.dist
			<< R"(,"time":)" << loss.time
			<< R"(,"loadFactor":)" << loss.LoadFactor()
			<< "}" << std::endl;
	}

	void Monitor::Show()
	{

	}
}