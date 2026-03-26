/**
 * @brief 图像处理高层接口
 * @date 2022/11/28
 * @author laoyao
 */

#include "image_handle.h"

#include <cuda_runtime.h>
#include <helper_cuda.h>

using namespace lystream;

ImageHandle::ImageHandle(ImgSize size)
{
    static bool first_call = true;
    if (first_call) {
        cudaDeviceProp prop {};
        checkCudaErrors(cudaGetDeviceProperties(&prop, 0));
        compute_capability_ = static_cast<float>(prop.major) + static_cast<float>(prop.minor) / 10.0f;

        first_call = false;
    }

    img_data_.size = size;
}

ImageHandle::~ImageHandle() = default;