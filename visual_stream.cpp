/**
 * @brief 可见光相机stream接口
 * @date 2022/11/28
 * @author laoyao
 */

#include "visual_stream.h"

#ifdef DaHeng
    #include "daheng/daheng.h"
using namespace lycamera::daheng;
#endif

#ifdef HIK
    #include "hik/hik.h"
using namespace lycamera::hik;
#endif

#ifdef IKapC
    #include "ikapc/ikapc.h"
using namespace lycamera::ikapc;
#endif

#ifdef DALSA
    #include "dalsa/dalsa.h"
using namespace lycamera::dalsa;
#endif

#ifdef Pylon
    #include "pylon/pylon.h"
using namespace lycamera::pylon;
#endif

#include "utils/parse/parse_config.h"
#include "utils_log/utils_log.h"

#include <boost/format.hpp>
#include <experimental/filesystem>
#include <utility>

using namespace lystream;
using namespace lycamera;
using namespace lyboost::log;

#ifdef NDEBUG
    #define CAMERA_FILEPATH  "/opt/lystream/config/"
    #define DEFAULT_FILENAME "/opt/lystream/config/visual_stream0.yaml"
#else
    #define CAMERA_FILEPATH  "/root/lystream/config/"
    #define DEFAULT_FILENAME "/root/lystream/config/visual_stream0.yaml"
#endif

VisualStream::VisualStream(std::string brand, unsigned int index)
{
    device_para_.index = index;
    stream_para_.name  = u8"VisualStream";

    if (brand.empty()) { Log_Error << logging::add_value("Tag", stream_para_.name) << "camera brand empty"; }

#ifdef DaHeng
    else if (brand == "DaHeng") {
        device_ = shared_ptr<CameraDevice>(new CameraDaheng(index));
    }
#endif

#ifdef HIK
    else if (brand == "HIK") {
        device_ = shared_ptr<CameraDevice>(new CameraHik(index));
    }
#endif

#ifdef IKapC
    else if (brand == "IKapC") {
        device_ = shared_ptr<CameraDevice>(new CameraIKapC(index));
    }
#endif

#ifdef DALSA
    else if (brand == "DALSA") {
        device_ = shared_ptr<CameraDevice>(new CameraDalsa(index));
    }
#endif

#ifdef Pylon
    else if (brand == "Pylon") {
        device_ = shared_ptr<CameraDevice>(new CameraPylon(index));
    }
#endif

    else {
        Log_Error << logging::add_value("Tag", stream_para_.name)
                  << boost::format("not support camera brand : %1%, please check camera config") % brand;
    }
    device_->setFrame([this](auto &&PH1) { CameraOnFrame(std::forward<decltype(PH1)>(PH1)); });
}

VisualStream::~VisualStream()
{
    if (isrun_) {
        isrun_ = !isrun_;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        device_->StopCapture();
        device_->DeInitCameraDevice();
    }
    device_ = nullptr;
}

