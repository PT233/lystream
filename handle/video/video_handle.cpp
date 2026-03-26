/**
 * @brief 图像处理高层接口
 * @date 2022/11/28
 * @author laoyao
 */

#include "video_handle.h"

using namespace lystream;

VideoHandle::VideoHandle(CodeSize size)
{
    img_data_.size = size;
}

VideoHandle::~VideoHandle() = default;