/**
 * @brief 算法流
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include "algorithm/algorithm.h"
#include "stream_control.h"
#include "utils/mem/stream_data_deque.h"

#include <boost/asio.hpp>
#include <camera_utils.h>
#include <mutex>
#include <thread>

namespace lystream
{
    /**
     * @brief 算法流类
     */
    class AlgoStream : public StreamControl
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
        explicit AlgoStream(Stream *stream, PolicyMode mode = PolicyMode::ExecutePerFrame, unsigned int fps = 0);

        /**
         * @brief 析构函数
         */
        ~AlgoStream() override;

        /**
         * @brief 启动流业务
         *
         * @details
         * 启动父类StartStream()，并初始化自身资源Initialization()
         */
        void StartStream() override;

        /**
         * @brief 停止流
         *
         * @details 此接口一般会显式调用，或者被其他后端隐士调用，每次会执行一次前端流的StopStream()
         */
        void StopStream() override;

        /**
         * @brief 增加算法处理算子
         * @details 调用StartStream()之前使用此接口，注册相关图像处理算子，初始化时根据注册的算子情况初始化资源
         * @param[in] handle 算法处理算子句柄
         */
        void addAlgoThread(Algorithm *handle);

      private:

        /**
         * @brief 初始化算法流
         * @details 这里会被StartStream()调用，根据注册的算法处理算子情况，初始化零拷贝资源，并适当开辟若干个处理线程
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
         * @brief 算法处理线程
         * @details 此函数会一直进行算法处理，并把算法处理结果传送给后端流
         * @param[in] index 线程索引
         */
        void Handle(unsigned int index);

        /**
         * @brief 算法处理结构
         */
        struct StreamBuffer {
            unsigned char *rawdata;      ///< 原始数据指针
            unsigned char *aligndata;    ///< 内存对其后的数据指针
            unsigned char *d_aligndata;  ///< 算法结果原始指针
            StreamDataDeque *data_queue; ///< 数据缓存
        };

        /**
         * @brief 算法流
         */
        struct StreamCtrl {
            std::mutex mtx;       ///< 互斥锁
            bool isready = false; ///< 算法流准备标识

            /**
             * @brief 算法流中的线程控制
             */
            struct ThreadCtrl {
                std::vector<std::thread> handle_th; ///< 算法线程句柄
                unsigned int handle_sum = 0;        ///< 处理线程统计
                unsigned int loop_count = 0;        ///< 循环处理索引
            } thread_ctrl;
        } stream_ctrl_;

        /**
         * @brief 算法流处理数据
         */
        struct StreamData {
            std::vector<Algorithm *> handle;  ///< 算法线程句柄
            std::vector<StreamBuffer> buffer; ///< 算法数据缓冲
        } stream_data_;
    };

} // namespace lystream