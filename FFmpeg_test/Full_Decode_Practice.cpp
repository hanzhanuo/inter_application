/*
 * 本文件详细演示了如何使用FFmpeg库对视频文件进行解复用（demuxing）和解码（decode），
 * 并将解码后的视频帧（YUV格式）和音频帧（PCM格式）分别保存到本地文件。
 * 
 * 主要流程如下：
 * 1. 打开输入媒体文件，获取媒体流信息。
 * 2. 分别查找视频流和音频流，打开对应的解码器。
 * 3. 为视频帧分配输出缓冲区，为音频帧分配输出文件。    // 这里是为视频帧分配YUV格式的缓冲区，为音频帧分配PCM格式的文件
 *
 * 4. 循环读取每一个packet，根据其类型送入对应解码器。  //也就是说到时候会有视频流和音频流不同的packet发过来是吗
 * 5. 解码后的视频帧写入YUV文件，音频帧写入PCM文件。
 * 
 * 6. 解码结束后，输出ffplay命令用于播放结果文件。    //所以上面都是在进行解码操作，直到到了这才是进行播放操作
 */


 /*
 进一步总结其中使用的函数，并且要能分析出来是哪一步使用了RTSP(即整个FFmpeg中，哪个是RTSP的核心部分)
 1. avformat_open_input: 打开输入媒体文件，获取格式上下文
 2. avformat_find_stream_info: 读取媒体流信息
 3. avcodec_find_decoder: 查找解码器
 4. avcodec_open2: 打开解码器
 5. av_read_frame: 读取每一个packet
 6. avcodec_send_packet: 发送packet到解码器
 7. avcodec_receive_frame: 接收解码后的视频帧
 8. av_image_alloc: 分配视频帧缓冲区
 9. av_frame_copy: 复制解码后的视频帧数据
 10. av_frame_free: 释放视频帧
 */


extern "C"
{
#include <libavformat/avformat.h>   // 媒体格式处理相关API
#include <libavcodec/avcodec.h>     // 编解码器相关API
#include <libavutil/opt.h>          // 工具函数
#include <libavutil/imgutils.h>     // 图像工具函数
}

// 输出文件指针，分别用于保存解码后的视频和音频数据
FILE * outVideoFp = nullptr, * outAudioFp = nullptr;

// 视频帧数据缓冲区及相关参数
static uint8_t *video_dst_data[4] = {NULL}; // 用于存储解码后的视频帧数据（最多4个平面）
static int      video_dst_linesize[4];      // 每个平面的步长（字节数）
static int video_dst_bufsize;               // 视频帧缓冲区总字节数
static int video_frame_count = 0;           // 已输出的视频帧计数
static int audio_frame_count = 0;           // 已输出的音频帧计数

// 视频像素格式
AVPixelFormat pix_fmt;

// 函数声明
// 打开指定类型（音频/视频）流的解码器，并返回解码器上下文
static int openCodecContext(int *stream_idx,
                              AVCodecContext **dec_ctx,
                              AVFormatContext *fmt_ctx,
                              enum AVMediaType type);

// 解码一个packet（音频或视频），并输出到文件
static int decodePacket(AVCodecContext *dec, const AVPacket *pkt);

// 输出一帧视频到YUV文件
static int outputVideoFrame(AVFrame *frame);

// 输出一帧音频到PCM文件
static int outputAudioFrame(AVFrame *frame);

// 根据音频采样格式获取ffplay命令行参数格式字符串
static int get_format_from_sample_fmt(const char **fmt,
                                      enum AVSampleFormat sample_fmt);

