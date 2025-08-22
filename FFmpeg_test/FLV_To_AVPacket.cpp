#include <stdio.h>
#include <iostream>
#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/file.h>
}

using std::string;
using std::cout;
using std::endl;

void flvToAVPacket(const string &flvPath) {
    AVFormatContext *fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, flvPath.c_str(), nullptr, nullptr) < 0) {
        cout << "无法打开FLV文件" << endl;
        return;
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        cout << "无法获取流信息" << endl;
        avformat_close_input(&fmt_ctx);
        return;
    }


    int video_stream_idx = -1;
    int audio_stream_idx = -1;
    for (int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        video_stream_idx = i;
        } else if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        audio_stream_idx = i;
        }
    }

    AVPacket *packet = av_packet_alloc();
    while (av_read_frame(fmt_ctx, packet) >= 0) {
        AVStream *stream = fmt_ctx->streams[packet->stream_index];
        double pts_sec = packet->pts * av_q2d(stream->time_base);  // 转换为秒

        if (packet->stream_index == video_stream_idx) {
            cout  << "视频包 | PTS:" << pts_sec << "s | 大小:" << packet->size;
        } else if (packet->stream_index == audio_stream_idx) {
            cout  << "音频包 | PTS:" << pts_sec << "s | 大小:" << packet->size;
        }

        av_packet_unref(packet);
    }

    av_packet_free(&packet);
    avformat_close_input(&fmt_ctx);
}


int main(){

    flvToAVPacket("../video_20250820_093123.flv");
    
    return 0;
}