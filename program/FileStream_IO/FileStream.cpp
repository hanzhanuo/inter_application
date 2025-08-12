#include "FileStream.hpp"


/*
需求是保存和读取录像文件
这就需要进行文件打开，关闭，删除，存储
处理文件操作中的错误情况，并提供相应的错误信息
````
*/

//现在这个类的需求是必须既能实现对于本地磁盘的读写操作，又能实现对于服务器端的文件读写操作了
//然后具体的不同行为，就体现在在具体测试用例上了。在类中属于是抽象为完全相同的内容

namespace ghz{



//int file_fd=open(_file_path.c_str(), O_RDWR | O_CREAT, 0644);
//表示对这个文件的权限是：可读可写，并且如果这个路径没有则可以用这条语句进行创建

//一般常见的文件都最好设计为0644，不要设计为0666这样的别人也可以随便编辑的权限



FileStream::FileStream(const string& path)
: _file_path(path)
{
}

bool FileStream::file_open(){
    _file_fd=open(_file_path.c_str(), O_RDWR |  O_DIRECT | O_LARGEFILE | O_CREAT, 0644);
    if(_file_fd==-1){
        perror("open");
        return false;
    }


    return true;
}

void FileStream::file_close(){
    close(_file_fd);
}

bool FileStream::file_create_directory(const string& dir){
    if(mkdir(dir.c_str(), 0755) == -1){   //对于目录的权限都是设计为0755即可，目录没有文件的权限那么重要

        //所以这里的主要错误判断就是是否创建成功和是否删除成功了
        perror("mkdir");
        return false;
    }
    return true;
}

bool FileStream::file_remove_directory(const string& dir){
    if(rmdir(dir.c_str()) == -1){
        perror("rmdir");
        return false;
    }
    return true;
}

//这里的vector只是左值引用，没加const，所以肯定是一个传入传出参数。
//这个一定要会
bool FileStream::file_list_directory(const string& dir, vector<string>& files){
    //对文件夹中饿内容进行遍历，如果遍历失败了，就return false
    //所以果然是通过entry实现对于文件夹的遍历操作
    DIR* dp = opendir(dir.c_str());
    if(dp == nullptr){
        perror("opendir");
        return false;
    }

    struct dirent* entry;
    while((entry = readdir(dp)) != nullptr){
        if(entry->d_type == DT_REG){
            files.push_back(entry->d_name);
        }
    }

    closedir(dp);
    return true;
}


//这里如果设置为萝卜坑，那所有调用这个函数的都需要设置一个萝卜坑参数了
int FileStream::handle_chunk(size_t chunk_size,int method){
    // 直接读取内存中的数据
    //这里返回的file_data是映射到内存中的首地址

    if(method==READ){
        //进行读操作
    }

    if(method==WRITE){
        //进行写操作
    }


    file_size=lseek(_file_fd, 0, SEEK_END);
    if(file_size == -1){
        perror("lseek");
        return -1;
    }

    //文件映射成功
    void* addr=mmap(nullptr,file_size,PROT_READ,MAP_PRIVATE,_file_fd,0);

    if(addr == MAP_FAILED){
        perror("mmap");
        return -1;
    }

    //下面这个是每次处理16个字节的操作，但是可以和进行帧操作进行类比：从而实现每次进行一个数据帧的读取操作
    //所以为了能读取各种各样的大小的内容，我觉得可以把这个操作封装成一个成员函数
    const char *file_data = (const char *)addr;   //除了void* 其他的内容想进行强转都需要通过static_cast来实现
   
   #pragma omp parallel for  这个参数能实现多线程的操作
   //所以这就取决于：我每个process要不要分给每个线程进行操作了————————即进行文件IO也是可以通过多线程对文件的内容进行拆分再handel的
    for (size_t i = 0; i < file_size; i += chunk_size) {
        size_t handle_size = (file_size - i) > chunk_size ? chunk_size : (file_size - i);
        
        processFrame(file_data + i, handle_size);
        //file_data是首地址，然后+i是加偏移量
    }
}


ssize_t FileStream::file_read(void* buf, size_t chunk_size){

    //此时打开文件的file_fd有了
    //然后就是获取文件大小
    file_size=lseek(_file_fd, 0, SEEK_END);
    if(file_size == -1){
        perror("lseek");
        return -1;
    }

    //文件映射成功
    void* addr=mmap(nullptr,file_size,PROT_READ,MAP_PRIVATE,_file_fd,0);

    if(addr == MAP_FAILED){
        perror("mmap");
        return -1;
    }

    //使用了内存映射就不需要进行read和write操作了，因为那些都是从磁盘中读，现在是直接映射到内存，从内存中的某片区域读了

    //其实每次mmap把整个文件都读出来之后，就可以把file_fd关上了，但是由于我使用的时候析构函数管理fd的生命周期，所以就可以不需要进行手动关闭

    

    /*
    解除映射
    解除映射是解除这个返回的addr指针，从而回收对这片映射空间的操作权限

    */ 
    //一定要能熟练应用这些if中调用函数，然后顺便进行判断的操作。
    //这种操作在文件IO中出现好多好多次了
    if (munmap(addr, file_size) == -1) {
        perror("munmap failed");
        return 1;
    }

    


}

//进行文件删除的操作
bool FileStream::file_remove(){

}



}   //end of namespace ghz





