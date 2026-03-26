/**
 * @brief 虚拟相机stream接口
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include "camera_utils.h"
#include "stream.h"

#include <string>
#include <thread>

namespace lystream
{
    using namespace lycamera;

    namespace virtualstream
    {
        typedef struct StreamFrameCtrl {
            unsigned int fps = 0;     ///< 相机图像帧率
            StreamTimeTick time_tick; ///< 时间控制信息
        } StreamFrameCtrl;

    } // namespace virtualstream

    /**
     * @brief 虚拟相机流类，仅用于调试用，测试任何场景下的相机流
     */
    class VirtualStream : public Stream
    {
      public:

        /**
         * @brief 构造函数
         */
        explicit VirtualStream();

        /**
         * @brief 析构函数
         */
        ~VirtualStream() override;

        /**
         * @brief 设置相机图像和帧率
         *
         * @param[in] data 相机图像结构，相机出的图像由这里提供
         * @param[in] fps 相机图像帧率，此项可测试任意帧率情况
         */
        void setCameraInfo(CameraData data, unsigned int fps);

        /**
         * @brief 相机图像生产者接口，按照指定帧率生产图像
         *
         * @param param 相机图像数据结构
         */
        void OnRecvFrame(void *param);

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
         * @brief 停止业务流
         *
         * @details 此函数最终停止相机流，并析构相关资源
         */
        void StopStream() override;

      private:

        /**
         * @brief 相机图像回调函数
         *
         * @details 可见光相机采集的图像数据由此接口回调提供
         * @param[in] frame 相机图像数据结构
         */
        void CameraOnFrame(void *frame);

        virtualstream::StreamFrameCtrl frame_ctrl_; ///< 相机帧率控制
        std::thread gather_th_;                     ///< 相机图像生产线程
        CameraData frame_data_ {};                  ///< 相机图像数据结构
        bool isend_      = true;                    ///< 生产者线程控制标识
        bool first_call_ = true;                    ///< 第一次调用标识
    };
} // namespace lystream