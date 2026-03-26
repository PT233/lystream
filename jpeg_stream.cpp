/**
 * @brief Jpeg图像流
 * @date 2022/11/28
 * @author laoyao
 */

#include "jpeg_stream.h"

#include "handle/image/image_handle.h"
#include "system/system.h"

using namespace lystream;
using namespace std::chrono;

JpegStream::JpegStream(Stream *stream, PolicyMode mode, unsigned int fps, StreamSize size) :
    StreamControl(stream, mode, fps), size_(size)
{}

JpegStream::~JpegStream()
{
    stream_ctrl_.isready = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    unsigned char *yuv_buffers[3] = {nullptr, nullptr, nullptr};
    for (auto &buffer : yuv_buffers) { buffer = new unsigned char[stream_para_.size.width * stream_para_.size.height]; }
    for (unsigned int i = 0; i < jpegstream::thread_num; i++) {
        encode_[i]->QBuffer(yuv_buffers);
        if (stream_ctrl_.handle_th[i].joinable()) stream_ctrl_.handle_th[i].join();
    }
    for (auto &buffer : yuv_buffers) delete[] buffer;
    for (auto &encode : encode_) { encode = nullptr; }
}

void JpegStream::StartStream()
{
    StreamControl::StartStream();
    Initialization();
}

void JpegStream::StopStream()
{
    StreamControl::StopStream();
    if (!stream_para_.ref_count) {
        isrun_ = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void JpegStream::Initialization()
{
    lock_guard<mutex> lck(stream_ctrl_.mtx);

    isrun_ = true;
    if (stream_ctrl_.isready) return;

    if (size_.width * size_.height > 0) {
        stream_para_.size.width  = size_.width;
        stream_para_.size.height = size_.height;
    }
    CodeSize size = {static_cast<int>(stream_para_.size.width), static_cast<int>(stream_para_.size.height)};

    for (auto &encode : encode_) {
        encode = NvdiaJpegEncoder::CreateNvJpegEncoder("jpeg_encode", size);
        JpegSetting(encode.get());
        encode->setV4l2Mode();
        encode->Initialization();
    }

    stream_ctrl_.handle_th[0] = std::thread([this] { Handle(encode_[0].get()); });
    stream_ctrl_.handle_th[1] = std::thread([this] { Handle(encode_[1].get()); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stream_ctrl_.isready = true;
}

void JpegStream::DataTransferCall(void *data)
{
    if (!stream_ctrl_.isready || !isrun_) return;

    std::lock_guard<mutex> lck(stream_ctrl_.mtx);

    if (execute_mode_ == PolicyMode::ExecuteInterval) {
        steady_clock::time_point cur_clock_point = std::chrono::steady_clock::now();
        unsigned int cur_count
            = duration_cast<microseconds>(cur_clock_point - time_tick_.clock_point).count() / time_tick_.clock_tick;

        if (cur_count > time_tick_.time_count) { time_tick_.time_count = cur_count; }
        else return;
    }

    auto *current_data = (ImageHandle::ImgData *)data;

    if (stream_ctrl_.isexchange) { encode_[0]->QBuffer(current_data->plane); }
    else {
        encode_[1]->QBuffer(current_data->plane);
    }
    stream_ctrl_.isexchange = !stream_ctrl_.isexchange;
}

void JpegStream::Handle(NvdiaJpegEncoder *encode)
{
    jpegstream::JpegData jpeg_data;
    jpeg_data.datalen = stream_para_.size.width * stream_para_.size.height * 3 / 2;
    jpeg_data.data    = new unsigned char[jpeg_data.datalen];

    std::function<void(void *data)> onframe;

    if (!encode->DqBuffer(jpeg_data.data, jpeg_data.datalen)) return;

    while (stream_ctrl_.isready) {
        for (auto func = ondata_list_.begin(); func < ondata_list_.end(); func++) {
            onframe = *func;
            onframe(&jpeg_data);
        }

        encode->DqBuffer(jpeg_data.data, jpeg_data.datalen);
    }

    delete[] jpeg_data.data;
}

void JpegStream::setJpegPara(jpegstream::JpegPara para)
{
    jpeg_para_ = para;
}

jpegstream::JpegPara JpegStream::getDefaultPara()
{
    jpegstream::JpegPara para;

    para.level = 75;
    para.scale = {0, 0};
    para.roi   = {0, 0, 0, 0};

    return para;
}

void JpegStream::JpegSetting(NvdiaJpegEncoder *encode) const
{
    if (jpeg_para_.level != 0) encode->setQuality((int)jpeg_para_.level);
    if (jpeg_para_.scale.width != 0 && jpeg_para_.scale.height != 0) {
        encode->setScale((int)jpeg_para_.scale.width, (int)jpeg_para_.scale.height);
    }

    if (jpeg_para_.roi.width != 0 && jpeg_para_.roi.height != 0) {
        CodecRoi roi = {static_cast<int>(jpeg_para_.roi.x), static_cast<int>(jpeg_para_.roi.y),
                        static_cast<int>(jpeg_para_.roi.width), static_cast<int>(jpeg_para_.roi.height)};
        encode->setCropRect(roi);
    }
}
