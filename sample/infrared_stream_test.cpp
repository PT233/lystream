/**
 * @brief 红外图像流测试
 * @date 2022/11/28
 * @author laoyao
 */

#include "../infrared_stream.h"
#include "utils_log/utils_log.h"

#include <unistd.h>

using namespace lyboost::log;
using namespace lystream;

#define CAMERA_BRAND "Infrared"

Stream *stream = nullptr;

void OnData(void *data)
{
    auto *current_frame = static_cast<ImageHandle::ImgData *>(data);
    //    Log_Normal << logging::add_value("Tag", CAMERA_BRAND)
    //               << "infrared recv, datalen:" << current_frame->size.width * current_frame->size.height
    //               << ", width:" << current_frame->size.width << ", height:" << current_frame->size.height << endl;
    //
    //    RoiRect rect = {0};
    //    unsigned short x, y;
    //    int temp = 0;
    //    temp     = dynamic_cast<InfraredStream *>(stream)->getHighestTemperature(rect, x, y);
    //    printf("the temp is %d, x is %d, y is %d\n", temp, x, y);
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

    //    static int cnt      = 0;
    //    {
    //        char buf[64];
    //        sprintf(buf, "./p%d.yuv", cnt);
    //        printf("AVFrame callback %d\n", cnt);
    //
    //        FILE *fp = fopen(buf, "wb");
    //        assert(fp);
    //        fwrite((const char *)current_frame->plane[0], 384 * 288, 1, fp);
    //        fwrite((const char *)current_frame->plane[1], 384 * 288 / 4, 1, fp);
    //        fwrite((const char *)current_frame->plane[2], 384 * 288 / 4, 1, fp);
    //        fclose(fp);
    //    }
    //    exit(1);
}

int main()
{
    stream = new InfraredStream("A", "192.168.10.5");

    stream->DataTransfer(OnData);
    stream->StartStream();

    sleep(30);

    delete stream;
}