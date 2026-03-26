/**
 * @brief Video流测试
 * @date 2022/11/28
 * @author laoyao
 */

#include "handle/image/bayer2yuv420.h"
#include "handle/image/bayer2yuv420withresize.h"
#include "image_stream.h"
#include "utils_log/utils_log.h"
#include "video_stream.h"
#include "visual_stream.h"

#include <unistd.h>

using namespace lyboost::log;
using namespace lystream;

#define CAMERA_BRAND "HIK"

void OnDataSub(void *data)
{
    //    auto *current_data = (VideoStream::VideoData *)data;
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
        printf("video sub encode rate is %d\n", ui32FrameCount);
        ui32FrameCount = 0;
    }
}

void OnDataMain(void *data)
{
    //    auto *current_data = (VideoStream::VideoData *)data;
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
        printf("video main encode rate is %d\n", ui32FrameCount);
        ui32FrameCount = 0;
    }
}

int main()
{
    ImgSize size = {2448, 2048};

    Stream *visual_stream                     = new VisualStream(CAMERA_BRAND, 0);
    StreamControl *bayer2yuv_stream           = new ImageStream(visual_stream);
    StreamControl *bayer2yuvwithresize_stream = new ImageStream(visual_stream);
    ImageHandle *bayer2yuv                    = new Bayer2YUV420(size);
    ImageHandle *bayer2yuvwithresize          = new Bayer2YUV420withResize(size, {1280, 720});
    StreamControl *video_stream
        = new VideoStream(bayer2yuv_stream, {size.width, size.height}, PolicyMode::ExecuteInterval, 30);

    StreamControl *video_streamsub
        = new VideoStream(bayer2yuvwithresize_stream, {1280, 720}, PolicyMode::ExecuteInterval, 25);

    dynamic_cast<ImageStream *>(bayer2yuv_stream)->addHandle(bayer2yuv);
    dynamic_cast<ImageStream *>(bayer2yuvwithresize_stream)->addHandle(bayer2yuvwithresize);

    {
        VideoStream::VideoPara video_para = dynamic_cast<VideoStream *>(video_stream)->getDefaultPara("H265");
        dynamic_cast<VideoStream *>(video_stream)->setVideoPara(video_para);
        video_stream->DataTransfer(OnDataMain);
        video_stream->StartStream();
    }
    {
        VideoStream::VideoPara video_para = dynamic_cast<VideoStream *>(video_streamsub)->getDefaultPara("H265");
        dynamic_cast<VideoStream *>(video_streamsub)->setVideoPara(video_para);
        video_streamsub->DataTransfer(OnDataSub);
        video_streamsub->StartStream();
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));

    video_stream->StopStream();
    video_streamsub->StopStream();

    std::this_thread::sleep_for(std::chrono::seconds(3));

    video_stream->StartStream();
    video_streamsub->StartStream();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    video_stream->StopStream();
    video_streamsub->StopStream();

    delete visual_stream;

    delete bayer2yuv_stream;
    delete bayer2yuvwithresize_stream;
    delete bayer2yuv;
    delete bayer2yuvwithresize;
    delete video_stream;
    delete video_streamsub;
}