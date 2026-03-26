/**
 * @brief Gray转YUV420
 * @date 2022/11/28
 * @author laoyao
 */

#include "gray2yuv420p.h"

#include <cstring>

using namespace lystream;

Gray2YUV420::Gray2YUV420(ImgSize size) : ImageHandle(size)
{
    for (int i = 1; i < 3; i++) {
        img_data_.plane[i] = new unsigned char[size.width * size.height];
        memset(img_data_.plane[i], 0x80, size.width * size.height);
    }
}

Gray2YUV420::~Gray2YUV420()
{
    for (int i = 1; i < 3; i++) { delete[] img_data_.plane[i]; }
}

void Gray2YUV420::ImageProcessing(void *indata, void **outdata)
{
    img_data_.plane[0] = static_cast<unsigned char *>(indata);
    *outdata           = &img_data_;
}