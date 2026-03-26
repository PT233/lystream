/**
 * @brief Bayer转YUV420基本转换
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include "image_handle.h"
#include "lynppi.h"

namespace lystream
{
    using namespace lynppi;

    class Bayer2YUV420 : public ImageHandle
    {
      public:

        explicit Bayer2YUV420(ImgSize size);
        ~Bayer2YUV420() override;

        void ImageProcessing(void *indata, void **outdata) override;

      private:

        shared_ptr<NPPIHelper> bayer2rgb_;
        shared_ptr<NPPIHelper> rgb2yuv420_;
        shared_ptr<ImageMat> bayer_;
        shared_ptr<ImageMat> rgb_;
        shared_ptr<ImageMat> yuv_;

        cudaStream_t stream_ {};
    };
} // namespace lystream