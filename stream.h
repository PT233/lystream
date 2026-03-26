/**
 * @brief 流基本抽象接口
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include "stream_type.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <vector>

namespace lystream
{
    /**
     * @brief 流基类，所有的流均继承于此，描述了流一般性操作和结构
     */
    class Stream
    {
      public:

        /**
         * @struct StreamPara
         * @brief 流属性结构
         *
         * @details 这应该是所有流的共同属性者状态等
         * @todo 此结构应该更丰富，更加具体的描述stream信息
         */
        typedef struct StreamPara {
            std::string name = u8"Stream"; ///< 流名字，仅调试用
            StreamSize size;               ///< 流数据大小，每种流可能大小不一样
            unsigned int ref_count = 0;    ///< 流引用计数
        } StreamPara;

        /**
         * @brief 构造函数
         *
         */
        Stream();

        /**
         * @brief 析构函数
         */
        virtual ~Stream();

        /**
         * @brief 启动流业务
         *
         * @details 此函数由子类具体流实现，标识相关流业务线启动
         */
        virtual void StartStream() = 0;

        /**
         * @brief 停止流业务
         *
         * @details 此函数由子类具体流实现，标识相关流业务线停止
         */
        virtual void StopStream() = 0;

        /**
         * @brief 设置流名字
         *
         * @details 此名字仅供调试专用
         */
        void setName(const std::string &name);

        /**
         * @brief 获取流处理状态
         *
         * @return 流正在运行返回true，反之返回false
         */
        [[maybe_unused]] [[nodiscard]] bool getStreamStatus();

        /**
         * @brief 获取流属性
         *
         * @return 返回对应流的StreamPara结构
         */
        [[nodiscard]] StreamPara getStreamPara();

        /**
         * @brief 注册后端流数据输出回调函数
         *
         * @details 业务流各有自身的功能，每帧流处理完后，需要把数据传递给后端，
         * 由此函数注册后端流回调接口，将处理好的数据通过注册函数向下传递
         */
        void DataTransfer(const std::function<void(void *data)> &ondata);

      protected:

        static constexpr unsigned short max_num_ = 10; ///< 预分配最大数量，仅仅为了输出处理不加入锁机制

        StreamPara stream_para_ {};                                      ///< 流属性
        std::atomic<bool> isrun_ {false};                                ///< 流处理状态标识
        std::vector<std::function<void(void *data)>> ondata_list_ {};    ///< 流数据输出回调数组
        [[maybe_unused]] inline static float compute_capability_ = 0.0f; ///< GPU计算能力
    };
} // namespace lystream