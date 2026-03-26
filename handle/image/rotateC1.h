/**
 * @brief mono8 旋转
 * @date 2023/12/08
 * @author
 */

#pragma once

#include "image_handle.h"
#include "lynppi.h"

namespace lystream
{
    using namespace lynppi;

    class Rotate_C1 : public ImageHandle
    {
      public:

        explicit Rotate_C1(ImgSize size, int angel);
        ~Rotate_C1() override;

        void ImageProcessing(void *indata, void **outdata) override;

      public:

        ImgSize m_rotateSize; // 旋转之后的尺寸

      private:

        shared_ptr<NPPIHelper> rotate_convert;
        shared_ptr<ImageMat> srcMat;
        shared_ptr<ImageMat> dstMat;

        cudaStream_t stream_ {};
    };
} // namespace lystream