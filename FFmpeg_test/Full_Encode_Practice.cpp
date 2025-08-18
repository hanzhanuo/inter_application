/*
 * 本示例演示了如何使用FFmpeg库进行视频编码，将原始YUV数据编码为H.264（或其他支持的编码格式），并写入到文件。
 * 
 * 主要流程总结如下：
 * 1. 查找并获取指定名称的编码器（如libx264）。
 * 2. 分配编码器上下文（AVCodecContext）和数据包（AVPacket）等必要结构体。
 * 3. 配置编码器参数（如分辨率、码率、帧率、像素格式等）。
 * 4. 打开编码器。
 * 5. 分配并初始化帧（AVFrame）及其缓冲区。
 * 6. 循环生成原始帧数据，送入编码器进行编码，并将编码后的数据写入文件。
 * 7. 刷新编码器，确保所有数据都被输出。
 * 8. 释放所有资源，关闭文件。
 */

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

/**
 * @brief 编码一帧数据并写入文件
 * 
 * @param encCtx 编码器上下文
 * @param frame  输入的原始帧（可为nullptr用于刷新编码器）
 * @param pkt    用于存储编码后数据的AVPacket
 * @param outFile 输出文件指针
 */
static void encode(AVCodecContext * encCtx, AVFrame * frame, AVPacket *pkt, FILE * outFile)
{

    
    printf("===== encode =====\n");
    int ret;
    if(frame) {
        printf("Send frame %lld\n", frame->pts);
    }

    //获取一个原始数据包frame后，交给编码器进行编码
    ret = avcodec_send_frame(encCtx, frame);
    if(ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }
    // 可能一个输入帧会产生多个输出包，因此用while循环接收
    while(ret >= 0) {
        // 从编码器接收编码后的数据包
        ret = avcodec_receive_packet(encCtx, pkt);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return; // 暂时没有可用数据或编码结束
        else if(ret < 0) {
            fprintf(stderr, "Error during encoding.\n");
            exit(1);
        }

        // 将编码后的数据写入文件
        printf("Write packet %lld (size=%5d)\n", pkt->pts, pkt->size);
        fwrite(pkt->data, 1, pkt->size, outFile);

        // 释放packet中的数据，准备下一个
        av_packet_unref(pkt);
    }
}

int main(int argc, char *argv[])
{
    // 1. 查找编码器
    AVCodecContext * codecCtx = nullptr;
    const  AVCodec * codec = nullptr;
    uint8_t endcode[] = {0,0,1,0xb7}; // MPEG1/2视频流结束码

    const char * codecName = "libx264";//可以更换为mpeg1video
    //获取编码器
    codec = avcodec_find_encoder_by_name(codecName);
    if(!codec) {
        fprintf(stderr, "Codec '%s' not found\n", codecName);
        exit(1);
    }

    // 2. 分配AVPacket
    AVPacket* pkt = av_packet_alloc();
    if(!pkt) {
        exit(1);
    }

    // 3. 分配并配置编码器上下文
    codecCtx = avcodec_alloc_context3(codec);
    if(!codecCtx) {
        fprintf(stderr, "Could not allocate AVCodecCtx\n");
        exit(1);
    }
    codecCtx->bit_rate = 400000; // 码率
    codecCtx->width = 640;       // 宽度
    codecCtx->height = 480;      // 高度
    codecCtx->time_base = (AVRational){1, 25}; // 时间基
    codecCtx->framerate = (AVRational){25, 1}; // 帧率
    codecCtx->gop_size = 10;     // 每10帧一个关键帧
    codecCtx->max_b_frames = 1;  // 最大B帧数
    codecCtx->pix_fmt = AV_PIX_FMT_YUV420P; // 像素格式

    // H264编码器可设置preset参数
    if(codec->id == AV_CODEC_ID_H264)
        av_opt_set(codecCtx->priv_data, "preset", "slow", 0);

    // 4. 打开编码器
    int ret = avcodec_open2(codecCtx, codec, nullptr);
    if(ret < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    // 5. 打开输出文件
    const char * outFileName = "testencode.mp4";
    FILE * fp = fopen(outFileName, "wb");
    if(!fp) {
        fprintf(stderr, "Could not open %s\n", outFileName);
        exit(1);
    }

    // 6. 分配并初始化帧
    AVFrame * frame = av_frame_alloc();
    if(!frame) {
        fprintf(stderr, "Could not allocate AVFrame\n");
        exit(1);
    }
    frame->format = codecCtx->pix_fmt;
    frame->width = codecCtx->width;
    frame->height = codecCtx->height;

    // 为帧分配实际数据缓冲区
    ret = av_frame_get_buffer(frame, 0);
    if(ret < 0) {
        fprintf(stderr, "Could not allocate buffer of AVFrame\n");
        exit(1);
    }

    int i, x, y;
    // 7. 生成并编码25帧（1秒视频）
    for (i = 0; i < 25; i++) {
        fflush(stdout);

        // 确保帧数据可写
        ret = av_frame_make_writable(frame);
        if (ret < 0)
            exit(1);

        // 填充Y分量（亮度）
        for (y = 0; y < codecCtx->height; y++) {
            for (x = 0; x < codecCtx->width; x++) {
                frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
            }
        }
        // 填充Cb和Cr分量（色度）
        for (y = 0; y < codecCtx->height/2; y++) {
            for (x = 0; x < codecCtx->width/2; x++) {
                frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
                frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
            }
        }

        frame->pts = i; // 设置帧的显示时间戳

        // 编码并写入文件
        encode(codecCtx, frame, pkt, fp);
    }

    // 8. 刷新编码器，确保所有数据输出
    encode(codecCtx, nullptr, pkt, fp);

    // 9. 对于MPEG1/2，写入结束码
    if(codec->id == AV_CODEC_ID_MPEG1VIDEO ||
       codec->id == AV_CODEC_ID_MPEG2VIDEO) {
        printf(" ==== write end code\n");
        fwrite(endcode, 1, sizeof(endcode), fp);
    }

    // 10. 释放资源
    fclose(fp);
    avcodec_free_context(&codecCtx);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    printf("===encode test end ===\n");

    return 0;
}
