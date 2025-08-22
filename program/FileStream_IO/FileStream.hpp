
/*
先整理需求，在整理技术

需求：
1. 支持文件的读写操作，包括文本文件和二进制文件。
2. 提供简单易用的接口，方便用户进行文件操作。
3. 支持文件的打开、关闭、读、写、删除等基本操作。
4. 处理文件操作中的错误情况，并提供相应的错误信息。

这里的文件主要就是录像文件
*/



/*
当前最新需求：先求出文件大小，再求出线程数，从而就能计算出分块的大小
然后再进行从内存映射的起始位置开始，然后进行for循环的具体分配进行处理的内存区域。从而获取start和end两个数据位置
然后再在process函数中通过bind传入这两个参数

*/

#include "header.hpp"
#include "ThreadPool.hpp"
#include "Condition.hpp"
#include "MutexLock.hpp"
#include "logger_myversion.hpp"
#include <string>
#include <vector>
#include <omp.h>

enum{
    READ=1,
    WRITE
};

namespace ghz{
    using std::string;
    using std::vector;

    using group_6::Condition;
    using group_6::MutexLock;
    using group_6::MutexLockGuard;

    class FileStream {
    public:
        FileStream(const string& path);
        ~FileStream()=default;

        bool file_open();
        void file_close();
        int handle_chunk(size_t chunk_size,int method);
        //这里的chunk_size是每次处理的大小，method是读写操作
        //所以这个函数既可以进行读操作，又可以进行写操作，是通用成员函数

        //现在需要做的：只有读数据的操作了，再看下去甚至都不需要专门写一个类了，因为只需要进行mmap映射就可以了
        
        //进行每块的内容进行处理的操作
        void processReadVideo(char* mapped_data, size_t start, size_t end, std::mutex& mtx);
        void processWriteVideo(char* mapped_data, size_t start, size_t end, std::mutex& mtx);

        //在这里可以添加目录操作，减少对于文件的读写操作
        //比如可以根据日期进行目录的组织

        //进行文件的目录操作
        bool file_create_directory(const string& dir);   //创建目录        
        bool file_list_directory(const string& dir, vector<string>& files);  //列出目录中的所有文件

        //都是bool操作，所以最终应该都是需要进行错误判断的

       
        //启动内存映像和结束内存映像的两个函数
        void* start_mmap(int method);
        void release_mmap(void* &addr, size_t size);


        

    private:
        const string& _file_path;
        int _file_fd;
        size_t _file_size;

        //进一步优化看看读写操作有没有必要进行数据成员的增加？
    };
}

