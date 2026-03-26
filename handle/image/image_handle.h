/**
 * @brief 图像处理高层接口
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

    class ImageHandle
    {
      public:

        typedef struct {
            ImgSize size;
            unsigned char *plane[3];
        } ImgData;

        explicit ImageHandle(ImgSize size);
        virtual ~ImageHandle();

        virtual void ImageProcessing(void *indata, void **outdata) = 0;

      protected:

        ImgData img_data_ {};
        inline static float compute_capability_ = 0.0f; ///< GPU计算能力
    };
} // namespace lystream