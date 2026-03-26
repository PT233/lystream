/**
 * @brief Gray转YUV420转换，带有Resize
 * @date 2022/11/28
 * @author laoyao
 */

#include "gray2yuv420withresize.h"

using namespace lystream;
using namespace lynppi::geometric_transforms;

Gray2YUV420withResize::Gray2YUV420withResize(ImgSize size, ImgSize resize) : ImageHandle(resize)
{
    for (int i = 1; i < 3; i++) {
        img_data_.plane[i] = new unsigned char[resize.width * resize.height];
        memset(img_data_.plane[i], 0x80, resize.width * resize.height);
    }

    cudaStreamCreateWithFlags(&stream_, cudaStreamDefault);

    resize_convert_ = shared_ptr<NPPIHelper>(new ResizeC1(size, resize, stream_));
    gray_           = shared_ptr<ImageMat>(new ImageMat8UC1(size.width, size.height, stream_));
    resize_         = shared_ptr<ImageMat>(new ImageMat8UC1(resize.width, resize.height, stream_));

    gray_->setAlgorithm(resize_convert_.get());
}

Gray2YUV420withResize::~Gray2YUV420withResize()
{
    for (int i = 1; i < 3; i++) { delete[] img_data_.plane[i]; }
}

void Gray2YUV420withResize::ImageProcessing(void *indata, void **outdata)
{
    if (compute_capability_ >= 7.2f)
        dynamic_cast<ImageMat8UC1 *>(gray_.get())->setData((unsigned char *)indata, ImageMat::CopyType::ZeroCopyMemory);
    else dynamic_cast<ImageMat8UC1 *>(gray_.get())->setData((unsigned char *)indata);
    
    gray_->Processing(resize_.get());
    dynamic_cast<ImageMat8UC1 *>(resize_.get())->getData(&img_data_.plane[0]);
    *outdata = &img_data_;
}
