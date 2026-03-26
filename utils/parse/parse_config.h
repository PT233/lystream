/**
 * @brief 解析配置通用接口
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include "jpeg_stream.h"
#include "video_stream.h"
#include "visual_stream.h"

extern void ParseVideoStreamPara(const std::string &filepath, lystream::VideoStream::VideoPara &para);
extern void ParseJpegStreamPara(const std::string &filepath, lystream::jpegstream::JpegPara &para);
extern void ReadCameraStreamPara(const string &filepath, lystream::VisualStream::CameraPara &para);
extern void WriteCameraStreamPara(const string &filepath, lystream::VisualStream::CameraPara &para);