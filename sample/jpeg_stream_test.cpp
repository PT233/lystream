/**
 * @brief Jpeg流测试
 * @date 2022/11/28
 * @author laoyao
 */

#include "handle/image/bayer2yuv420.h"
#include "image_stream.h"
#include "jpeg_stream.h"
#include "utils_log/utils_log.h"
#include "visual_stream.h"

using namespace lyboost::log;
using namespace lystream;

#define CAMERA_BRAND "Pylon"

void CameraOnData(void *data)
{
    static time_t lInit;
    static uint32_t ui32FrameCount = 0;
    time_t lEnd;

    if (!ui32FrameCount) { time(&lInit); }

    ui32FrameCount++;
    time(&lEnd);
    if (lEnd - lInit >= 1) {
        printf("camera frame rate is %d\n", ui32FrameCount);
        ui32FrameCount = 0;
    }
}

void OnData(void *data)
{
    //    auto *current_data = (JpegStream::JpegData *)data;
    //
    //    static int index   = 0;
    //    char filename[125] = {0};
    //    sprintf(filename, "main%d.yuv", index++);
    //    auto *out_file = new ofstream(filename);
    //
    //    for (int plane = 0; plane < 3; plane++) {
    //        if (plane == 0) {
    //            out_file->write((char *)current_data->plane[plane], current_data->roi.width *
    //            current_data->roi.height);
    //        }
    //        else {
    //            out_file->write((char *)current_data->plane[plane],
    //                            current_data->roi.width * current_data->roi.height / 4);
    //        }
    //    }
    //
    //    out_file->close();
    //    delete out_file;

    static time_t lInit;
    static uint32_t ui32FrameCount = 0;
    time_t lEnd;

    if (!ui32FrameCount) { time(&lInit); }

    ui32FrameCount++;
    time(&lEnd);
    if (lEnd - lInit >= 1) {
        printf("jpeg encode rate is %d\n", ui32FrameCount);
        ui32FrameCount = 0;
    }
}

int main()
{
    VisualStream::CameraPara camera_para = {};

    Stream *visual_stream = new VisualStream(CAMERA_BRAND, 0);
    visual_stream->setName("visual_stream");
    visual_stream->DataTransfer(CameraOnData);
    visual_stream->StartStream();
    camera_para = dynamic_cast<VisualStream *>(visual_stream)->ReadCameraPara();

    ImgSize size = {static_cast<int>(camera_para.setting.roi.width), static_cast<int>(camera_para.setting.roi.height)};
    StreamControl *bayer2yuv_stream = new ImageStream(visual_stream);
    bayer2yuv_stream->setName("bayer2yuv_stream");
    ImageHandle *bayer2yuv     = new Bayer2YUV420(size);
    StreamControl *jpeg_stream = new JpegStream(bayer2yuv_stream, PolicyMode::ExecuteInterval, 20);
    jpeg_stream->setName("jpeg_stream");
    dynamic_cast<ImageStream *>(bayer2yuv_stream)->addHandle(bayer2yuv);

    jpegstream::JpegPara jpeg_para = JpegStream::getDefaultPara();
    dynamic_cast<JpegStream *>(jpeg_stream)->setJpegPara(jpeg_para);
    jpeg_stream->DataTransfer(OnData);
    jpeg_stream->StartStream();

    std::this_thread::sleep_for(std::chrono::seconds(5000));

    // jpeg_stream->StopStream();
    //
    // std::this_thread::sleep_for(std::chrono::seconds(3));
    //
    // jpeg_stream->StartStream();
    //
    // std::this_thread::sleep_for(std::chrono::seconds(5));
    //
    // jpeg_stream->StopStream();
    // visual_stream->StopStream();

    delete visual_stream;
    delete bayer2yuv_stream;
    delete bayer2yuv;
    delete jpeg_stream;
}