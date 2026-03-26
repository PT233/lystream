/**
 * @brief 图像流
 * @date 2022/11/28
 * @author laoyao
 */

#include "image_stream.h"

#include "camera.h"
#include "common.h"

using namespace lystream;
using namespace lycamera;
using namespace std::chrono;

ImageStream::ImageStream(Stream *stream, PolicyMode mode, unsigned int fps) : StreamControl(stream, mode, fps)
{}

ImageStream::~ImageStream()
{
    stream_ctrl_.isready = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    for (unsigned int i = 0; i < stream_ctrl_.handle_sum; i++) {
        StreamDataDeque::StreamData stream_data;
        stream_data.data    = stream_data_.buffer[i].aligndata;
        stream_data.datalen = stream_para_.size.width * stream_para_.size.height;
        stream_data_.buffer[i].data_queue->PushFront(stream_data);
        if (stream_ctrl_.handle_th[i].joinable()) stream_ctrl_.handle_th[i].join();

        delete[] stream_data_.buffer[i].rawdata;
        delete stream_data_.buffer[i].data_queue;
    }
}

void ImageStream::StartStream()
{
    StreamControl::StartStream();
    Initialization();
}

void ImageStream::StopStream()
{
    StreamControl::StopStream();
    if (!stream_para_.ref_count) {
        isrun_ = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void ImageStream::Initialization()
{
    lock_guard<mutex> lck(stream_ctrl_.mtx);
    
    isrun_ = true;
    if (stream_ctrl_.isready) return;

    unsigned int img_len    = stream_para_.size.width * stream_para_.size.height;
    stream_ctrl_.handle_sum = stream_data_.handle.size();


    for (unsigned int i = 0; i < stream_ctrl_.handle_sum; i++) {
        imagestream::StreamBuffer buffer {};
        buffer.rawdata    = new unsigned char[img_len + ZEROCOPY_ALIGNMENT];
        buffer.aligndata  = (unsigned char *)ALIGN_UP(buffer.rawdata);
        buffer.data_queue = new StreamDataDeque((int)img_len);
        stream_data_.buffer.push_back(buffer);

        void *out_data = nullptr;
        stream_data_.handle[i]->ImageProcessing(buffer.aligndata, &out_data);
    }

    for (unsigned int i = 0; i < stream_ctrl_.handle_sum; i++) {
        std::thread th = std::thread([this, &i] { DataHandle(i); });
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        stream_ctrl_.handle_th.push_back(std::move(th));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    stream_ctrl_.isready = true;
}

void ImageStream::DataTransferCall(void *data)
{
    if (!stream_ctrl_.isready || !isrun_) return;

    if (execute_mode_ == PolicyMode::ExecuteInterval) {
        steady_clock::time_point cur_clock_point = std::chrono::steady_clock::now();
        unsigned int cur_count
            = duration_cast<microseconds>(cur_clock_point - time_tick_.clock_point).count() / time_tick_.clock_tick;

        if (cur_count > time_tick_.time_count) { time_tick_.time_count = cur_count; }
        else return;
    }

    auto *current_frame = static_cast<CameraData *>(data);

    StreamDataDeque::StreamData stream_data;
    stream_data.data       = current_frame->data;
    stream_data.datalen    = current_frame->datalen;
    stream_data.extra_data = nullptr;

    stream_ctrl_.loop_count = (stream_ctrl_.loop_count + 1) <= stream_ctrl_.handle_sum ? stream_ctrl_.loop_count : 0;

    stream_data_.buffer[stream_ctrl_.loop_count].data_queue->PushFront(stream_data);
    ++stream_ctrl_.loop_count;
}

void ImageStream::DataHandle(unsigned int index)
{
    StreamDataDeque::StreamData stream_data;
    stream_data.data = stream_data_.buffer[index].aligndata;

    std::function<void(void *data)> onframe;
    void *out_data = nullptr;

    while (!stream_ctrl_.isready) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    stream_data_.buffer[index].data_queue->PopBack(&stream_data);
    while (stream_ctrl_.isready) {
        stream_data_.handle[index]->ImageProcessing(stream_data.data, &out_data);

        for (auto func = ondata_list_.begin(); func < ondata_list_.end(); func++) {
            onframe = *func;
            onframe(out_data);
        }

        stream_data_.buffer[index].data_queue->PopBack(&stream_data);
    }
}

void ImageStream::addHandle(ImageHandle *handle)
{
    stream_data_.handle.push_back(handle);
}
