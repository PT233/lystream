/**
 * @brief 算法处理高层接口
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include "lynppi.h"

#include <atomic>
#include <functional>
#include <mutex>
#include <vector>

namespace lystream
{
    using namespace lynppi;

    class Algorithm
    {
      public:

        typedef struct {
            ImgSize size;
            void *data;
        } AlgoData;

        explicit Algorithm(ImgSize size);
        virtual ~Algorithm();

        virtual void AlgoHandle(void *indata, void **outdata) = 0;

      protected:

        AlgoData algo_data_ {};
    };
} // namespace lystream