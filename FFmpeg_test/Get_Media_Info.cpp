extern "C"
{
#include <libavformat/avformat.h>
}

#include <iostream>
using std::cout;


/*
这个文件能实现对于获取的文件流的解封装操作


步骤：
1.分配解复用器上下文AVFormatContext

2.根据url打开本地文件或网络流avformat_open_input

3.读取媒体的部分数据包以获取码流信息avformat_find_stream_info

到了这个第三步就已经获取文件流信息成功了，剩下的就是对已经获取的文件流信息进行读取输出的操作了

4.循环读取码流信息

> a.从文件中读取数据包av_read_frame
>
> b.定位文件avformat_seek_file或av_seek_frame

5.关闭解复用器avformat_close_input
*/

static double r2d(AVRational r) {
    return r.den == 0 ? 0 : (double)r.num / (double) r.den;
}

int testDemuxer1()
{
    const char * fileName = "D:\\videos\\test.mp4";
    AVDictionary * opts = nullptr;
    AVFormatContext * pFmtCtx = nullptr;

    int ret = avformat_open_input(&pFmtCtx, fileName, nullptr, &opts);
    if(ret !=0) {
        char buf[1024] = {0};
        printf("open %s failed, %s\n", fileName, buf);
        return EXIT_FAILURE;
    }

    cout << "打开媒体文件成功";

    ret = avformat_find_stream_info(pFmtCtx, nullptr);
    if(ret < 0) {
        fprintf(stderr, "avformat_find_stream_info error\n");
        return EXIT_FAILURE;
    }

    cout << "媒体文件名字: " << pFmtCtx->filename;
    cout << "音视频流的个数:" << pFmtCtx->nb_streams;
    cout << "媒体文件的平均码率: " << pFmtCtx->bit_rate << "bps";
    cout << "媒体文件总时长: " << pFmtCtx->duration << "微秒";

    int tus = pFmtCtx->duration / AV_TIME_BASE;
    int thh = tus / 3600;
    int tmm = (tus % 3600) / 60;
    int tss = tus % 60;
    cout << "媒体文件总时长: "<< thh << "时" << tmm << "分" << tss << "秒";

    for(int i = 0; i < pFmtCtx->nb_streams; ++i) {
        AVStream * as = pFmtCtx->streams[i];
        if(AVMEDIA_TYPE_AUDIO == as->codecpar->codec_type) {
            //如果是音频流，则打印音频流的信息
            cout << "\n>> 音频信息:";
            cout << "index: " << as->index;
            cout << "音频采样率: " << as->codecpar->sample_rate << "Hz";
            cout << "音频声道数目: " << as->codecpar->channels;

            if(AV_SAMPLE_FMT_FLTP == as->codecpar->format) {
                cout << "音频采样格式: AV_SAMPLE_FMT_FLTP";
            } else if(AV_SAMPLE_FMT_S16P == as->codecpar->format) {
                cout << "音频采样格式： AV_SAMPLE_FMT_S16P";
            }

            if(AV_CODEC_ID_AAC == as->codecpar->codec_id) {
                cout << "音频压缩编码格式:AAC";
            } else if(AV_CODEC_ID_MP3 == as->codecpar->codec_id) {
                cout << "音频压缩编码格式:MP3";
            }

            cout << "as->duration:" << as->duration;//单位为毫秒
            cout<< as->time_base.num << "/" << as->time_base.den;
            cout << "r2d:" << r2d(as->time_base);
            int durationAudio = as->duration * r2d(as->time_base);
            int thh = durationAudio / 3600;
            int tmm = durationAudio % 3600 / 60;
            int tss = durationAudio % 60;
            cout << "音频总时长:" << thh << "时" << tmm << "分" << tss << "秒";
        } else if(AVMEDIA_TYPE_VIDEO == as->codecpar->codec_type) {
            cout << "\n>> 视频信息:";
            cout << "index: " << as->index;
            cout << "视频帧率:" << r2d(as->avg_frame_rate);
            cout << "视频分辨率:" << as->codecpar->width << "x" <<as->codecpar->height;
            //cout << "视频压缩编码格式:" << as->codecpar->codec_id;
            if(AV_CODEC_ID_H264 == as->codecpar->codec_id) {
                cout << "视频压缩编码格式:H264";
            } else if(AV_CODEC_ID_FLV1 == as->codecpar->codec_id) {
                cout << "视频压缩编码格式:FLV";
            }

            int durationVideoS = as->duration * r2d(as->time_base);
            cout << "视频总时长:" << durationVideoS / 3600 << "时"
                     << durationVideoS % 3600 / 60 << "分"
                     << durationVideoS % 60 << "秒";
        }
     }

    //退出程序时，回收资源
    if(pFmtCtx) {
        avformat_close_input(&pFmtCtx);
    }

    return 0;
}