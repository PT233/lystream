/**
 * @brief 红外相机stream接口
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include "camera.h"
#include "image_stream.h"
#include "lynppi.h"
#include "stream.h"
#include "utils/ffmpeg/ffmpeg_video_decoder.h"

#include <mutex>
#include <string>

namespace lystream
{

    using namespace lynppi;
    using namespace lycamera;

    /**
     * 红外相机流类
     */
    class InfraredStream : public Stream
    {
      public:

        /**
         * 红外相机参数类
         */
        typedef struct CameraPara {
            std::string type; ///< 相机类型
            std::string ip;   ///< 相机IP
            StreamSize size;
        } CameraPara;

        /**
         * @brief 构造函数
         * @param[in] para 红外相机参数
         */
        InfraredStream(std::string_view type, std::string_view ip);
        /**
         * 析构函数
         */
        ~InfraredStream() override;

        /**
         * @brief 启动流业务
         *
         * @details 此函数第一次调用时，初始化红外相机资源
         */
        void StartStream() override;

        /**
         * @brief 停止业务流
         * @details 此函数最终停止相机流，并析构相关资源
         */
        void StopStream() override;

        /**
         * @brief 获取最高温度
         *
         * @param[in] rect 计算最高温度的区域
         * @param[out] x   计算结果，最高温度的x坐标
         * @param[out] y   计算结果，最高温度的y坐标
         * @return 返回最高温度的值
         */
        [[maybe_unused]] [[nodiscard]] int getHighestTemperature(RoiRect rect, unsigned short &x, unsigned short &y);

        /**
         * @brief 读取相机参数
         *
         * @return 返回读取的相机参数结构
         */
        CameraPara ReadCameraPara();

      private:

        /**
         * @brief 空
         */
        void WriteCameraPara();

        /**
         * @brief 相机图像回调函数
         *
         * @details 红外相机采集的图像数据由此接口回调提供
         * @param[in] frame 相机图像数据结构
         */
        void OnFrame(void *frame);

        /**
         * @brief 温度数据回调函数
         * @param[in] data 温度数据矩阵指针
         * @param[in] datalen 温度数据大小
         */
        void TempCallback(char *data, long datalen);

        /**
         * @brief 红外数据处理
         */
        //        void VideoHandle();

        void TempMatrixHandle(unsigned int index);

        static constexpr unsigned int handle_sum_ = 2;
        /**
         * 红外流控制
         */
        struct StreamCtrl {
            std::thread handle_th[3]; ///< 相机操作句柄
            unsigned int loop_count = 0;
        } stream_ctrl_;

        /**
         * 红外流数据
         */
        struct StreamData {
            CameraPara device_para = {};    ///< 红外相机参数
            std::atomic<InfraredTemp> temp; ///< 温度数据
            std::atomic<RoiRect> rect;
            ImageHandle::ImgData img_data; ///< 红外图像数据
            StreamDataDeque *temp_queue[handle_sum_];
        } stream_data_ {};

        shared_ptr<CameraDevice> device_ = nullptr; ///< 红外相机句柄
        FFmpegVideoDecoder decoder_;
    };
} // namespace lystream
