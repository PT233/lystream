/**
 * @brief 相机配置文件测试
 * @date 2022/11/28
 * @author laoyao
 */

#include "utils/parse/parse_config.h"

using namespace lystream;

int main()
{
    lystream::VisualStream::CameraPara para;

#ifndef NDEBUG
    #define CONFIG_PATH "/opt/lystream/config/visual_stream0.yaml"
#else
    #define CONFIG_PATH "/root/lystream/config/visual_stream0.yaml"
#endif
    ReadCameraStreamPara(CONFIG_PATH, para);

    para.setting.enable   = true;
    para.vendor_name      = "1234";
    para.model_name       = "5678";
    para.firmware_version = "9101112";

    para.camera_type = CameraColorType::Color_Camera;

    para.setting.roi.width  = 10;
    para.setting.roi.height = 20;
    para.setting.roi.x      = 100;
    para.setting.roi.y      = 200;

    para.setting.trigger_mode       = Trigger::TRIGGER_ON;
    para.setting.trigger_source     = TriggerSource::TRIGGER_SOURCE_LINE0;
    para.setting.mirror[0]          = Mirror::HORIZONTAL_MIRROR_ON;
    para.setting.mirror[1]          = Mirror::VERTICAL_MIRROR_ON;
    para.setting.exposure_auto      = Exposure::EXPOSURE_CONTINUOUS;
    para.setting.exposure_limit.min = 10.0f;
    para.setting.exposure_limit.max = 12.0f;

    para.setting.gain           = 1.1f;
    para.setting.gain_mode      = Gain::GAIN_CONTINUOUS;
    para.setting.gain_limit.min = 1.2f;
    para.setting.gain_limit.max = 1.3f;

    para.setting.white_balance_auto = WhiteBalance::WHITE_BALANCE_CONTINUOUS;

    WriteCameraStreamPara(CONFIG_PATH, para);

    return 0;
}