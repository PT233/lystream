/**
 * @brief 公共接口
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#define ZEROCOPY_ALIGNMENT 4096
#define ALIGN_UP(x)        (((size_t)x + (ZEROCOPY_ALIGNMENT - 1)) & (~(ZEROCOPY_ALIGNMENT - 1)))