/**
 * @brief 算法处理高层接口
 * @date 2022/11/28
 * @author laoyao
 */

#include "algorithm.h"

lystream::Algorithm::Algorithm(ImgSize size)
{
    algo_data_.size = size;
}

lystream::Algorithm::~Algorithm() = default;