int main()
{
    int ret = 0;

    // 输入输出文件名
    const char * srcFilename = "D:\\videos\\lldq-test480.mp4"; // 输入视频文件路径
    const char * videoDstFilename = "lldq.yuv";                // 输出YUV文件名
    const char * audioDstFilename = "lldq.pcm";                // 输出PCM文件名

    // 打开输出视频文件（YUV）
    outVideoFp = fopen(videoDstFilename, "wb");
    if(!outVideoFp) {
        fprintf(stderr, "fopen file %s error\n", videoDstFilename);
        return EXIT_FAILURE;
    }

    // 打开输出音频文件（PCM）
    outAudioFp = fopen(audioDstFilename, "wb");
    if(!outAudioFp) {
        fprintf(stderr, "fopen file %s error\n", audioDstFilename);
        return EXIT_FAILURE;
    }

    // 打开输入媒体文件，获取格式上下文
    AVFormatContext  * inFmtCtx = nullptr;
    ret = avformat_open_input(&inFmtCtx, srcFilename, nullptr, nullptr);
    if(ret < 0) {
        fprintf(stderr, "Could not open source file %s\n", srcFilename);
        exit(1);
    }

    // 读取媒体流信息（如流数量、类型等）
    ret = avformat_find_stream_info(inFmtCtx, nullptr);
    if(ret < 0) {
        fprintf(stderr, "Could not find stream information.\n");
        exit(1);
    }

    int videoStreamIdx, audioStreamIdx; // 视频流和音频流的索引
    AVCodecContext * videoCodecCtx = nullptr, *audioCodecCtx = nullptr; // 解码器上下文
    AVStream * videoStream = nullptr, * audioStream = nullptr; // 流指针

    // 打开视频解码器
    if(openCodecContext(&videoStreamIdx, &videoCodecCtx, inFmtCtx, AVMEDIA_TYPE_VIDEO) >=0) {
        videoStream = inFmtCtx->streams[videoStreamIdx];

        int width = videoCodecCtx->width;     // 视频宽度
        int height = videoCodecCtx->height;   // 视频高度
        pix_fmt = videoCodecCtx->pix_fmt;     // 视频像素格式

        // 分配视频帧缓冲区，存储解码后的视频数据
        ret = av_image_alloc(video_dst_data, video_dst_linesize,
                             width, height, pix_fmt, 1);
        if(ret < 0) {
            fprintf(stderr, "Could not allocate raw video buffer\n");
            return EXIT_FAILURE;
        }
        video_dst_bufsize = ret; // 缓冲区总字节数
    }

    // 打开音频解码器
    if(openCodecContext(&audioStreamIdx, &audioCodecCtx, inFmtCtx, AVMEDIA_TYPE_AUDIO) >= 0) {
        audioStream = inFmtCtx->streams[audioStreamIdx];
    }

    // 打印输入流的详细媒体信息（如流类型、编解码器、分辨率、采样率等）
    av_dump_format(inFmtCtx, -1, srcFilename, 0);

    // 检查是否成功找到音视频流
    if(!videoStream && !audioStream) {
        fprintf(stderr, "Could not find audio or video stream in the input.\n");
        exit(1);
    }

    // 分配AVFrame结构体，用于存储解码后的帧数据
    AVFrame * frame = av_frame_alloc();
    if(!frame) {
        fprintf(stderr, "Could not allocate AVFrame\n");
        exit(1);
    }
    // 分配AVPacket结构体，用于存储读取到的压缩数据包
    AVPacket * pkt = av_packet_alloc();
    if(!pkt) {
        fprintf(stderr, "Could not allocate AVPacket\n");
        exit(1);
    }
    int count = 0; // 统计读取的packet数量

    // 主循环：不断读取packet，送入对应解码器
    while(av_read_frame(inFmtCtx, pkt) >= 0) {
        printf("count: %d\n", ++count);

        //所以这里的数组下标是对于packet进行的类型辨别，然后再进行对应的解码方法
        if(pkt->stream_index == videoStreamIdx)
            ret = decodePacket(videoCodecCtx, pkt); // 解码视频packet
        else if(pkt->stream_index == audioStreamIdx)
            ret = decodePacket(audioCodecCtx, pkt); // 解码音频packet

        av_packet_unref(pkt); // 释放packet内部数据，复用pkt
        if(ret < 0)
            break;
    }

    // 刷新解码器，处理缓冲区中剩余的数据（如B帧等）
    if(videoCodecCtx)
    {    decodePacket(videoCodecCtx, nullptr);  }
    if(audioCodecCtx)
    {    decodePacket(audioCodecCtx, nullptr);  }

    printf("====== Demuxing succeeded. ======\n");


    //下面这里的视频流和音频流是AVStream，然后是从AVStream结构体中获取出来的(所以是取的该结构体的数组下标)

    // 输出播放YUV文件的ffplay命令，便于用户直接播放解码后的视频
    if(videoStream) {
        printf(">> Play the output video file with the command:\n"
               "ffplay -f rawvideo -pix_fmt %s -video_size %dx%d %s\n",
               av_get_pix_fmt_name(pix_fmt),
               videoCodecCtx->width, videoCodecCtx->height, videoDstFilename);
    }

    // 输出播放PCM文件的ffplay命令，便于用户直接播放解码后的音频
    if(audioStream) {
        enum AVSampleFormat sfmt = audioCodecCtx->sample_fmt; // 音频采样格式
        int nChannels = audioCodecCtx->channels;              // 声道数
        const char * fmt;

        // 判断音频采样格式是否为planar（平面），如果是只输出第一个声道
        if(av_sample_fmt_is_planar(sfmt)) {
            const char * packed = av_get_sample_fmt_name(sfmt);
            printf("Warning: the sample format the decoder produced is planar "
                   "(%s). This example will output the first channel only.\n",
                   packed ? packed: "?");
            sfmt = av_get_packed_sample_fmt(sfmt); // 转为packed格式
            nChannels = 1; // 只输出一个声道
        }

        // 获取ffplay命令行参数格式
        ret = get_format_from_sample_fmt(&fmt, sfmt);
        if(ret < 0) {
            goto end;
        }

        printf(">>Play the output audio file with the command:\n"
               "ffplay -f %s -ac %d -ar %d %s\n",
               fmt, nChannels,
               audioCodecCtx->sample_rate,
               audioDstFilename);
    }

end:
    // 释放所有资源，防止内存泄漏
    avcodec_free_context(&videoCodecCtx); // 释放视频解码器上下文
    avcodec_free_context(&audioCodecCtx); // 释放音频解码器上下文
    avformat_close_input(&inFmtCtx);      // 关闭输入文件
    av_frame_free(&frame);                // 释放帧结构体
    av_free(video_dst_data[0]);           // 释放视频帧缓冲区
    return 0;
}

