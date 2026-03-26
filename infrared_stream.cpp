/**
 * @brief 红外相机stream接口
 * @date 2022/11/28
 * @author laoyao
 */

#include "infrared_stream.h"

#ifdef IRay
    #include "iray/iray.h"
using namespace lycamera::iray;
#endif

#ifdef MAGNITY
    #include "magnity/magnity.h"
using namespace lycamera::magnity;
#endif

#include "utils_log/utils_log.h"

#include <boost/format.hpp>
#include <memory>
#include <utility>
#include <yaml-cpp/yaml.h>

using namespace lystream;
using namespace lyboost::log;

#ifdef NDEBUG
    #define DEFAULT_FILENAME "/opt/lystream/config/infrared_stream.yaml"
#else
    #define DEFAULT_FILENAME "/root/lystream/config/infrared_stream.yaml"
#endif

#define TAG_LOG "InfraredStream"

InfraredStream::InfraredStream(std::string_view type, std::string_view ip)
{
    stream_data_.device_para.type = type;
    stream_data_.device_para.ip   = ip;

    if (stream_data_.device_para.type == "A") {
#ifdef IRay
        device_ = shared_ptr<CameraDevice>(new CameraIRay(0, CamIrTypeA, stream_data_.device_para.ip));

        dynamic_cast<CameraIRay *>(device_.get())->temp_frame_ = [this](auto &&PH1, auto &&PH2) {
            TempCallback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
        };
#endif
        for (auto &temp_queue : stream_data_.temp_queue) {
            temp_queue = new StreamDataDeque(384 * 288 * sizeof(float));
        }

        for (unsigned int i = 0; i < 2; i++) {
            stream_ctrl_.handle_th[i + 1] = std::thread([this, i] { TempMatrixHandle(i); });
        }

        decoder_.open();
    }
    else if (stream_data_.device_para.type == "B") {
#ifdef IRay
        device_ = shared_ptr<CameraDevice>(new CameraIRay(0, CamIrTypeB, stream_data_.device_para.ip));
        dynamic_cast<CameraIRay *>(device_.get())->temp_frame_ = [this](auto &&PH1, auto &&PH2) {
            TempCallback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
        };
#endif
        for (auto &temp_queue : stream_data_.temp_queue) {
            temp_queue = new StreamDataDeque(640 * 512 * sizeof(float));
        }
        for (unsigned int i = 0; i < 2; i++) {
            stream_ctrl_.handle_th[i + 1] = std::thread([this, i] { TempMatrixHandle(i); });
        }

    }
    else if (stream_data_.device_para.type == "C") {
#ifdef MAGNITY
        YAML::Node config = YAML::LoadFile(DEFAULT_FILENAME);
        device_           = shared_ptr<CameraDevice>(new CameraMAG(0, CamIrTypeC, stream_data_.device_para.ip));
        dynamic_cast<CameraMAG *>(device_.get())->temp_frame_ = [this](auto &&PH1, auto &&PH2) {
            TempCallback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
        };
        CameraMAG::setEmissivity(config["CameraSettingPara"]["Emissivity"].as<float>());
#endif
    }
}

InfraredStream::~InfraredStream()
{
    isrun_ = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    device_->StopCapture();
    device_->DeInitCameraDevice();
    device_ = nullptr;
    if (stream_data_.device_para.type == "A") decoder_.close();
    if (stream_data_.device_para.type == "A" || stream_data_.device_para.type == "B") {
        //        if (stream_data_.device_para.type == "A") {
        //            if (stream_ctrl_.handle_th[0].joinable()) stream_ctrl_.handle_th[0].join();
        //        }
        for (auto &temp_queue : stream_data_.temp_queue) {
            auto *temp_data = new float[stream_para_.size.width * stream_para_.size.height];
            StreamDataDeque::StreamData stream_data;
            stream_data.data       = (unsigned char *)temp_data;
            stream_data.datalen    = stream_para_.size.width * stream_para_.size.height * sizeof(float);
            stream_data.extra_data = nullptr;

            temp_queue->PushFront(stream_data);
            delete[] temp_data;
        }
        for (unsigned int i = 0; i < 2; i++) {
            if (stream_ctrl_.handle_th[i + 1].joinable()) stream_ctrl_.handle_th[i + 1].join();
        }

        for (auto &temp_queue : stream_data_.temp_queue) delete temp_queue;
    }

    for (auto &plane : stream_data_.img_data.plane) delete[] plane;
}

