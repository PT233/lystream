/**
 * @brief 虚拟图像流测试
 * @date 2022/11/28
 * @author laoyao
 */

#include "utils_log/utils_log.h"
#include "virtual_stream.h"

#include <unistd.h>

using namespace lyboost::log;
using namespace lystream;

void OnData(void *data)
{
    auto *current_frame = static_cast<CameraData *>(data);
    Log_Normal << logging::add_value("Tag", "Virtual") << "virtual recv, datalen:" << current_frame->datalen
               << ", width:" << current_frame->width << ", height:" << current_frame->height;
}

int main()
{
    int width = 2048, height = 1536;

    CameraData raw_data;
    raw_data.data     = new unsigned char[width * height];
    raw_data.width    = width;
    raw_data.height   = height;
    raw_data.datalen  = width * height;
    raw_data.img_type = CameraColorType::Color_Camera;

    std::ifstream data_file("/root/lycamera/virtual_camera/test/bayerrg8_2048_1536.raw");

    data_file.read((char *)raw_data.data, width * height);
    data_file.close();

    Stream *stream = new VirtualStream();

    dynamic_cast<VirtualStream *>(stream)->setCameraInfo(raw_data, 100);
    stream->DataTransfer(OnData);
    stream->StartStream();

    sleep(3);

    stream->StopStream();

    delete stream;
}