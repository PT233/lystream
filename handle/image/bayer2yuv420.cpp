/**
 * @brief Bayer转YUV420基本转换
 * @date 2022/11/28
 * @author laoyao
 */

#include "bayer2yuv420.h"

#include <helper_cuda.h>
#include <thread>

using namespace lystream;
using namespace lynppi::color_conversion;

Bayer2YUV420::Bayer2YUV420(ImgSize size) : ImageHandle(size)
{
    RoiRect roi = {0, 0, size.width, size.height};
    cudaStreamCreateWithFlags(&stream_, cudaStreamDefault);
    int yuvstep[3] = {size.width, size.width / 2, size.width / 2};

    bayer2rgb_  = shared_ptr<NPPIHelper>(new Bayer8toRGB(size, roi, NPPI_BAYER_RGGB, stream_));
    rgb2yuv420_ = shared_ptr<NPPIHelper>(new RGBtoYUV420_C3P3(size, roi, stream_));
    bayer_      = shared_ptr<ImageMat>(new ImageMat8UC1(size.width, size.height, stream_));
    rgb_        = shared_ptr<ImageMat>(new ImageMat8UC3(size.width, size.height, stream_));
    yuv_        = shared_ptr<ImageMat>(new ImageMat8UP3(yuvstep, size.height, stream_));

    bayer_->setAlgorithm(bayer2rgb_.get());
    rgb_->setAlgorithm(rgb2yuv420_.get());
}

Bayer2YUV420::~Bayer2YUV420()
{
    bayer2rgb_  = nullptr;
    rgb2yuv420_ = nullptr;
    bayer_      = nullptr;
    rgb_        = nullptr;
    yuv_        = nullptr;

    cudaStreamDestroy(stream_);
}

void Bayer2YUV420::ImageProcessing(void *indata, void **outdata)
{
    if (compute_capability_ >= 7.2f)
        dynamic_cast<ImageMat8UC1 *>(bayer_.get())
            ->setData((unsigned char *)indata, ImageMat::CopyType::ZeroCopyMemory);
    else dynamic_cast<ImageMat8UC1 *>(bayer_.get())->setData((unsigned char *)indata);
    
    bayer_->Processing(rgb_.get());
    rgb_->Processing(yuv_.get());
    dynamic_cast<ImageMat8UP3 *>(yuv_.get())->getData(img_data_.plane);
    *outdata = &img_data_;
}