void InfraredStream::StartStream()
{
    ++stream_para_.ref_count;
    if (isrun_) return;

    while (!device_->InitCameraDevice()) {
        device_->DeInitCameraDevice();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    device_->setFrame([this](auto &&PH1) { OnFrame(std::forward<decltype(PH1)>(PH1)); });

    WriteCameraPara();
    device_->StartCapture();

    int width = 0, height = 0;
    device_->getIntValue("Width", width);
    stream_para_.size.width = width;
    device_->getIntValue("Height", height);
    stream_para_.size.height = height;

    for (auto &img : stream_data_.img_data.plane)
        img = new unsigned char[static_cast<int>(stream_para_.size.width) * static_cast<int>(stream_para_.size.height)];

    stream_data_.img_data.size
        = {static_cast<int>(stream_para_.size.width), static_cast<int>(stream_para_.size.height)};

    isrun_ = true;
    //    if (stream_data_.device_para.type == "A") {
    //        stream_ctrl_.handle_th[0] = std::thread([this] { VideoHandle(); });
    //    }
}

void InfraredStream::StopStream()
{
    --stream_para_.ref_count;
    if (!stream_para_.ref_count) {
        isrun_ = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void InfraredStream::OnFrame(void *frame)
{
    if (!isrun_) return;
    //    static int index = 0;
    //    Log_Normal << logging::add_value("Tag", "111") << "the num is " << index++;
    auto *current_data = (CameraData *)frame;

    if (stream_data_.device_para.type == "B" || stream_data_.device_para.type == "C") {
        int len = stream_data_.img_data.size.width * stream_data_.img_data.size.height;
        for (int i = 0; i < 3; i++) {
            if (i == 0) { memcpy(stream_data_.img_data.plane[i], current_data->data, len); }
            else {
                memcpy(stream_data_.img_data.plane[i], current_data->data + len + (i - 1) * len / 4, len / 4);
            }
        }
    }
    else if (stream_data_.device_para.type == "A") {
        if (!decoder_.naluDecode((unsigned char *)current_data->data, static_cast<int>(current_data->datalen),
                                 stream_data_.img_data.plane))
            return;
        //        static bool first_frame = true;
        //        if (first_frame) {
        //            if (current_data->data[4] == 0x61 || current_data->data[4] == 0x65) {
        //                decoder_.naluDecode(current_data->data, static_cast<int>(current_data->datalen),
        //                                    stream_data_.img_data.plane);
        //                first_frame = false;
        //            }
        //        }
        //        else {
        //            decoder_.naluDecode(current_data->data, static_cast<int>(current_data->datalen),
        //                                stream_data_.img_data.plane);
        //        }
    }
    std::function<void(void *data)> onframe;
    for (auto func = ondata_list_.begin(); func < ondata_list_.end(); func++) {
        onframe = *func;
        onframe(&stream_data_.img_data);
    }
}

void InfraredStream::WriteCameraPara()
{}

void InfraredStream::TempCallback(char *data, long datalen)
{
    if (!isrun_) return;

    unsigned int templen = stream_para_.size.width * stream_para_.size.height;
    auto *temp           = new unsigned short[templen];

    if (stream_data_.device_para.type == "A") { memcpy(temp, data, templen * 2); }
    else if (stream_data_.device_para.type == "B") {
        memcpy(temp, data, templen * 2);

        for (unsigned int ii = 0; ii < templen / 2; ii++) {
            temp[ii * 2]     = (unsigned short)((unsigned short)(data[ii * 2] << 8) + data[ii * 2 + 1 + templen]);
            temp[ii * 2 + 1] = (unsigned short)((unsigned short)(data[ii * 2 + 1] << 8) + data[ii * 2 + templen]);
        }
    }

    if (stream_data_.device_para.type == "C") {
        auto *temp_data = (InfraredTemp *)data;
        stream_data_.temp.store(*temp_data);
    }
    else if (stream_data_.device_para.type == "A" || stream_data_.device_para.type == "B") {
        auto *temp_data = new float[templen];

        float ftempvalue;
        for (unsigned int j = 0; j < stream_para_.size.height; j++) {
            for (unsigned int i = 0; i < stream_para_.size.width; i++) {
                if (temp[j * stream_para_.size.height + i] > 7300) // 7301~16383， (Value-3300)/15-273.15
                {
                    ftempvalue = 15.0;
                    temp_data[j * stream_para_.size.width + i]
                        = ((float)temp[j * stream_para_.size.width + i] - 3300.0f) / ftempvalue - 273.15f;
                }
                else // 0~7300，则温度换算公式（换算为摄氏度）(Value + 7000) / 30 - 273.15
                {
                    ftempvalue = 30.0;
                    temp_data[j * stream_para_.size.width + i]
                        = ((float)temp[j * stream_para_.size.width + i] + 7000.0f) / ftempvalue - 273.15f;
                }
            }
        }

        StreamDataDeque::StreamData stream_data;
        stream_data.data       = (unsigned char *)temp_data;
        stream_data.datalen    = templen * sizeof(float);
        stream_data.extra_data = nullptr;

        stream_ctrl_.loop_count = (stream_ctrl_.loop_count + 1) <= handle_sum_ ? stream_ctrl_.loop_count : 0;
        stream_data_.temp_queue[stream_ctrl_.loop_count]->PushFront(stream_data);
        ++stream_ctrl_.loop_count;

        delete[] temp_data;
    }
    delete[] temp;
}

// void InfraredStream::VideoHandle()
//{
//     if (!isrun_) return;
//
//     if (decode_ != nullptr) decode_->DqBuffer(stream_data_.img_data.plane);
//     while (isrun_) {
//         std::function<void(void *data)> onframe;
//         for (auto func = ondata_list_.begin(); func < ondata_list_.end(); func++) {
//             onframe = *func;
//             onframe(&stream_data_.img_data);
//         }
//
//         if (decode_ != nullptr) decode_->DqBuffer(stream_data_.img_data.plane);
//     }
// }

int InfraredStream::getHighestTemperature(RoiRect rect, unsigned short &x, unsigned short &y)
{
    static RoiRect current_rect = {0};

    float temp_highest;

    if (current_rect.x != rect.x || current_rect.y != rect.y || current_rect.width != rect.width
        || current_rect.height != rect.height) {
        current_rect = rect;
        if (stream_data_.device_para.type == "C") {
#ifdef MAGNITY
            dynamic_cast<CameraMAG *>(device_.get())->setTempRoi(rect.x, rect.y, rect.width, rect.height);
#endif
        }
        else {
            stream_data_.rect.store(current_rect);
        }
    }

    InfraredTemp current_temp = stream_data_.temp.load();
    x                         = current_temp.x;
    y                         = current_temp.y;
    temp_highest              = current_temp.temp;

    return (int)(static_cast<float>(temp_highest) * 10.0f);
}

InfraredStream::CameraPara InfraredStream::ReadCameraPara()
{
    stream_data_.device_para.size = {stream_para_.size.width, stream_para_.size.height};
    return stream_data_.device_para;
}

void InfraredStream::TempMatrixHandle(unsigned int index)
{
    StreamDataDeque::StreamData stream_data;
    stream_data.data = new unsigned char[640 * 512 * sizeof(float)];
    InfraredTemp current_temp;

    stream_data_.temp_queue[index]->PopBack(&stream_data);

    while (isrun_) {
        auto *temp_data      = (float *)stream_data.data;
        RoiRect current_rect = stream_data_.rect.load();
        if (current_rect.width == 0 && current_rect.height == 0) {
            current_rect.x      = 0;
            current_rect.y      = 0;
            current_rect.width  = static_cast<int>(stream_para_.size.width) - 1;
            current_rect.height = static_cast<int>(stream_para_.size.height) - 1;
        }

        current_temp.temp = temp_data[(int)current_rect.y * stream_para_.size.width + (int)current_rect.x];
        for (int j = static_cast<int>(current_rect.y); j < static_cast<int>(current_rect.y) + current_rect.height;
             j++) {
            for (int i = static_cast<int>(current_rect.x); i < static_cast<int>(current_rect.x) + current_rect.width;
                 i++) {
                if (temp_data[j * stream_para_.size.width + i] > current_temp.temp) {
                    current_temp.temp = temp_data[j * stream_para_.size.width + i];
                    current_temp.x    = i;
                    current_temp.y    = j;
                }
            }
        }
        stream_data_.temp.store(current_temp);

        stream_data_.temp_queue[index]->PopBack(&stream_data);
    }
    delete[] stream_data.data;
}