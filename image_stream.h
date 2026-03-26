/**
 * @brief 图像流
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include "handle/image/image_handle.h"
#include "stream_control.h"
#include "utils/mem/stream_data_deque.h"

#include <camera_utils.h>
#include <mutex>
#include <thread>

namespace lystream
{
    namespace imagestream
    {
        /**
         * @brief 图像处理数据结构
         */
        typedef struct StreamBuffer {
            unsigned char *rawdata;      ///< 原始数据指针
            unsigned char *aligndata;    ///< 内存对其后的数据指针
            StreamDataDeque *data_queue; ///< 数据缓存
        } StreamBuffer;

        /**
         * 图像处理流控制
         */
        typedef struct StreamCtrl {
            std::mutex mtx;                ///< 互斥锁
            bool isready = false;          ///< 初始化成功标识
            vector<std::thread> handle_th; ///< 处理线程句柄
            unsigned handle_sum     = 0;   ///< 处理线程统计
            unsigned int loop_count = 0;   ///< 循环处理索引
        } StreamCtrl;

        /**
         * 图像处理流数据
         */
        typedef struct StreamData {
            vector<ImageHandle *> handle; ///< 图像处理句柄
            vector<StreamBuffer> buffer;  ///< 图像处理缓冲
        } StreamData;

    } // namespace imagestream

    /**
     * @brief 图像处理流
     */
    class ImageStream : public StreamControl
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
        explicit ImageStream(Stream *stream, PolicyMode mode = PolicyMode::ExecutePerFrame, unsigned int fps = 0);

        /**
         * @brief 析构函数
         */
        ~ImageStream() override;

        /**
         * @brief 启动流业务
         *
         * @details
         * 启动前端流的StartStream()，并执行自身初始化Initialization()
         */
        void StartStream() override;

        /**
         * @brief 停止流
         *
         * @details 此接口一般会显式调用，或者被其他后端隐士调用，每次会执行一次前端流的StopStream()
         */
        void StopStream() override;

        /**
         * @brief 增加图像处理算子
         * @details 调用StartStream()之前使用此接口，注册相关图像处理算子，初始化时根据注册的算子情况初始化资源
         * @param[in] handle 图像处理算子句柄
         */
        void addHandle(ImageHandle *handle);

      private:

        /**
         * @brief 图像处理初始化
         *
         * @details 这里会被StartStream()调用，根据注册的图像处理算子情况，初始化零拷贝资源，并适当开辟若干个处理线程
         */
        void Initialization();

        /**
         * @brief 前端流数据接收回调函数
         *
         * @details StartStream()后，可通过DataTransferCall()拿到前端流数据
         * @param data 前端流处理完后的数据，具体结构视前端流而定
         */
        void DataTransferCall(void *data) override;

        /**
         * @brief 图像处理线程
         * @details 此函数会一直进行图像处理，并把图像处理结果传送给后端流
         * @param[in] index 处理线程索引
         */
        void DataHandle(unsigned int index);

        imagestream::StreamCtrl stream_ctrl_;
        imagestream::StreamData stream_data_;
    };

} // namespace lystream