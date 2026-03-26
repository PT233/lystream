/**
 * @brief 配置文件测试
 * @date 2022/11/28
 * @author laoyao
 */

#include "utils/parse/parse_config.h"

using namespace lystream;

int main()
{
    jpegstream::JpegPara para;
    ParseJpegStreamPara("/root/lystream/config/jpeg_stream.yaml", para);

    cout << "level : " << para.level << endl;
    cout << "scale width : " << para.scale.width << endl;
    cout << "scale height: " << para.scale.height << endl;
    cout << "roi offset_x : " << para.roi.x << endl;
    cout << "roi offset_y : " << para.roi.y << endl;
    cout << "roi offset_width : " << para.roi.width << endl;
    cout << "roi offset_height : " << para.roi.height << endl;

    return 0;
}