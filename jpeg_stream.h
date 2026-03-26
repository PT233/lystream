/**
 * @brief Jpeg图像流
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include "nvdia_jpeg_encoder.h"
#include "stream_control.h"

#include <thread>

namespace lystream
{
    using namespace lycodec;
    using namespace lycodec::lyjpeg;

    namespace jpegstream
    {
        /**
         * @brief 线程最大数量
         */
        constexpr unsigned int thread_num = 2;

        /**
         * @brief Jpeg图像参数结构
         */
        typedef struct {
            unsigned int level; ///< jpeg编码等级
            StreamSize scale;   ///< jpeg编码缩放

            /**
             * @brief Roi参数结构
             */
            struct Roi {
                unsigned int x;      ///< x坐标
                unsigned int y;      ///< y坐标
                unsigned int width;  ///< 宽度
                unsigned int height; ///< 高度
            } roi;
        } JpegPara;

        /**
         * @brief Jpeg数据结构
         */
        typedef struct {
            unsigned char *data; ///< 数据指针
            size_t datalen;      ///< 数据大小
        } JpegData;

        /**
         * @brief Jpeg流控制结构
         */
        typedef struct StreamCtrl {
            std::mutex mtx;       ///< 互斥锁
            bool isready = false; ///< Jpeg流准备标识

            std::thread handle_th[thread_num]; ///< 处理线程句柄
            bool isexchange = true;            ///< 处理线程选择轮询

            StreamTimeTick jpeg_clock; ///< Jpeg流时钟
        } StreamCtrl;

    } // namespace jpegstream

    /**
     * @brief Jpeg图像流类
     */
    class JpegStream : public StreamControl
    {
      public:

        /**
         * @brief 构造函数
         *
         * @param[in] stream 流对象指针
         * @param[in] mode 执行模式，是每帧执行还是间隔执行
         * @param[in] fps 如果是间隔执行，需要指定fps
         */
        explicit JpegStream(Stream *stream, PolicyMode mode = PolicyMode::ExecutePerFrame, unsigned int fps = 0,
                            StreamSize size = {0, 0});

        /**
         * 析构函数
         */
        ~JpegStream() override;

        /**
         * @brief 启动流业务
         *
         * @details 此函数第一次调用时，初始化红外相机资源
         */
        void StartStream() override;

        /**
         * @brief 停止流业务
         *
         * @details 此函数最终停止stream_流处理
         */
        void StopStream() override;

        /**
         * @brief 设置Jpeg编码参数
         * @param[in] para Jpeg编码参数
         */
        void setJpegPara(jpegstream::JpegPara para);

        /**
         * @brief 获取默认Jpeg编码参数
         * @return 返回默认Jpeg编码参数
         */
        static jpegstream::JpegPara getDefaultPara();

      private:

        /**
         * @brief 线程最大数量
         */
#define THREAD_NUM (2)

        /**
         * Jpeg流初始化
         */
        void Initialization();

        /**
         * @brief 输入数据接收回调函数
         *
         * @details StartStream()后，可通过DataTransferCall()拿到输入数据
         * @param data 输入数据，具体结构视具体情况而定
         */
        void DataTransferCall(void *data) override;

        /**
         * @brief 图像处理线程
         * @param[in] index 线程索引
         */
        void Handle(NvdiaJpegEncoder *encode);

        /**
         * @brief Jpeg流参数设定
         * @param encode Jpeg编码句柄
         */
        void JpegSetting(NvdiaJpegEncoder *encode) const;

        jpegstream::StreamCtrl stream_ctrl_ {};
        jpegstream::JpegPara jpeg_para_ {}; ///< Jpeg流编码参数

        shared_ptr<NvdiaJpegEncoder> encode_[THREAD_NUM] = {nullptr}; ///< Jpeg流编码句柄
        StreamSize size_;

        std::mutex mtx_;
    };
} // namespace lystream
