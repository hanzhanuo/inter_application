
/*
先整理需求，在整理技术

需求：
1. 支持文件的读写操作，包括文本文件和二进制文件。
2. 提供简单易用的接口，方便用户进行文件操作。
3. 支持文件的打开、关闭、读、写、删除等基本操作。
4. 处理文件操作中的错误情况，并提供相应的错误信息。

这里的文件主要就是录像文件
*/

#include "header.hpp"
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

    class FileStream {
    public:
        FileStream(const string& path);
        ~FileStream()=default;

        bool file_open();
        void file_close();
        int handle_chunk(size_t chunk_size,int method);
        ssize_t file_read(void* buf, size_t chunk_size);  //由于这个count刚好用不到，就设计为进行读取并处理的单位吧        
        ssize_t file_write(const void* buf, size_t chunk_size);
        bool file_remove();

        //在这里可以添加目录操作，减少对于文件的读写操作
        //比如可以根据日期进行目录的组织

        //进行文件的目录操作
        bool file_create_directory(const string& dir);   //创建目录
        bool file_remove_directory(const string& dir);   //删除目录
        bool file_list_directory(const string& dir, vector<string>& files);  //列出目录中的所有文件

        //都是bool操作，所以最终应该都是需要进行错误判断的

    private:
        const string& _file_path;
        int _file_fd;
        size_t file_size;

        //进一步优化看看读写操作有没有必要进行数据成员的增加？
    };
}