void VisualStream::StartStream()
{
    ++stream_para_.ref_count;
    Log_Normal << logging::add_value("Tag", stream_para_.name)
               << boost::format("the reference count of current stream is %1%") % stream_para_.ref_count;
    isrun_ = true;
    if (!first_run_) return;

    while (!device_->InitCameraDevice()) {
        device_->DeInitCameraDevice();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    WriteCameraParaFromFile();
    device_->StartCapture();

    stream_para_.size = {static_cast<unsigned int>(device_para_.setting.roi.width),
                         static_cast<unsigned int>(device_para_.setting.roi.height)};
    first_run_        = !first_run_;
}

void VisualStream::StopStream()
{
    --stream_para_.ref_count;
    Log_Normal << logging::add_value("Tag", stream_para_.name)
               << boost::format("the reference count of current stream is %1%") % stream_para_.ref_count;

    if (!stream_para_.ref_count) {
        isrun_ = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        device_->StopCapture();
        device_->DeInitCameraDevice();
        first_run_ = true;
    }
}

void VisualStream::CameraOnFrame(void *frame)
{
    if (!isrun_) return;

    std::function<void(void *data)> onframe;
    for (auto func = ondata_list_.begin(); func < ondata_list_.end(); func++) {
        onframe = *func;
        onframe(frame);
    }
}

void VisualStream::WriteCameraPara(VisualStream::CameraPara &para)
{
    //    device_->StopCapture();
    //    device_->DeInitCameraDevice();
    //    sleep(1);
    //    device_->InitCameraDevice();

    device_->StopCapture();

    //    bool is_stop_grab = false;

    if (para.setting.roi.width != device_para_.setting.roi.width
        || para.setting.roi.height != device_para_.setting.roi.height
        || para.setting.roi.x != device_para_.setting.roi.x || para.setting.roi.y != device_para_.setting.roi.y) {
        //        is_stop_grab = true;
        //        device_->StopCapture();

        device_->setIntValue("Width", static_cast<int>(para.setting.roi.width));
        device_->setIntValue("Height", static_cast<int>(para.setting.roi.height));
        device_->setIntValue("OffsetX", static_cast<int>(para.setting.roi.x));
        device_->setIntValue("OffsetY", static_cast<int>(para.setting.roi.y));
    }
    if (para.setting.mirror[0] != device_para_.setting.mirror[0]) {
        if (para.setting.mirror[0] == Mirror::HORIZONTAL_MIRROR_ON) device_->setBoolValue("ReverseX", true);
        else if (para.setting.mirror[0] == Mirror::HORIZONTAL_MIRROR_OFF) device_->setBoolValue("ReverseX", false);
    }
    if (para.setting.mirror[1] != device_para_.setting.mirror[1]) {
        if (para.setting.mirror[1] == Mirror::VERTICAL_MIRROR_ON) device_->setBoolValue("ReverseY", true);
        else if (para.setting.mirror[1] == Mirror::VERTICAL_MIRROR_OFF) device_->setBoolValue("ReverseY", false);
    }
    if (para.setting.exposure_auto != device_para_.setting.exposure_auto) {
        device_->setEnumValue("ExposureAuto", static_cast<unsigned int>(para.setting.exposure_auto));
    }
    if (para.setting.exposure_auto == Exposure::EXPOSURE_OFF) {
        if (para.setting.exposure != device_para_.setting.exposure)
            device_->setFloatValue("ExposureTime", para.setting.exposure);
    }

    if (para.setting.exposure_limit.min != device_para_.setting.exposure_limit.min
        || para.setting.exposure_limit.max != device_para_.setting.exposure_limit.max) {
        device_->setIntValue("AutoExposureTimeLowerLimit", para.setting.exposure_limit.min);
        device_->setIntValue("AutoExposureTimeUpperLimit", para.setting.exposure_limit.max);
    }
    if (para.setting.gain != device_para_.setting.gain) { device_->setFloatValue("Gain", para.setting.gain); }
    if (para.setting.gain_mode != device_para_.setting.gain_mode) {
        device_->setEnumValue("GainAuto", static_cast<unsigned int>(para.setting.gain_mode));
    }

    if (para.setting.gain_limit.min != device_para_.setting.gain_limit.min
        || para.setting.gain_limit.max != device_para_.setting.gain_limit.max) {
        device_->setFloatValue("AutoGainLowerLimit", para.setting.gain_limit.min);
        device_->setFloatValue("AutoGainUpperLimit", para.setting.gain_limit.max);
    }

    if (device_para_.vendor_name == "Basler") {
        if (para.setting.brightness != device_para_.setting.brightness)
            device_->setFloatValue("AutoTargetBrightness", para.setting.brightness);

        if (para.setting.fuctionprofile != device_para_.setting.fuctionprofile) {
            if (para.setting.fuctionprofile == AutoFunctionProfile::MinimizeGain) {
                device_->setEnumValue("AutoFunctionProfile",
                                      static_cast<unsigned int>(AutoFunctionProfile::MinimizeGain));
            }
            else if (para.setting.fuctionprofile == AutoFunctionProfile::MinimizeExposureTime)
                device_->setEnumValue("MinimizeExposureTime",
                                      static_cast<unsigned int>(AutoFunctionProfile::MinimizeExposureTime));
        }
    }

    if (para.setting.user_set_save) {
        para.setting.user_set_save = false;

        //        if (!is_stop_grab) {
        //            is_stop_grab = true;
        //            device_->StopCapture();
        //        }

        device_->setEnumValue("UserSetSelector", static_cast<unsigned int>(UserSelect::USERSELECT_USERSET1));
        device_->setEnumValue("UserSetDefault", static_cast<unsigned int>(UserSelect::USERSELECT_USERSET1));
        device_->setBoolValue("UserSetSave", true);
    }

    char file_str[256] = {0};
    sprintf(file_str, "%s/visual_stream%d.yaml", CAMERA_FILEPATH, device_para_.index);
    ReadCameraPara();
    WriteCameraStreamPara(file_str, device_para_);

    //    if (is_stop_grab) device_->StartCapture();
    device_->StartCapture();
}

void VisualStream::WriteCameraParaFromFile()
{
    char file_str[256] = {0};

    sprintf(file_str, "%s/visual_stream%d.yaml", CAMERA_FILEPATH, device_para_.index);
    if (!exists(std::experimental::filesystem::path(file_str))) {
        copy(std::experimental::filesystem::path(DEFAULT_FILENAME), std::experimental::filesystem::path(file_str));
    }
    ReadCameraStreamPara(file_str, device_para_);

    if (device_para_.setting.enable) {
        device_para_.setting.enable = !device_para_.setting.enable;
        device_->setIntValue("Width", static_cast<int>(device_para_.setting.roi.width));
        device_->setIntValue("Height", static_cast<int>(device_para_.setting.roi.height));
        device_->setIntValue("OffsetX", static_cast<int>(device_para_.setting.roi.x));
        device_->setIntValue("OffsetY", static_cast<int>(device_para_.setting.roi.y));
        if (device_para_.setting.mirror[0] == Mirror::HORIZONTAL_MIRROR_ON) device_->setBoolValue("ReverseX", true);
        else if (device_para_.setting.mirror[0] == Mirror::HORIZONTAL_MIRROR_OFF)
            device_->setBoolValue("ReverseX", false);
        if (device_para_.setting.mirror[1] == Mirror::VERTICAL_MIRROR_ON) device_->setBoolValue("ReverseY", true);
        else if (device_para_.setting.mirror[1] == Mirror::VERTICAL_MIRROR_OFF)
            device_->setBoolValue("ReverseY", false);
        device_->setEnumValue("TriggerMode", static_cast<unsigned int>(device_para_.setting.trigger_mode));
        if (device_para_.setting.trigger_mode == Trigger::TRIGGER_ON)
            device_->setEnumValue("TriggerSource", static_cast<unsigned int>(device_para_.setting.trigger_source));
        device_->setEnumValue("ExposureAuto", static_cast<unsigned int>(device_para_.setting.exposure_auto));
        if (device_para_.setting.exposure_auto != Exposure::EXPOSURE_OFF) {
            device_->setIntValue("AutoExposureTimeLowerLimit", device_para_.setting.exposure_limit.min);
            device_->setIntValue("AutoExposureTimeUpperLimit", device_para_.setting.exposure_limit.max);
        }
        else device_->setFloatValue("ExposureTime", device_para_.setting.exposure);
        device_->setEnumValue("GainAuto", static_cast<unsigned int>(device_para_.setting.gain_mode));
        if (device_para_.setting.gain_mode != Gain::GAIN_OFF) {
            device_->setFloatValue("AutoGainLowerLimit", device_para_.setting.gain_limit.min);
            device_->setFloatValue("AutoGainUpperLimit", device_para_.setting.gain_limit.max);
        }
        else device_->setFloatValue("Gain", device_para_.setting.gain);
        device_->setEnumValue("BalanceWhiteAuto", static_cast<unsigned int>(device_para_.setting.white_balance_auto));
        device_->setEnumValue("UserSetSelector", static_cast<unsigned int>(UserSelect::USERSELECT_USERSET1));
        device_->setEnumValue("UserSetDefault", static_cast<unsigned int>(UserSelect::USERSELECT_USERSET1));
        device_->setBoolValue("UserSetSave", true);
    }

    ReadCameraPara();
    stream_para_.size = {static_cast<unsigned int>(device_para_.setting.roi.width),
                         static_cast<unsigned int>(device_para_.setting.roi.height)};
    WriteCameraStreamPara(file_str, device_para_);
}

VisualStream::CameraPara VisualStream::ReadCameraPara()
{
    bool bool_value;
    int int_value;

    device_->getStringValue("DeviceVendorName", device_para_.vendor_name);
    device_->getStringValue("DeviceModelName", device_para_.model_name);

    device_->getStringValue("DeviceFirmwareVersion", device_para_.firmware_version);
    device_->getStringValue("DeviceSerialNumber", device_para_.serial_number);
    device_->getFloatValue("DeviceTemperature", device_para_.temperature);
    device_->getIntValue("Width", int_value);
    device_para_.setting.roi.width = int_value;
    device_->getIntValue("Height", int_value);
    device_para_.setting.roi.height = int_value;
    device_->getIntValue("OffsetX", int_value);
    device_para_.setting.roi.x = int_value;
    device_->getIntValue("OffsetY", int_value);
    device_para_.setting.roi.y = int_value;
    device_->getBoolValue("ReverseX", bool_value);
    if (bool_value) device_para_.setting.mirror[0] = Mirror::HORIZONTAL_MIRROR_ON;
    else device_para_.setting.mirror[0] = Mirror::HORIZONTAL_MIRROR_OFF;
    device_->getBoolValue("ReverseY", bool_value);
    if (bool_value) device_para_.setting.mirror[1] = Mirror::VERTICAL_MIRROR_ON;
    else device_para_.setting.mirror[1] = Mirror::VERTICAL_MIRROR_OFF;
    device_->getEnumValue("TriggerMode", int_value);
    device_para_.setting.trigger_mode = static_cast<Trigger>(int_value);
    device_->getEnumValue("TriggerSource", int_value);
    device_para_.setting.trigger_source = static_cast<TriggerSource>(int_value);
    device_->getEnumValue("ExposureAuto", int_value);
    device_->getFloatValue("ExposureTime", device_para_.setting.exposure);
    device_para_.setting.exposure_auto = static_cast<Exposure>(int_value);
    device_->getIntValue("AutoExposureTimeLowerLimit", device_para_.setting.exposure_limit.min);
    device_->getIntValue("AutoExposureTimeUpperLimit", device_para_.setting.exposure_limit.max);
    device_->getFloatValue("Gain", device_para_.setting.gain);
    if (device_para_.vendor_name != "I-Tek") {
        device_->getEnumValue("GainAuto", int_value);
        device_para_.setting.gain_mode = static_cast<Gain>(int_value);
        device_->getFloatValue("AutoGainLowerLimit", device_para_.setting.gain_limit.min);
        device_->getFloatValue("AutoGainUpperLimit", device_para_.setting.gain_limit.max);
        device_->getEnumValue("BalanceWhiteAuto", int_value);
        device_para_.setting.white_balance_auto = static_cast<WhiteBalance>(int_value);
    }

    if (device_para_.vendor_name == "Basler") {
        device_->getFloatValue("AutoTargetBrightness", device_para_.setting.brightness);
        device_->getEnumValue("AutoFunctionProfile", int_value);
        device_para_.setting.fuctionprofile = static_cast<AutoFunctionProfile>(int_value);
    }

    return device_para_;
}
