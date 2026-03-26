/**
 * @brief 可见光图像流测试
 * @date 2022/11/28
 * @author laoyao
 */

#include "handle/image/gray2yuv420p.h"
#include "handle/image/rotateC1.h"
#include "image_stream.h"
#include "jpeg_stream.h"
#include "utils_log/utils_log.h"
#include "visual_stream.h"

using namespace lyboost::log;
using namespace lystream;

#define CAMERA_BRAND "HIK"

void OnData(void *data)
{
    auto *current_frame = static_cast<CameraData *>(data);
    Log_Normal << logging::add_value("Tag", CAMERA_BRAND) << CAMERA_BRAND << " recv, datalen:" << current_frame->datalen
               << ", width:" << current_frame->width << ", height:" << current_frame->height << endl;
}

void handleJpegStream(void *data)
{
    auto *jpgData           = (lystream::jpegstream::JpegData *)data;
    static unsigned int num = 0;
    num++;

    std::cout << "jpg num: " << num << std::endl;
}

int main()
{
    Stream *stream = new VisualStream(CAMERA_BRAND, 0);

    //    stream->DataTransfer(OnData);
    stream->StartStream();

    lystream::VisualStream::CameraPara m_cameraParam = dynamic_cast<lystream::VisualStream *>(stream)->ReadCameraPara();
    lynppi::ImgSize size
        = {static_cast<int>(m_cameraParam.setting.roi.width), static_cast<int>(m_cameraParam.setting.roi.height)};

    int angel                             = 90;
    lystream::ImageHandle *rotateHandle   = new lystream::Rotate_C1(size, angel);
    lynppi::ImgSize rotateSize            = {dynamic_cast<lystream::Rotate_C1 *>(rotateHandle)->m_rotateSize.width,
                                             dynamic_cast<lystream::Rotate_C1 *>(rotateHandle)->m_rotateSize.height};
    lystream::StreamControl *rotateStream = new lystream::ImageStream(stream);
    rotateStream->setName("rotate stream");
    dynamic_cast<lystream::ImageStream *>(rotateStream)->addHandle(rotateHandle);

    lystream::StreamSize jpgSize
        = {static_cast<unsigned int>(rotateSize.width), static_cast<unsigned int>(rotateSize.height)};
    lystream::StreamControl *jpegStream
        = new lystream::JpegStream(rotateStream, lystream::PolicyMode::ExecutePerFrame, 100, jpgSize);
    jpegStream->setName("jpeg stream");
    jpegStream->DataTransfer([](auto &&PH1) { handleJpegStream(std::forward<decltype(PH1)>(PH1)); });

    jpegStream->StartStream();

    std::this_thread::sleep_for(std::chrono::seconds(30));

    stream->StopStream();
    jpegStream->StopStream();

    delete stream;
}