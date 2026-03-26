/**
 * @brief 图像流测试
 * @date 2022/11/28
 * @author laoyao
 */

#include "handle/image/bayer2yuv420.h"
#include "handle/image/bayer2yuv420withresize.h"
#include "image_stream.h"
#include "utils_log/utils_log.h"
#include "visual_stream.h"

#include <unistd.h>

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

void OnData_sub(void *data)
{
    //    auto *current_data = (ImageHandle::ImgData *)data;
    //
    //    static int index   = 0;
    //    char filename[125] = {0};
    //    sprintf(filename, "sub%d.yuv", index++);
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

    //    out_file->close();
    //    delete out_file;

    static time_t lInit;
    static uint32_t ui32FrameCount = 0;
    time_t lEnd;

    if (!ui32FrameCount) { time(&lInit); }

    ui32FrameCount++;
    time(&lEnd);
    if (lEnd - lInit >= 1) {
        printf("Image handle sub rate is %d\n", ui32FrameCount);
        ui32FrameCount = 0;
    }
}

void OnData_main(void *data)
{
    auto *current_data = (ImageHandle::ImgData *)data;
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
        printf("Image handle main rate is %d\n", ui32FrameCount);
        ui32FrameCount = 0;
    }
}

int main()
{
    constexpr int thread_num = 2;

    VisualStream::CameraPara camera_para = {};

    Stream *visual_stream = new VisualStream(CAMERA_BRAND, 0);
    visual_stream->setName("visual_stream");
    visual_stream->DataTransfer(CameraOnData);
    visual_stream->StartStream();
    camera_para  = dynamic_cast<VisualStream *>(visual_stream)->ReadCameraPara();
    ImgSize size = {static_cast<int>(camera_para.setting.roi.width), static_cast<int>(camera_para.setting.roi.height)};

    StreamControl *bayer2yuv_stream           = new ImageStream(visual_stream, PolicyMode::ExecuteInterval, 25);
    StreamControl *bayer2yuvwithresize_stream = new ImageStream(visual_stream, PolicyMode::ExecuteInterval, 30);
    bayer2yuv_stream->setName("bayer2yuv_stream");
    bayer2yuvwithresize_stream->setName("bayer2yuvwithresize_stream");

    ImageHandle *bayer2yuv[thread_num];
    for (auto &convert : bayer2yuv) {
        convert = new Bayer2YUV420(size);
        dynamic_cast<ImageStream *>(bayer2yuv_stream)->addHandle(convert);
    }

    ImageHandle *bayer2yuvwithresize[thread_num];
    for (auto &convert : bayer2yuvwithresize) {
        convert = new Bayer2YUV420withResize(size, {1280, 720});
        dynamic_cast<ImageStream *>(bayer2yuvwithresize_stream)->addHandle(convert);
    }

    bayer2yuv_stream->DataTransfer(OnData_main);
    bayer2yuvwithresize_stream->DataTransfer(OnData_sub);

    bayer2yuv_stream->StartStream();
    bayer2yuvwithresize_stream->StartStream();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    bayer2yuv_stream->StopStream();
    bayer2yuvwithresize_stream->StopStream();

    std::this_thread::sleep_for(std::chrono::seconds(3));

    bayer2yuv_stream->StartStream();
    bayer2yuvwithresize_stream->StartStream();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    bayer2yuv_stream->StopStream();
    bayer2yuvwithresize_stream->StopStream();

    delete visual_stream;
    delete bayer2yuv_stream;
    delete bayer2yuvwithresize_stream;

    for (int i = 0; i < thread_num; i++) {
        delete bayer2yuv[i];
        delete bayer2yuvwithresize[i];
    }
}
