/**
 * @brief 流框架类型定义
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include <chrono>

namespace lystream
{
    /**
     * 流数据大小，流数据基础类型
     */
    typedef struct StreamSize {
        unsigned int width;  ///< 宽度
        unsigned int height; ///< 高度
    } StreamSize;

    /**
     * @brief 处理策略
     */
    enum class PolicyMode : char
    {
        ExecutePerFrame = 0, ///< 每帧执行
        ExecuteInterval,     ///< 间隔执行
    };

    /**
     * 流处理时间点控制
     */
    typedef struct StreamTimeTick {
        unsigned int clock_tick = 0;
        std::chrono::steady_clock::time_point clock_point;
        unsigned long time_count = 0;
    } StreamTimeTick;

} // namespace lystream