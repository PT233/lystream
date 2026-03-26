/**
 * @brief 可见光图像流测试
 * @date 2022/11/28
 * @author laoyao
 */

#include "utils_log/utils_log.h"
#include "visual_stream.h"

using namespace lyboost::log;
using namespace lystream;

#define CAMERA_BRAND "Pylon"

void OnData(void *data)
{
    auto *current_frame = static_cast<CameraData *>(data);
    Log_Normal << logging::add_value("Tag", CAMERA_BRAND) << "daheng recv, datalen:" << current_frame->datalen
               << ", width:" << current_frame->width << ", height:" << current_frame->height << endl;
}

int main()
{
    Stream *stream = new VisualStream(CAMERA_BRAND, 0);

    stream->DataTransfer(OnData);
    stream->StartStream();

    VisualStream::CameraPara camera_para   = {};
    camera_para                            = dynamic_cast<VisualStream *>(stream)->ReadCameraPara();
    //    camera_para.setting.exposure           = 100;
    camera_para.setting.exposure_auto      = Exposure::EXPOSURE_CONTINUOUS;
    camera_para.setting.exposure_limit.min = 105;
    camera_para.setting.exposure_limit.max = 1420;

    //    camera_para.setting.gain           = 0.0f;
    camera_para.setting.gain_mode      = Gain::GAIN_CONTINUOUS;
    camera_para.setting.gain_limit.min = 0.0f;
    camera_para.setting.gain_limit.max = 12.0f;

    camera_para.setting.brightness     = 0.3f;
    camera_para.setting.fuctionprofile = AutoFunctionProfile::MinimizeExposureTime;

    dynamic_cast<VisualStream *>(stream)->WriteCameraPara(camera_para);

    VisualStream::CameraPara camera_para2 = {};
    camera_para2                          = dynamic_cast<VisualStream *>(stream)->ReadCameraPara();

    printf("the exposure_auto is %d\n", static_cast<int>(camera_para2.setting.exposure_auto));
    printf("the exposure limit min is %d\n", camera_para2.setting.exposure_limit.min);
    printf("the exposure limit max is %d\n", camera_para2.setting.exposure_limit.max);

    printf("the gain_auto is %d\n", static_cast<int>(camera_para2.setting.gain_mode));
    printf("the gain limit min is %f\n", camera_para2.setting.gain_limit.min);
    printf("the gain limit max is %f\n", camera_para2.setting.gain_limit.max);

    printf("the brightness is %f\n", camera_para2.setting.brightness);
    printf("the fuctionprofile is %d\n", static_cast<int>(camera_para2.setting.fuctionprofile));

    std::this_thread::sleep_for(std::chrono::seconds(3));

    //    stream->StopStream();
    //
    //    std::this_thread::sleep_for(std::chrono::seconds(3));
    //    stream->StartStream();
    //
    //    std::this_thread::sleep_for(std::chrono::seconds(10));
    //    stream->StopStream();

    delete stream;
}
