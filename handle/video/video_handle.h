/**
 * @brief 视频处理高层接口
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include "codec.h"
#include "lynppi.h"

#include <atomic>
#include <functional>
#include <mutex>
#include <vector>

namespace lystream
{
    using namespace lynppi;
    using namespace lycodec;

    class VideoHandle
    {
      public:

        typedef struct {
            CodeSize size;
            unsigned char *plane[3];
        } VideoData;

        explicit VideoHandle(CodeSize size);
        virtual ~VideoHandle();

        virtual void ImageProcessing(void *indata, void **outdata) = 0;

      protected:

        VideoData img_data_ {};
    };
} // namespace lystream