#include "ffmpeg_video_decoder.h"

#include <cassert>
#include <cstdio>
#include <unistd.h>

/**
 * @brief: 检测内存块第一个nalu类型
 * @return nal_type on seccess
 *   return -1 on error
 *   5: idr帧
 *   7: sps帧（sps nalu后面一般连着pps nalu和idr nalu，
 *    一次回调产生3个nalu，但是stream_test目前只能检测内存块的第一个nalu）
 *   1: 非idr帧
 */
int nalu_test(const unsigned char *dat, int len)
{
    unsigned char nal_header = 0;

    if (len < 5) {
        // printf("unexpected stream length: %d\n", len);
        return -1;
    }

    if (dat[0] == 0 && dat[1] == 0) {
        if (dat[2] == 0) {
            if (dat[3] == 1) {
                nal_header = dat[4];
                return nal_header & 0x1f;
            }
            else {}
        }
        else if (dat[2] == 1) {
            nal_header = dat[3];
            return nal_header & 0x1f;
        }
        else {}
    }
    else {}

    printf("unknown stream format, start_code: [%02x %02x %02x %02x %02x]\n", dat[0], dat[1], dat[2], dat[3], dat[4]);
    return -1;
}

void ffmpeg_print_error(int ret)
{
    char buf[128];
    av_strerror(ret, buf, 128);
    printf("%s\n", buf);
}

/**
 * @brief: 初始化解码器
 * @return true
 */
bool FFmpegVideoDecoder::open()
{
    //    av_register_all();
    avcodec_register_all();

    // 创建AVPacket
    m_pkt = av_packet_alloc();

    // 查找 H264 CODEC
    m_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!m_codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
    // m_codec = avcodec_find_decoder(AV_CODEC_ID_MPEG1VIDEO);

    // 初始化解析器上下文
    m_codecParserCtx = av_parser_init(m_codec->id);
    if (!m_codecParserCtx) {
        fprintf(stderr, "Parser not found\n");
        exit(1);
    }

    // 初始化CODEC的默认参数
    m_codecCtx = avcodec_alloc_context3(m_codec);
    if (!m_codecCtx) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        exit(1);
    }

    // 1. 打开CODEC，这里初始化H.264解码器，调用decode_init本地函数
    if (avcodec_open2(m_codecCtx, m_codec, nullptr) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    //    m_codecCtx->time_base.num = 1;
    //    m_codecCtx->frame_number = 1; //每包一个视频帧
    //    m_codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    //    m_codecCtx->bit_rate = 0;
    //    m_codecCtx->time_base.den = 50;  // 帧率
    //    m_codecCtx->width         = 384; // 视频宽
    //    m_codecCtx->height        = 288; // 视频高

    // 为AVFrame申请空间，并清零
    // 这个只分配结构体的内存，并不分配实际图像数据的内存
    if (!(m_frame = av_frame_alloc())) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    m_bFirstIdrReady = false;

    //    m_fpstream = fopen("./stream.h264", "wb");
    //    assert(m_fpstream);

    // printf("init ok\n");
    return true;
}

void FFmpegVideoDecoder::close()
{
    av_parser_close(m_codecParserCtx);
    avcodec_free_context(&m_codecCtx);
    av_frame_free(&m_frame);
    av_packet_free(&m_pkt);
}

/**
 * @brief: 处理nalu，送解码器，同时等待解码结束，取图像回调
 * @note:
 *   不能使用av_parser_parse2去处理内存块，因为这个函数只使用于文件形式的h264流，
 *   因为当其读到pNaluDat结尾时（naluSize=0）时，会设置上下文中的EOF标准，会导致后面的操作失败
 */
bool FFmpegVideoDecoder::naluDecode(unsigned char *pNaluDat, int naluSize, unsigned char *outdata[3])
{
    int ret;

    // 保证首帧为IDR帧
    if (!m_bFirstIdrReady) {
        int nal_type = nalu_test(pNaluDat, naluSize);
        if (nal_type != 5 && nal_type != 7) {
            printf("not key frame <nal_type=%d>\n", nal_type);
            return false;
        }
        else {
            printf("key frame ready...<nal_type=%d>\n", nal_type);
            m_bFirstIdrReady = true;
        }
    }
    //    fwrite(pNaluDat, naluSize, 1, m_fpstream);

    // 这个方式可行，但是必须保证从sps或者i帧开始送
    // 否则发送会失败
    //[h264 @ 0x55bc6fdfd0] non-existing PPS 0 referenced
    //[h264 @ 0x55bc6fdfd0] decode_slice_header error
    //[h264 @ 0x55bc6fdfd0] no frame!
    // avcodec_send_packet failed, -1094995529
    // { ERROR_TAG(INVALIDDATA), "Invalid data found when processing input" },
    m_pkt->data = pNaluDat;
    m_pkt->size = naluSize;

    ret = avcodec_send_packet(m_codecCtx, m_pkt);
    if (ret == AVERROR(EAGAIN)) {
        fprintf(stderr, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
    }
    else if (ret < 0) {
        printf("avcodec_send_packet failed, %d\n", ret);
        return false;
    }

    while (true) {
        // 这个函数执行真正的解码工作，可能较耗时，阻塞
        // 这里面会为m_frame分配实际的内存
        ret = avcodec_receive_frame(m_codecCtx, m_frame);
        if (ret != 0) {
            if (ret < 0) {
                if (ret != AVERROR(EAGAIN)) {
                    printf("avcodec_receive_frame failed, %d\n", ret);
                    ffmpeg_print_error(ret);
                }

                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) { return false; }
            }
        }
        else break;
    }

    for (int i = 0; i < 3; i++) {
        if (i == 0) { memcpy(outdata[i], m_frame->data[i], m_frame->linesize[i] * m_frame->height); }
        else {
            memcpy(outdata[i], m_frame->data[i], m_frame->linesize[i] * m_frame->height >> 1);
        }
    }

    return true;
}