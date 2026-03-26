/**
 * @brief 解析配置通用接口
 * @date 2022/11/28
 * @author laoyao
 */

#include "parse_config.h"

#include <boost/format.hpp>
#include <fstream>
#include <yaml-cpp/yaml.h>

using namespace lystream;

void ParseVideoStreamPara(const string &filepath, VideoStream::VideoPara &para)
{
    YAML::Node config = YAML::LoadFile(filepath);

    para.encoder_type = config["VideoEncodePara"]["type"].as<string>();
    para.fps          = config["VideoEncodePara"]["fps"].as<unsigned int>();
    para.bitrate      = config["VideoEncodePara"]["bitrate"].as<unsigned int>();
    para.ratemode     = config["VideoEncodePara"]["ratemode"].as<string>();
    if (para.encoder_type == "H264") {
        para.profile = config["VideoEncodePara"]["H264Para"]["profile"].as<string>();
        para.level   = config["VideoEncodePara"]["H264Para"]["level"].as<string>();
    }
    else if (para.encoder_type == "H265") {
        para.profile = config["VideoEncodePara"]["H265Para"]["profile"].as<string>();
        para.level   = config["VideoEncodePara"]["H265Para"]["level"].as<string>();
    }
    para.ifrmae_interval = config["VideoEncodePara"]["iframeinterval"].as<unsigned int>();
    para.idr_interval    = config["VideoEncodePara"]["idrinterval"].as<unsigned int>();
    para.all_iframe      = config["VideoEncodePara"]["alliframe"].as<bool>();
    para.insert_aud      = config["VideoEncodePara"]["insertaud"].as<bool>();
    para.max_permode     = config["VideoEncodePara"]["maxperf"].as<bool>();
}

void ParseJpegStreamPara(const string &filepath, jpegstream::JpegPara &para)
{
    YAML::Node config = YAML::LoadFile(filepath);

    para.level        = config["JpegEncodePara"]["level"].as<unsigned int>();
    para.scale.width  = config["JpegEncodePara"]["scale"]["width"].as<unsigned int>();
    para.scale.height = config["JpegEncodePara"]["scale"]["height"].as<unsigned int>();
    para.roi.x        = config["JpegEncodePara"]["roi"]["offset_x"].as<unsigned int>();
    para.roi.y        = config["JpegEncodePara"]["roi"]["offset_y"].as<unsigned int>();
    para.roi.width    = config["JpegEncodePara"]["roi"]["offset_width"].as<unsigned int>();
    para.roi.height   = config["JpegEncodePara"]["roi"]["offset_height"].as<unsigned int>();
}

