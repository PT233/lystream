#pragma once

extern "C"
{
#include "libavcodec/avcodec.h"

#include <libavformat/avformat.h>
}

class FFmpegVideoDecoder
{
  public:

    FFmpegVideoDecoder()          = default;
    virtual ~FFmpegVideoDecoder() = default;

    bool open();
    void close();

    bool naluDecode(unsigned char *pNaluDat, int naluSize, unsigned char *outdata[3]);

  private:

    AVCodec *m_codec {};                       // 编解码CODEC
    AVCodecContext *m_codecCtx {};             // 编解码CODEC context
    AVCodecParserContext *m_codecParserCtx {}; // 解析器上下文
    AVPacket *m_pkt {};                        // 存放解码前的h264数据
    AVFrame *m_frame {};                       // 解码后的图像
    bool m_bFirstIdrReady {};                  // 保证首帧为idr帧

    //    FILE *m_fpstream {}; // 测试，保存视频流
};