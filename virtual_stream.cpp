/**
 * @brief 虚拟相机stream接口
 * @date 2022/11/28
 * @author laoyao
 */

#include "virtual_stream.h"

#include "utils_log/utils_log.h"

#include <boost/format.hpp>
#include <cstring>

using namespace std::chrono;
using namespace lystream;
using namespace lyboost::log;

VirtualStream::VirtualStream() = default;

VirtualStream::~VirtualStream()
{
    isend_ = false;
    if (gather_th_.joinable()) gather_th_.join();
    delete[] frame_data_.data;
}

void VirtualStream::StartStream()
{
    ++stream_para_.ref_count;
    Log_Normal << logging::add_value("Tag", stream_para_.name)
               << boost::format("the reference count of current stream is %1%") % stream_para_.ref_count;
    isrun_ = true;

    if (!first_call_) return;

    stream_para_.size = {frame_data_.width, frame_data_.height};
    gather_th_        = std::thread([this](auto &&PH1) { OnRecvFrame(std::forward<decltype(PH1)>(PH1)); }, nullptr);

    first_call_ = !first_call_;
}

void VirtualStream::StopStream()
{
    --stream_para_.ref_count;
    Log_Normal << logging::add_value("Tag", stream_para_.name)
               << boost::format("the reference count of current stream is %1%") % stream_para_.ref_count;
    if (!stream_para_.ref_count) isrun_ = false;
}

void VirtualStream::CameraOnFrame(void *frame)
{
    if (!isrun_) return;

    std::function<void(void *data)> onframe;

    for (auto func = ondata_list_.begin(); func < ondata_list_.end(); func++) {
        onframe = *func;
        onframe(frame);
    }
}

void VirtualStream::OnRecvFrame([[maybe_unused]] void *param)
{
    while (isend_) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));

        steady_clock::time_point cur_clock_point = std::chrono::steady_clock::now();
        unsigned int cur_count
            = duration_cast<microseconds>(cur_clock_point - frame_ctrl_.time_tick.clock_point).count()
              / frame_ctrl_.time_tick.clock_tick;
        if (cur_count > frame_ctrl_.time_tick.time_count) { frame_ctrl_.time_tick.time_count = cur_count; }
        else continue;

        CameraOnFrame(&frame_data_);
    }
}

void VirtualStream::setCameraInfo(CameraData data, unsigned int fps)
{
    frame_data_      = data;
    frame_data_.data = new unsigned char[data.datalen];
    memcpy(frame_data_.data, data.data, frame_data_.datalen);

    frame_ctrl_.time_tick.clock_tick  = 1000 * 1000 / fps;
    frame_ctrl_.time_tick.clock_point = std::chrono::steady_clock::now();
}