void ReadCameraStreamPara(const string &filepath, VisualStream::CameraPara &para)
{
    YAML::Node config = YAML::LoadFile(filepath);

    para.setting.enable   = config["CameraSettingPara"]["Enable"].as<bool>();
    para.vendor_name      = config["CameraSettingPara"]["DeviceVendorName"].as<string>();
    para.model_name       = config["CameraSettingPara"]["DeviceModelName"].as<string>();
    para.firmware_version = config["CameraSettingPara"]["DeviceFirmwareVersion"].as<string>();
    para.serial_number    = config["CameraSettingPara"]["DeviceSerialNumber"].as<string>();
    if (config["CameraSettingPara"]["CameraType"].as<string>() == "Mono")
        para.camera_type = CameraColorType::Mono_Camera;
    else if (config["CameraSettingPara"]["CameraType"].as<string>() == "Color")
        para.camera_type = CameraColorType::Color_Camera;

    para.setting.roi.width  = config["CameraSettingPara"]["CameraRoi"][0].as<int>();
    para.setting.roi.height = config["CameraSettingPara"]["CameraRoi"][1].as<int>();
    para.setting.roi.x      = config["CameraSettingPara"]["CameraRoi"][2].as<int>();
    para.setting.roi.y      = config["CameraSettingPara"]["CameraRoi"][3].as<int>();

    if (config["CameraSettingPara"]["TriggerMode"].as<string>() == "Off")
        para.setting.trigger_mode = Trigger::TRIGGER_OFF;
    else if (config["CameraSettingPara"]["TriggerMode"].as<string>() == "On")
        para.setting.trigger_mode = Trigger::TRIGGER_ON;

    if (config["CameraSettingPara"]["TriggerSource"].as<string>() == "Software")
        para.setting.trigger_source = TriggerSource::TRIGGER_SOURCE_SOFTWARE;
    else if (config["CameraSettingPara"]["TriggerSource"].as<string>() == "Line0")
        para.setting.trigger_source = TriggerSource::TRIGGER_SOURCE_LINE0;
    else if (config["CameraSettingPara"]["TriggerSource"].as<string>() == "Line1")
        para.setting.trigger_source = TriggerSource::TRIGGER_SOURCE_LINE1;
    else if (config["CameraSettingPara"]["TriggerSource"].as<string>() == "Line2")
        para.setting.trigger_source = TriggerSource::TRIGGER_SOURCE_LINE2;

    if (config["CameraSettingPara"]["Mirror"][0].as<bool>()) para.setting.mirror[0] = Mirror::HORIZONTAL_MIRROR_ON;
    else para.setting.mirror[0] = Mirror::HORIZONTAL_MIRROR_OFF;
    if (config["CameraSettingPara"]["Mirror"][1].as<bool>()) para.setting.mirror[1] = Mirror::VERTICAL_MIRROR_ON;
    else para.setting.mirror[1] = Mirror::VERTICAL_MIRROR_OFF;

    para.setting.exposure = config["CameraSettingPara"]["ExposureTime"].as<float>();
    if (config["CameraSettingPara"]["ExposureAuto"].as<string>() == "Off")
        para.setting.exposure_auto = Exposure::EXPOSURE_OFF;
    else if (config["CameraSettingPara"]["ExposureAuto"].as<string>() == "Once")
        para.setting.exposure_auto = Exposure::EXPOSURE_ONCE;
    else if (config["CameraSettingPara"]["ExposureAuto"].as<string>() == "Continuous")
        para.setting.exposure_auto = Exposure::EXPOSURE_CONTINUOUS;

    para.setting.exposure_limit.min = config["CameraSettingPara"]["ExposureLimit"][0].as<int>();
    para.setting.exposure_limit.max = config["CameraSettingPara"]["ExposureLimit"][1].as<int>();

    para.setting.gain = config["CameraSettingPara"]["GainRegular"].as<float>();
    if (config["CameraSettingPara"]["GainAuto"].as<string>() == "Off") para.setting.gain_mode = Gain::GAIN_OFF;
    else if (config["CameraSettingPara"]["GainAuto"].as<string>() == "Once") para.setting.gain_mode = Gain::GAIN_ONCE;
    else if (config["CameraSettingPara"]["GainAuto"].as<string>() == "Continous")
        para.setting.gain_mode = Gain::GAIN_CONTINUOUS;

    para.setting.gain_limit.min = config["CameraSettingPara"]["GainLimit"][0].as<float>();
    para.setting.gain_limit.max = config["CameraSettingPara"]["GainLimit"][1].as<float>();

    if (config["CameraSettingPara"]["BalanceWhiteAuto"].as<string>() == "Off")
        para.setting.white_balance_auto = WhiteBalance::WHITE_BALANCE_OFF;
    else if (config["CameraSettingPara"]["BalanceWhiteAuto"].as<string>() == "Once")
        para.setting.white_balance_auto = WhiteBalance::WHITE_BALANCE_ONCE;
    else if (config["CameraSettingPara"]["BalanceWhiteAuto"].as<string>() == "Continuous")
        para.setting.white_balance_auto = WhiteBalance::WHITE_BALANCE_CONTINUOUS;
}

#define SED_WRITE(key, value, file) (boost::format(R"(sed -i 's/%1%.*/%1% : %2%/g' %3%)") % key % value % file)

