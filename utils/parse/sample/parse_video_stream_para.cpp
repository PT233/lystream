/**
 * @brief 配置文件测试
 * @date 2022/11/28
 * @author laoyao
 */

#include "utils/parse//parse_config.h"

using namespace lystream;

int main()
{
    VideoStream::VideoPara para;
    ParseVideoStreamPara("/root/lystream/config/video_stream.yaml", para);

    cout << "encode type : " << para.encoder_type << endl;
    cout << "fps : " << para.fps << endl;
    cout << "bitrate : " << para.bitrate << endl;
    cout << "ratemode : " << para.ratemode << endl;
    cout << "profile : " << para.profile << endl;
    cout << "levle : " << para.level << endl;
    cout << "iframe interval : " << para.ifrmae_interval << endl;
    cout << "idr interval : " << para.idr_interval << endl;
    cout << boolalpha << "insert aud : " << para.insert_aud << endl;
    cout << "all iframe : " << para.all_iframe << endl;
    cout << "max perf : " << para.max_permode << endl;

    return 0;
}