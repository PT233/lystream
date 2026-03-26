/**
 * @brief mono8 旋转
 * @date 2023/12/08
 * @author
 */

#include "rotateC1.h"

#include <thread>

using namespace lystream;
using namespace lynppi::geometric_transforms;

Rotate_C1::Rotate_C1(ImgSize size, int angel) : ImageHandle(size)
{
    RoiRect roi = {0, 0, size.width, size.height};
    cudaStreamCreateWithFlags(&stream_, cudaStreamNonBlocking);

    rotate_convert = shared_ptr<NPPIHelper>(new RotateC1(size, roi, angel, stream_));
    srcMat         = shared_ptr<ImageMat>(new ImageMat8UC1(size.width, size.height, stream_));
    m_rotateSize   = dynamic_cast<RotateC1 *>(rotate_convert.get())->getDstSize();
    dstMat         = shared_ptr<ImageMat>(new ImageMat8UC1(m_rotateSize.width, m_rotateSize.height, stream_));
    srcMat->setAlgorithm(rotate_convert.get());

    for (int i = 1; i < 3; i++) {
        img_data_.plane[i] = new unsigned char[m_rotateSize.width * m_rotateSize.height];
        memset(img_data_.plane[i], 0x80, m_rotateSize.width * m_rotateSize.height);
    }
}

Rotate_C1::~Rotate_C1()
{
    rotate_convert = nullptr;
    srcMat         = nullptr;
    dstMat         = nullptr;
    for (int i = 1; i < 3; i++) { delete[] img_data_.plane[i]; }
    cudaStreamDestroy(stream_);
}

void Rotate_C1::ImageProcessing(void *indata, void **outdata)
{
    if (compute_capability_ >= 7.2f)
        dynamic_cast<ImageMat8UC1 *>(srcMat.get())
            ->setData((unsigned char *)indata, ImageMat::CopyType::ZeroCopyMemory);
    else dynamic_cast<ImageMat8UC1 *>(srcMat.get())->setData((unsigned char *)indata);

    srcMat->Processing(dstMat.get());
    dynamic_cast<ImageMat8UC1 *>(dstMat.get())->getData(img_data_.plane);
    img_data_.size.width  = m_rotateSize.width;
    img_data_.size.height = m_rotateSize.height;

    *outdata = &img_data_;
}