#include <stdio.h>
extern "C"   //FFmpeg是C语言编写的，所以必须要加extern才能在cpp中使用
//然后还要注意extern的形式：使用语句块的形式这样单独包含，这样是最好的直接区分c和cpp库的写法
{
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavcodec/avcodec.h>
}

#include <string>
using std::string;


//在对FFmpeg进行编译的时候，必须要在makefile中加-l这个库才能编译成功

int read_packet(void * ptr, uint8_t * buf, int buf_size)
{
    static int total = 0;
    FILE * fp = (FILE*)ptr;
    int sz = fread(buf, 1, buf_size, fp);
    total += sz;
    printf("Read %d bytes, total:%d\n", sz, total);
    return sz;
}

int64_t seek_func(void * opaque, int64_t offset, int whence)
{
    int64_t ret = 0;
    FILE * fp = (FILE*)opaque;
    if(whence == AVSEEK_SIZE) {
        return EXIT_FAILURE;
    }
    fseek(fp, offset, whence);
    ret = ftell(fp);
    printf("ftell pos: %lld\n", ret);
    return ret;
}

int testAVIO2()
{
    printf("==============================\n");
    //FILE * fp = fopen("D:\\ac_wlh480.mp4", "rb");//可以打开文件，但读取时出错
    //FILE * fp = fopen("output.flv", "rb");//打开文件失败
    FILE * fp = fopen("D:\\videos\\output.flv", "rb");
    if(nullptr == fp) {
        printf("open file error\n");
        return EXIT_FAILURE;
    }

    int size = 1024;
    unsigned char * pBuff = (unsigned char *)malloc(size);

    AVIOContext * pIOCtx = avio_alloc_context(
                pBuff,
                size,
                0,
                fp,
                read_packet,
                0,
                seek_func);
    printf("avio_alloc_context end\n");

    AVFormatContext * pFmtCtx = avformat_alloc_context();
    if(pFmtCtx == nullptr) {
        printf("avformat_alloc_context error\n");
        return EXIT_FAILURE;
    }
    printf("avformat_alloc_context end\n");
    pFmtCtx->pb = pIOCtx;//关联绑定在一起
    pFmtCtx->flags = AVFMT_FLAG_CUSTOM_IO;

    AVInputFormat * pInputFmt = nullptr;
    if(avformat_open_input(&pFmtCtx, "", pInputFmt, nullptr) < 0) {
        printf("avformat_open_input error.\n");
        goto quit;
    } else {
        printf("open stream success.\n");
    }

    if(avformat_find_stream_info(pFmtCtx, nullptr) < 0) {
        printf("avformat_find_stream_info error\n");
        goto quit;
    } else {
        printf("avformat_find_stream_info success.\n");
        printf("nb_streams: %d\n", pFmtCtx->nb_streams);
    }

quit:
    avformat_close_input(&pFmtCtx);
    avformat_free_context(pFmtCtx);
    free(pBuff);

    return 0;
}