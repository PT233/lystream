/**
 * @brief 可见光相机stream接口
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include "camera.h"
#include "camera_utils.h"
#include "stream.h"

#include <string>

namespace lystream
{
    using namespace lycamera;

    /**
     * @brief 可将光相机数据流，总是前端流
     */
    class VisualStream : public Stream
    {
      public:

        /**
         * @struct CameraParaSetting
         *
         * @brief 可将光相机设置相关参数, 此接口描述了常用的相机参数，需要和lycamera库适配
         */
        typedef struct CameraParaSetting {
            bool enable;                     ///< 相机参数写使能，决定相关的参数是否写到相机
            CameraRoi roi;                   ///< 相机ROI参数
            Mirror mirror[2];                ///< 镜像反转使能，[0]表示水平镜像；[1]表示垂直镜像
            float exposure;                  ///< 曝光时间
            Exposure exposure_auto;          ///< 自动曝光
            ExposureLimit exposure_limit;    ///< 自动曝光限制
            float gain;                      ///< 增益
            Gain gain_mode;                  ///< 增益模式
            GainLimit gain_limit;            ///< 增益限制
            Trigger trigger_mode;            ///< 触发模式
            TriggerSource trigger_source;    ///< 触发源
            WhiteBalance white_balance_auto; ///< 自动白平衡
            float brightness;
            AutoFunctionProfile fuctionprofile;
            UserSelect user_select;      ///< 用户配置选择
            UserSelect user_set_default; ///< 设置默认用户配置
            bool user_set_save = false;  ///< 保存参数到相机
        } CameraParaSetting;

        /**
         * @struct CameraPara
         *
         * @brief 相机参数结构
         */
        typedef struct CameraPara {
            std::string vendor_name;      ///< 厂商名字
            std::string model_name;       ///< 相机型号
            std::string firmware_version; ///< 相机固件版本
            std::string serial_number;    ///< 相机序列号
            float temperature;            ///< 相机温度
            CameraColorType camera_type;  ///< 相机类型, 目前有彩色和黑白之分
            unsigned int index;           ///< 相机索引

            CameraParaSetting setting; ///< 相机参数设置
        } CameraPara;

        /**
         * @brief 构造函数
         * @param[in] brand 相机品牌，这里只能是\n
         *     - DaHeng : 大恒系列相机\n
         *     - HIK : 海康系列相机\n
         *     - IKapC : 埃科系列相机\n
         * @param[in] index 相机索引，一般设置为0
         */
        explicit VisualStream(std::string brand, unsigned int index);

        /**
         * @brief 析构函数
         */
        ~VisualStream() override;

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

        /**
         * @brief 把参数写到相机配置里面
         *
         * @param[in] para 相机参数结构
         */
        [[maybe_unused]] void WriteCameraPara(CameraPara &para);

        /**
         * @brief 读取相机参数
         *
         * @return 返回读取的相机参数结构
         */
        CameraPara ReadCameraPara();

      private:

        /**
         * @brief 将相机参数写到配置文件里面
         *
         * @details 默认配置文件路径：/opt/lystream/config/visual_stream0.yaml，如果存在多可见光相机时，index会自动累加
         */
        void WriteCameraParaFromFile();

        /**
         * @brief 相机图像回调函数
         *
         * @details 可见光相机采集的图像数据由此接口回调提供
         * @param[in] frame 相机图像数据结构
         */
        virtual void CameraOnFrame(void *frame);

        shared_ptr<CameraDevice> device_ = nullptr; ///< 相机操作句柄
        CameraPara device_para_          = {};      ///< 相机参数结构
        bool first_run_                  = true;    ///< 第一次调用标识
    };
} // namespace lystream