void WriteCameraStreamPara(const string &filepath, VisualStream::CameraPara &para)
{
    if (para.setting.enable) system(SED_WRITE("Enable", "true", filepath).str().c_str());
    else system(SED_WRITE("Enable", "false", filepath).str().c_str());

    system(SED_WRITE("DeviceVendorName", para.vendor_name, filepath).str().c_str());
    system(SED_WRITE("DeviceVendorName", para.model_name, filepath).str().c_str());
    system(SED_WRITE("DeviceFirmwareVersion", para.firmware_version, filepath).str().c_str());
    system(SED_WRITE("DeviceSerialNumber", para.serial_number, filepath).str().c_str());

    if (para.camera_type == CameraColorType::Mono_Camera)
        system(SED_WRITE("CameraType", "Mono", filepath).str().c_str());
    else if (para.camera_type == CameraColorType::Color_Camera)
        system(SED_WRITE("CameraType", "Color", filepath).str().c_str());

    string temp_str;

    temp_str = (boost::format("[ %1%, %2%, %3%, %4% ]") % para.setting.roi.width % para.setting.roi.height
                % para.setting.roi.x % para.setting.roi.y)
                   .str();

    system(SED_WRITE("CameraRoi", temp_str.c_str(), filepath).str().c_str());

    temp_str
        = (boost::format("[ %1%, %2% ]") % ((para.setting.mirror[0] == Mirror::HORIZONTAL_MIRROR_ON) ? "true" : "false")
           % (para.setting.mirror[1] == Mirror::VERTICAL_MIRROR_ON ? "true" : "false"))
              .str();

    system(SED_WRITE("Mirror", temp_str.c_str(), filepath).str().c_str());

    system(SED_WRITE("ExposureTime", para.setting.exposure, filepath).str().c_str());
    if (para.setting.exposure_auto == Exposure::EXPOSURE_OFF)
        system(SED_WRITE("ExposureAuto", "Off", filepath).str().c_str());
    else if (para.setting.exposure_auto == Exposure::EXPOSURE_ONCE)
        system(SED_WRITE("ExposureAuto", "Once", filepath).str().c_str());
    else if (para.setting.exposure_auto == Exposure::EXPOSURE_CONTINUOUS)
        system(SED_WRITE("ExposureAuto", "Continuous", filepath).str().c_str());

    temp_str
        = (boost::format("[ %1%, %2% ]") % para.setting.exposure_limit.min % para.setting.exposure_limit.max).str();
    system(SED_WRITE("ExposureLimit", temp_str.c_str(), filepath).str().c_str());

    system(SED_WRITE("GainRegular", para.setting.gain, filepath).str().c_str());

    if (para.setting.gain_mode == Gain::GAIN_OFF) system(SED_WRITE("GainAuto", "Off", filepath).str().c_str());
    else if (para.setting.gain_mode == Gain::GAIN_ONCE) system(SED_WRITE("GainAuto", "Once", filepath).str().c_str());
    else if (para.setting.gain_mode == Gain::GAIN_CONTINUOUS)
        system(SED_WRITE("GainAuto", "Continuous", filepath).str().c_str());

    temp_str = (boost::format("[ %1%, %2% ]") % para.setting.gain_limit.min % para.setting.gain_limit.max).str();
    system(SED_WRITE("GainLimit", temp_str.c_str(), filepath).str().c_str());

    if (para.setting.trigger_mode == Trigger::TRIGGER_ON)
        system(SED_WRITE("TriggerMode", "On", filepath).str().c_str());
    else if (para.setting.trigger_mode == Trigger::TRIGGER_OFF)
        system(SED_WRITE("TriggerMode", "Off", filepath).str().c_str());

    if (para.setting.trigger_source == TriggerSource::TRIGGER_SOURCE_SOFTWARE)
        system(SED_WRITE("TriggerSource", "Software", filepath).str().c_str());
    else if (para.setting.trigger_source == TriggerSource::TRIGGER_SOURCE_LINE0)
        system(SED_WRITE("TriggerSource", "Line0", filepath).str().c_str());
    else if (para.setting.trigger_source == TriggerSource::TRIGGER_SOURCE_LINE1)
        system(SED_WRITE("TriggerSource", "Line1", filepath).str().c_str());
    else if (para.setting.trigger_source == TriggerSource::TRIGGER_SOURCE_LINE2)
        system(SED_WRITE("TriggerSource", "Line2", filepath).str().c_str());

    if (para.setting.white_balance_auto == WhiteBalance::WHITE_BALANCE_OFF)
        system(SED_WRITE("BalanceWhiteAuto", "Off", filepath).str().c_str());
    else if (para.setting.white_balance_auto == WhiteBalance::WHITE_BALANCE_ONCE)
        system(SED_WRITE("BalanceWhiteAuto", "Once", filepath).str().c_str());
    else if (para.setting.white_balance_auto == WhiteBalance::WHITE_BALANCE_CONTINUOUS)
        system(SED_WRITE("BalanceWhiteAuto", "Continuous", filepath).str().c_str());
}