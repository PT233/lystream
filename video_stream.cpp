/**
 * @brief 视频流
 * @date 2022/11/28
 * @author laoyao
 */

#include "video_stream.h"

#include "handle/image/image_handle.h"
#include "utils/system/system.h"

#include <utility>

using namespace lystream;
using namespace std::chrono;

VideoStream::VideoStream(Stream *stream, CodecSize size, PolicyMode mode, unsigned int fps) :
    StreamControl(stream, mode, fps)
{
    size_ = size;
}

VideoStream::~VideoStream()
{
    stream_ctrl_.isready = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    encode_ = nullptr;
    if (stream_ctrl_.handle_th.joinable()) stream_ctrl_.handle_th.join();
}

void VideoStream::StartStream()
{
    StreamControl::StartStream();
    Initialization();
}

void VideoStream::StopStream()
{
    StreamControl::StopStream();
    if (!stream_para_.ref_count) {
        isrun_ = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void VideoStream::Initialization()
{
    lock_guard<mutex> lck(stream_ctrl_.mtx);
    isrun_ = true;

    if (stream_ctrl_.isready) return;

    stream_para_.size = {static_cast<unsigned int>(size_.width), static_cast<unsigned int>(size_.height)};

    encode_ = std::make_shared<NvdiaVideoEncoder>("video_encode", size_);
    VideoSetting(encode_.get());
    encode_->Initialization();

    stream_ctrl_.handle_th = std::thread([this] { Handle(encode_.get()); });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stream_ctrl_.isready = true;
}

void VideoStream::DataTransferCall(void *data)
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
    if (handle_ != nullptr) {
        VideoHandle::VideoData *out_data = nullptr;
        handle_->ImageProcessing(current_data, (void **)&out_data);
        encode_->QBuffer(out_data->plane);
    }
    else {
        encode_->QBuffer(current_data->plane);
    }
}

void VideoStream::VideoSetting(NvdiaVideoEncoder *encode)
{
    encode->setEncoderType(video_para_.encoder_type);
    encode->setFps((int)video_para_.fps, 1);
    encode->setRateMode(video_para_.ratemode.c_str());
    if (video_para_.bitrate == 0) {
        encode->setBitrate(getVideoRate(stream_para_.size.width * stream_para_.size.height, video_para_.encoder_type));
    }
    else encode->setBitrate(video_para_.bitrate);
    encode->setProfile(video_para_.profile.c_str());
    encode->setLevel(const_cast<char *>(video_para_.level.c_str()));
    encode->setColorSpace(video_para_.colorspace.c_str());
    if (video_para_.max_permode) encode->setMaxPermode(video_para_.max_permode);
    if (video_para_.all_iframe) encode->setAllIframe();
    encode->setIfrmaeInterval((int)video_para_.ifrmae_interval);
    encode->setIdrInterval((int)video_para_.idr_interval);
    if (video_para_.insert_spspps_idr) encode->setInsertSpsPpsIdr();
    if (video_para_.insert_vui) encode->setInsertVui();
}

void VideoStream::Handle(NvdiaVideoEncoder *encode)
{
    VideoData video_data;
    video_data.datalen = stream_para_.size.width * stream_para_.size.height * 3 / 2;
    video_data.data    = new unsigned char[video_data.datalen];

    std::function<void(void *data)> onframe;

    if (!encode->DqBuffer(video_data.data, video_data.datalen)) return;

    while (stream_ctrl_.isready) {
        for (auto func = ondata_list_.begin(); func < ondata_list_.end(); func++) {
            onframe = *func;
            onframe(&video_data);
        }
        encode->DqBuffer(video_data.data, video_data.datalen);
    }

    delete[] video_data.data;
}

void VideoStream::setVideoPara(VideoStream::VideoPara para)
{
    video_para_ = std::move(para);
}

VideoStream::VideoPara VideoStream::getDefaultPara(const string &encode_type) const
{
    VideoPara para;

    para.encoder_type = encode_type;

    if (encode_type == "H264") {
        para.level   = "5.1";
        para.profile = "high";
    }
    else if (encode_type == "H265") {
        para.level   = "main5.1";
        para.profile = "main";
    }

    para.fps               = 30;
    para.ratemode          = "cbr";
    para.bitrate           = getVideoRate(stream_para_.size.width * stream_para_.size.height, encode_type);
    //    para.peak_bitrate      = getVideoRate(stream_ctrl_.roi.width * stream_ctrl_.roi.height, encode_type);
    para.colorspace        = "BT.601";
    para.max_permode       = true;
    para.all_iframe        = false;
    para.ifrmae_interval   = 10;
    para.idr_interval      = 30;
    para.insert_vui        = true;
    para.insert_spspps_idr = true;

    return para;
}

unsigned int VideoStream::getVideoRate(unsigned int pixlen, const std::string &videotype)
{
    if (videotype == "H264") {
        if (pixlen <= 384 * 288) return 512 * 1024;
        if (pixlen <= 720 * 576) return 1 * 1024 * 1024;
        if (pixlen <= 1280 * 720) return 2 * 1024 * 1024;
        if (pixlen <= 1280 * 1080) return 2 * 1024 * 1024;
        if (pixlen <= 1920 * 1080) return 4 * 1024 * 1024;
        if (pixlen <= 3000 * 1000) return 4 * 1024 * 1024;
        if (pixlen <= 4000 * 1000) return 4 * 1024 * 1024;
        if (pixlen <= 5000 * 1000) return 4 * 1024 * 1024;
        if (pixlen <= 6000 * 1000) return 4 * 1024 * 1024;
        if (pixlen <= 8000 * 1000) return 4 * 1024 * 1024;
        return 4 * 1024 * 1024;
    }
    else if (videotype == "H265") {
        if (pixlen <= 720 * 576) return 512 * 1024;
        if (pixlen <= 1280 * 720) return 1 * 1024 * 1024;
        if (pixlen <= 1280 * 960) return 1 * 1024 * 1024;
        if (pixlen <= 1920 * 1080) return 2 * 1024 * 1024;
        if (pixlen <= 3000 * 1000) return 2 * 1024 * 1024;
        if (pixlen <= 4000 * 1000) return 2 * 1024 * 1024;
        if (pixlen <= 5000 * 1000) return 3 * 1024 * 1024;
        if (pixlen <= 6000 * 1000) return 3 * 1024 * 1024;
        if (pixlen <= 8000 * 1000) return 4 * 1024 * 1024;
        return 4 * 1024 * 1024;
    }
    else return 4 * 1024 * 1024;
}

void VideoStream::addHandle(VideoHandle *handle)
{
    handle_ = handle;
}
