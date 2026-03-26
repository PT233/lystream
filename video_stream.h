/**
 * @brief 视频流
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include "handle/image/image_handle.h"
#include "handle/video/video_handle.h"
#include "nvdia_video_encoder.h"
#include "stream_control.h"

#include <memory>
#include <thread>

namespace lystream
{
    using namespace lycodec;
    using namespace lycodec::lyvideo;

    /**
     * 视频流类，这里描述了视频流相关的一般性操作
     */
    class VideoStream : public StreamControl
    {
      public:

        /**
         * 视频编码参数结构
         */
        typedef struct {
            /**
             * 编码类型，可选择值：H264/H265
             */
            string encoder_type;
            /**
             * 编码帧率
             */
            unsigned int fps;
            /**
             * 码率
             */
            unsigned int bitrate;
            /**
             * 速率模式，可选值：定码率或者编码率
             */
            string ratemode;
            /**
             * profile类型
             */
            string profile;
            /**
             * 编码等级
             */
            string level;
            /**
             * 色域
             */
            string colorspace;
            /**
             * 最大性能使能
             */
            bool max_permode;
            /**
             * 编码输出所有I帧
             */
            bool all_iframe;
            /**
             * I帧间隔，实时流一般10~20
             */
            unsigned int ifrmae_interval;
            /**
             * IDR帧间隔，实时流一般30
             */
            unsigned int idr_interval;
            /**
             * 在IDR帧插入视频流头信息
             */
            bool insert_spspps_idr;
            /**
             * 插入VUI数据结构
             */
            bool insert_vui;
            /**
             * 插入AUD帧
             */
            bool insert_aud;
        } VideoPara;

        /**
         * 视频流数据结构
         */
        typedef struct {
            /**
             * 数据指针
             */
            unsigned char *data;
            /**
             * 数据大小
             */
            size_t datalen;
        } VideoData;

        /**
         * 构造函数
         *
         * @param [in]stream 数据源结点
         * @param [in]size   图像编码大小
         * @param [in]mode   处理过滤模式
         * @param [in]fps    过滤模式帧率，只有在PolicyMode::ExecuteInterval中
         */
        explicit VideoStream(Stream *stream, CodecSize size, PolicyMode mode = PolicyMode::ExecutePerFrame,
                             unsigned int fps = 0);
        /**
         * 析构函数
         */
        ~VideoStream() override;

        /**
         * 启动业务流
         */
        void StartStream() override;
        /**
         * 停止业务流
         */
        void StopStream() override;

        /**
         * 设置视频流参数
         *
         * @param [in]para 视频流参数结构
         */
        void setVideoPara(VideoPara para);

        /**
         * 获取默认视频流参数结构
         *
         * @param [in]encode_type 编码类型，可选值：H264、H265
         * @return 返回视频流参数结构
         */
        VideoPara getDefaultPara(const string &encode_type) const;

        /**
         * 增加一个处理算子
         *
         * @param [in]handle 视频处理算子
         */
        void addHandle(VideoHandle *handle);

      private:

        /**
         * 初始化视频流资源
         */
        void Initialization();

        /**
         * 视频流数据输入回调接口
         *
         * @param [in]data 输入数据
         */
        void DataTransferCall(void *data) override;

        /**
         * 视频流参数设置
         *
         * @param [in]encode 视频编码器操作句柄
         */
        void VideoSetting(NvdiaVideoEncoder *encode);

        /**
         * 视频流编码处理
         *
         * @param [in]encode 视频编码器操作句柄
         */
        void Handle(NvdiaVideoEncoder *encode);

        /**
         * 获取视频自适应码率
         *
         * 根据视频类型以及分辨率自动计算出合适的视频码率
         *
         * @param [in]pixlen    图像大小
         * @param [in]videotype 视频类型
         * @return 返回自适应的视频码率
         */
        static unsigned int getVideoRate(unsigned int pixlen, const std::string &videotype);

        /**
         * 视频流控制结构
         */
        struct {
            /**
             * 初始化互斥
             */
            std::mutex mtx;
            /**
             * 初始化成功标识
             */
            bool isready = false;
            /**
             * 视频流处理线程句柄
             */
            std::thread handle_th;
        } stream_ctrl_;

        /**
         * 视频流参数结构
         */
        VideoPara video_para_;
        /**
         * 视频流大小
         */
        CodeSize size_ {};
        /**
         * 视频处理操作句柄
         */
        VideoHandle *handle_ = nullptr;

        /**
         * 视频编码器操作句柄
         */
        shared_ptr<NvdiaVideoEncoder> encode_ = nullptr;

        std::mutex mtx_;


    };
} // namespace lystream
