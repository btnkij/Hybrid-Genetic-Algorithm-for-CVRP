#pragma once

#include <cstdlib>
#include <utility>
#include <cassert>


/**
* 随机生成闭区间[low, high]中的整数，均匀分布
**/
inline int randint(int low, int high)
{
    //return rand() % (high - low + 1) + low;
    return rand() % (high - low + 1) + low;
}

/**
* 随机[0, 1)的小数，均匀分布
**/
inline double randprob()
{
    //return (double)rand() / RAND_MAX;
    return (double)rand() / RAND_MAX;
}

inline std::pair<int, int> randpair(int low, int high)
{
    assert(high - low + 1 >= 2);
    int x = randint(low, high), y;
    do {
        y = randint(low, high);
    } while (y == x);
    if (x > y)std::swap(x, y);
    return std::make_pair(x, y);
}

template<class RandomIt>
inline void randshuffle(RandomIt first, RandomIt last)
{
    for (int i = (last - first) - 1; i > 0; --i)
    {
        int j = rand() % (i + 1);
        std::swap(first[i], first[j]);
    }
}