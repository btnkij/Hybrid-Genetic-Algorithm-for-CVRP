
#include "problem.h"

#include <vector>


namespace VRP
{
	double Vehicle::speed;
	double Vehicle::workTime;

	void Problem::floyd()
	{
		for (int k = 0; k < nNode; k++)
		{
			for (int i = 0; i < nNode; i++)
			{
				for (int j = 0; j < i; j++)
				{
					double tmp = dis[i][k] + dis[k][j];
					if (tmp < dis[i][j])
					{
						dis[i][j] = dis[j][i] = tmp;
						via[i][j] = via[j][i] = k;
					}
				}
			}
		}
	}

	void Problem::getPath(int from, int to, std::vector<int>& path)const
	{
		if (via[from][to] == -1)
		{
			path.push_back(from);
			if (from != to)
				path.push_back(to);
			return;
		}
		getPath(from, via[from][to], path);
		path.pop_back();
		getPath(via[from][to], to, path);
	}

	Problem problem;
}
