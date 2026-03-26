/**
 * @brief Gray转YUV420转换，带有Resize
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include "image_handle.h"
#include "lynppi.h"

namespace lystream
{
    using namespace lynppi;

    class Gray2YUV420withResize : public ImageHandle
    {
      public:

        explicit Gray2YUV420withResize(ImgSize size, ImgSize resize);
        ~Gray2YUV420withResize() override;

        void ImageProcessing(void *indata, void **outdata) override;

      private:

        shared_ptr<NPPIHelper> resize_convert_;
        shared_ptr<ImageMat> gray_;
        shared_ptr<ImageMat> resize_;
        cudaStream_t stream_ {};
    };
} // namespace lystream