/*
 * 打开指定类型（音频/视频）流的解码器，并返回解码器上下文
 * 参数说明：
 *   stream_idx: 返回找到的流索引
 *   dec_ctx: 返回分配好的解码器上下文
 *   fmt_ctx: 输入的格式上下文
 *   type: 媒体类型（音频/视频）
 */
static int openCodecContext(int *stream_idx,
                              AVCodecContext **dec_ctx,
                              AVFormatContext *fmt_ctx,
                              enum AVMediaType type)
{
    int ret, index;
    AVStream * st;
    AVCodec * dec = nullptr;
    AVDictionary * opts = nullptr;

    // 查找最优的流（如第一个视频流或音频流）
    ret = av_find_best_stream(fmt_ctx, type, -1, -1, nullptr, 0);
    if(ret < 0) {
        fprintf(stderr, "Could not find %s stream\n",
                av_get_media_type_string(type));
        return ret;
    }

    index = *stream_idx = ret;
    st = fmt_ctx->streams[index]; // 获取对应的流指针

    // 查找解码器
    dec = avcodec_find_decoder(st->codecpar->codec_id);
    if(!dec){
        fprintf(stderr, "Failed to find %s codec\n",
                av_get_media_type_string(type));
        return AVERROR(EINVAL);
    }

    // 分配解码器上下文
    *dec_ctx = avcodec_alloc_context3(dec);
    if(!*dec_ctx) {
        fprintf(stderr, "Failed to allocate the %s codec context\n",
                av_get_media_type_string(type));
        return AVERROR(ENOMEM);
    }

    // 拷贝流参数到解码器上下文
    ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar);
    if(ret < 0) {
        fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                av_get_media_type_string(type));
        return ret;
    }

    // 打开解码器
    ret = avcodec_open2(*dec_ctx, dec, &opts);
    if(ret < 0) {
        fprintf(stderr, "Failed to open %s codec\n",
                av_get_media_type_string(type));
        return ret;
    }

    return 0;
}


