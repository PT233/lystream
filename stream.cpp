/**
 * @brief 流基本抽象接口
 * @date 2022/11/28
 * @author laoyao
 */

#include "stream.h"

#include "utils_log/utils_log.h"

#include <boost/format.hpp>
#include <cuda_runtime.h>
#include <helper_cuda.h>

using namespace lystream;
using namespace lyboost::log;

Stream::Stream()
{
    static bool first_run = true;
    if (first_run) {
        cudaDeviceProp prop {};
        checkCudaErrors(cudaGetDeviceProperties(&prop, 0));
        compute_capability_ = static_cast<float>(prop.major) + static_cast<float>(prop.minor) / 10.0f;
        first_run           = !first_run;

        Log_Normal << logging::add_value("Tag", stream_para_.name)
                   << boost::format("the current GPU device's compute capability is %1%") % compute_capability_;
    }

    ondata_list_.reserve(max_num_);
}

Stream::~Stream() = default;

void Stream::DataTransfer(const std::function<void(void *data)> &ondata)
{
    ondata_list_.push_back(ondata);
}

[[maybe_unused]] [[nodiscard]] bool Stream::getStreamStatus()
{
    return isrun_;
}

Stream::StreamPara Stream::getStreamPara()
{
    return stream_para_;
}

void Stream::setName(const std::string &name)
{
    stream_para_.name = name;
}