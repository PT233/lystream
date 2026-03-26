/**
 * @brief 流控制抽象接口
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include "stream.h"

namespace lystream
{
    /**
     * @brief 流装饰抽象类，描述了装饰具体类的一般方法
     */
    class StreamControl : public Stream
    {
      public:

        /**
         * @brief 构造函数
         *
         * @param[in] stream 流对象指针
         * @param[in] mode 执行模式，是每帧执行还是间隔执行\n
         *     - 每帧执行：对前端的数据每一帧都要处理，都要输出，即前端帧率=后端帧率\n
         *     - 间隔执行：对前端的数据，按时间比例抽帧，不是前端的每一帧都要处理，即前端帧率!=后端帧率\n
         * @param[in] fps 如果mode指定为间隔执行，需要指定fps，也为后端帧率
         *
         * @note 参数中fps是可以超过前端fps，但是这样做没意义。
         */
        explicit StreamControl(Stream *stream, PolicyMode mode = PolicyMode::ExecutePerFrame, unsigned int fps = 0);

        /**
         * @brief 析构函数
         */
        ~StreamControl() override;

        /**
         * @brief 启动流，多路复用接口
         *
         * @details 此接口一般会显式调用，或者被其他后端流隐士调用，但并不是每一次调用都会执行启动流操作\n
         * 当首次调用时，会将DataTransferCall()注册到本实例（本实例作为后端流）中来接收前端流数据源,并启动前端流处理任务，此时DataTransferCall()会持续回调得到前端流数据
         *
         * @note 流拥有引用计数的概念，StartStream()自动+1，StopStream()自动-1
         */
        void StartStream() override;

        /**
         * @brief 停止流
         *
         * @details 此接口一般会显式调用，或者被其他后端隐士调用，每次会执行一次前端流的StopStream()
         */
        void StopStream() override;

      protected:

        /**
         * @brief 前端流数据接收回调函数
         *
         * @details StartStream()后，可通过DataTransferCall()拿到前端流数据
         * @param data 前端流处理完后的数据，具体结构视前端流而定
         */
        virtual void DataTransferCall(void *data) = 0;

        StreamTimeTick time_tick_; ///< 时间控制信息，当处于间隔执行时才启用
        PolicyMode execute_mode_;  ///< 执行策略

      private:

        Stream *stream_;        ///< 前端流
        bool first_run_ = true; ///< 第一次调用标识
    };
} // namespace lystream