//end of 打开解码器函数






/*
 * 解码一个packet（音频或视频），并输出到文件
 * 参数说明：
 *   dec: 解码器上下文
 *   pkt: 输入的压缩数据包（可为nullptr用于刷新解码器）
 */
static int decodePacket(AVCodecContext *dec, const AVPacket *pkt)
{
    // 将压缩数据送入解码器
    int ret = avcodec_send_packet(dec, pkt);
    if(ret < 0) {
        fprintf(stderr, "Error ocurred when avcodec_send_packet\n");
        return ret;
    }

    // 分配帧结构体，用于接收解码后的数据
    AVFrame * frame = av_frame_alloc();
    if(!frame) {
        fprintf(stderr, "Could not allocate AVFrame\n");
        exit(1);
    }

    // 不断从解码器取出解码后的帧
    while(ret >= 0) {
        ret = avcodec_receive_frame(dec, frame);
        if(ret < 0) {
            // AVERROR(EAGAIN): 需要更多输入数据
            // AVERROR_EOF: 解码器已无更多输出
            if(ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;
            fprintf(stderr, "Error during decoding\n");
            return ret;
        }

        // 根据类型输出到对应文件
        if(dec->codec->type == AVMEDIA_TYPE_VIDEO) {
            ret = outputVideoFrame(frame); // 输出视频帧
        } else if(dec->codec->type == AVMEDIA_TYPE_AUDIO) {
            ret = outputAudioFrame(frame); // 输出音频帧
        }

        av_frame_unref(frame); // 复用frame，清空数据
        if(ret < 0)
        {    return ret;    }
    }
    av_frame_free(&frame); // 释放帧结构体
    return 0;
}

/*
 * 输出一帧视频到YUV文件
 * 参数说明：
 *   frame: 解码后的视频帧
 */
static int outputVideoFrame(AVFrame *frame)
{
    printf("video_frame n: %d coded_n:%d\n",
           video_frame_count++, frame->coded_picture_number);

    // 拷贝帧数据到输出缓冲区
    av_image_copy(video_dst_data, video_dst_linesize,
                  (const uint8_t**)frame->data, frame->linesize,
                  pix_fmt, frame->width, frame->height);

    // 写入文件
    fwrite(video_dst_data[0], 1, video_dst_bufsize, outVideoFp);
    return 0;
}

/*
 * 输出一帧音频到PCM文件
 * 参数说明：
 *   frame: 解码后的音频帧
 */
static int outputAudioFrame(AVFrame *frame)
{
    // 计算一帧音频数据的实际字节数
    size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)frame->format);

    printf("audio frame n: %d, nb_samples:%d, pts:%lld\n",
           audio_frame_count++, frame->nb_samples, frame->pts);

    // 只写第一个声道的数据（如为planar格式）
    fwrite(frame->extended_data[0], 1, unpadded_linesize, outAudioFp);
    return 0;
}

/*
 * 根据音频采样格式获取ffplay命令行参数格式字符串
 * 参数说明：
 *   fmt: 返回的格式字符串指针
 *   sample_fmt: 输入的音频采样格式
 */
static int get_format_from_sample_fmt(const char **fmt,
                                      enum AVSampleFormat sample_fmt)
{
    int i;
    // 支持的采样格式及其ffplay命令行参数
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
        { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
        { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
        { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
        { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
        { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
    };
    *fmt = NULL;

    // 遍历查找匹配的采样格式
    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le); // 根据字节序选择
            return 0;
        }
    }

    // 不支持的采样格式
    fprintf(stderr,
            "sample format %s is not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return -1;
}

/*
 * 总结代码流程思路：
 * 1. 打开输入媒体文件，获取媒体流信息。
 * 2. 查找视频流和音频流，分别打开解码器。
 * 3. 为视频帧分配输出缓冲区，为音频帧分配输出文件。
 * 4. 循环读取每一个packet，根据其类型送入对应解码器。
 * 5. 解码后的视频帧写入YUV文件，音频帧写入PCM文件。
 * 6. 解码结束后，输出ffplay命令用于播放结果文件。
 * 7. 释放所有资源。
 */