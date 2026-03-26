/**
 * @brief 流控制抽象接口
 * @date 2022/11/28
 * @author laoyao
 */

#include "stream_control.h"

#include "utils_log/utils_log.h"

#include <boost/format.hpp>

using namespace lystream;
using namespace lyboost::log;

StreamControl::StreamControl(Stream *stream, PolicyMode mode, unsigned int fps) : execute_mode_(mode), stream_(stream)
{
    if (execute_mode_ == PolicyMode::ExecuteInterval) {
        time_tick_.clock_tick  = 1000 * 1000 / fps;
        time_tick_.clock_point = std::chrono::steady_clock::now();
    }
}

StreamControl::~StreamControl() = default;

void StreamControl::StartStream()
{
    ++stream_para_.ref_count;
    Log_Normal << logging::add_value("Tag", stream_para_.name)
               << boost::format("the reference count of current stream is %1%") % stream_para_.ref_count;

    if (first_run_) {
        stream_->DataTransfer([this](auto &&PH1) { DataTransferCall(std::forward<decltype(PH1)>(PH1)); });
        first_run_ = !first_run_;
    }

    stream_->StartStream();
    stream_para_.size = stream_->getStreamPara().size;
}

void StreamControl::StopStream()
{
    --stream_para_.ref_count;
    Log_Normal << logging::add_value("Tag", stream_para_.name)
               << boost::format("the reference count of current stream is %1%") % stream_para_.ref_count;
    stream_->StopStream();
}