/**
 * @brief Gray转YUV420
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include "image_handle.h"
#include "lynppi.h"

namespace lystream
{
    using namespace lynppi;

    class Gray2YUV420 : public ImageHandle
    {
      public:

        explicit Gray2YUV420(ImgSize size);
        ~Gray2YUV420() override;

        void ImageProcessing(void *indata, void **outdata) override;

      private:
    };
} // namespace lystream