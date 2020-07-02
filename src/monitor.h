
#include <list>
#include <utility>

#include "loss_metrics.h"

namespace VRP
{
	class Monitor
	{
	public:
		void Update(int step, const LossMetrics& loss);
		void Show();
	};
}