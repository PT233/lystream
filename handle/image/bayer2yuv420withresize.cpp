/**
 * @brief Bayer转YUV420基本转换
 * @date 2022/11/28
 * @author laoyao
 */

#include "bayer2yuv420withresize.h"

#include <helper_cuda.h>

using namespace lystream;
using namespace lynppi::color_conversion;
using namespace lynppi::geometric_transforms;

Bayer2YUV420withResize::Bayer2YUV420withResize(ImgSize size, ImgSize resize) : ImageHandle(resize)
{
    RoiRect roi   = {0, 0, size.width, size.height};
    RoiRect reroi = {0, 0, resize.width, resize.height};

    cudaStreamCreateWithFlags(&stream_, cudaStreamNonBlocking);

    int yuvstep[3] = {resize.width, resize.width / 2, resize.width / 2};

    bayer2rgb_  = shared_ptr<NPPIHelper>(new Bayer8toRGB(size, roi, NPPI_BAYER_RGGB, stream_));
    resize_     = shared_ptr<NPPIHelper>(new ResizeC3(size, resize, stream_));
    rgb2yuv420_ = shared_ptr<NPPIHelper>(new RGBtoYUV420_C3P3(resize, reroi, stream_));
    bayer_      = shared_ptr<ImageMat>(new ImageMat8UC1(size.width, size.height, stream_));
    rgb_[0]     = shared_ptr<ImageMat>(new ImageMat8UC3(size.width, size.height, stream_));
    rgb_[1]     = shared_ptr<ImageMat>(new ImageMat8UC3(resize.width, resize.height, stream_));
    yuv_        = shared_ptr<ImageMat>(new ImageMat8UP3(yuvstep, resize.height, stream_));

    bayer_->setAlgorithm(bayer2rgb_.get());
    rgb_[0]->setAlgorithm(resize_.get());
    rgb_[1]->setAlgorithm(rgb2yuv420_.get());

}

Bayer2YUV420withResize::~Bayer2YUV420withResize()
{
    bayer2rgb_  = nullptr;
    rgb2yuv420_ = nullptr;
    bayer_      = nullptr;
    rgb_[0]     = nullptr;
    rgb_[1]     = nullptr;
    yuv_        = nullptr;

    cudaStreamDestroy(stream_);
}

void Bayer2YUV420withResize::ImageProcessing(void *indata, void **outdata)
{
    if (compute_capability_ >= 7.2f)
        dynamic_cast<ImageMat8UC1 *>(bayer_.get())
            ->setData((unsigned char *)indata, ImageMat::CopyType::ZeroCopyMemory);
    else dynamic_cast<ImageMat8UC1 *>(bayer_.get())->setData((unsigned char *)indata);
    
    bayer_->Processing(rgb_[0].get());
    rgb_[0]->Processing(rgb_[1].get());
    rgb_[1]->Processing(yuv_.get());
    dynamic_cast<ImageMat8UP3 *>(yuv_.get())->getData(img_data_.plane);
    *outdata = &img_data_;
}
