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

    MyLogger& logger = MyLogger::getInstance();   //每次都要直接加这句话
    



//int file_fd=open(_file_path.c_str(), O_RDWR | O_CREAT, 0644);
//表示对这个文件的权限是：可读可写，并且如果这个路径没有则可以用这条语句进行创建

//一般常见的文件都最好设计为0644，不要设计为0666这样的别人也可以随便编辑的权限



FileStream::FileStream(const string& path)
: _file_path(path)
{
    LOG(INFO,"构造函数成功");
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



void FileStream::processChunk(char* mapped_data, size_t start, size_t end, std::mutex& mtx) {
    // 加锁保证输出顺序（非必须，仅演示线程安全）
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "线程 " << std::this_thread::get_id()    //如何在linux中获取线程id？是不是我需要设计一个函数来实现获取
              << " 处理范围: [" << start << ", " << end << ")" << std::endl;

    // 处理映射区域的数据（无需加锁，因各线程访问独立区域）
    for (size_t i = start; i < end; ++i) {
        mapped_data[i] = toupper(mapped_data[i]); // 示例：转大写
    }
}


//这里如果设置为萝卜坑，那所有调用这个函数的都需要设置一个萝卜坑参数了
int FileStream::handle_chunk(size_t chunk_size,int method){
    // 直接读取内存中的数据
    //这里返回的file_data是映射到内存中的首地址


    file_size=lseek(_file_fd, 0, SEEK_END);
    if(file_size == -1){
        perror("lseek");
        return -1;
    }

    void* addr=nullptr;

    
    if(method==READ){
        //进行读操作

        //文件映射成功
    void* addr=mmap(nullptr,file_size,PROT_READ,MAP_PRIVATE,_file_fd,0);

    if(addr == MAP_FAILED){
        perror("mmap");
        return -1;
    }

    //下面这个是每次处理16个字节的操作，但是可以和进行帧操作进行类比：从而实现每次进行一个数据帧的读取操作
    //所以为了能读取各种各样的大小的内容，我觉得可以把这个操作封装成一个成员函数
    const char *file_data = (const char *)addr;   //除了void* 其他的内容想进行强转都需要通过static_cast来实现
   


    /*
    上面是实现内存映射，下面是实现多线程分块访问这片内存映射区域
    首先无论是读还是写，都是一样的先内存映射，再确定分区，唯一的不同就是调用的process函数不同
    然后现在需要进行的处理是：对每个process进行参数计算，然后把他们通过bind添加到addtask中，然后就等待处理即可
    所以此时每个分区都是只会对自己的分区进行读写修改，所以并不会涉及任何的顺序序号的问题
    因此现在的问题就单纯是process的参数计算，然后再进行process的代码逻辑的书写即可
    */

   //#pragma omp parallel for  这个参数能实现多线程的操作
   //所以这就取决于：我每个process要不要分给每个线程进行操作了————————即进行文件IO也是可以通过多线程对文件的内容进行拆分再handel的
    for (size_t i = 0; i < file_size; i += chunk_size) {
        size_t handle_size = (file_size - i) > chunk_size ? chunk_size : (file_size - i);
        
        //processFrame(file_data + i, handle_size);
        /*
        file_data是首地址，然后+i是加偏移量
        这个process只需要通过bind和placeholders配合即可实现
        所以并不是放在这个调用的，而是放在addtask中和bind配合才能实现调用的
        */
    }
    
}   //end of if READ

    if(method==WRITE){
        //进行写操作

        void* addr=mmap(nullptr,file_size,PROT_READ|PROT_WRITE,MAP_SHARED,_file_fd,0);

        if(addr == MAP_FAILED){
        perror("mmap");
        return -1;
        }

        //下面这个是每次处理16个字节的操作，但是可以和进行帧操作进行类比：从而实现每次进行一个数据帧的读取操作
        //所以为了能读取各种各样的大小的内容，我觉得可以把这个操作封装成一个成员函数
        const char *file_data = (const char *)addr;   //除了void* 其他的内容想进行强转都需要通过static_cast来实现
   
        //#pragma omp parallel for  这个参数能实现多线程的操作
        //所以这就取决于：我每个process要不要分给每个线程进行操作了————————即进行文件IO也是可以通过多线程对文件的内容进行拆分再handel的
        for (size_t i = 0; i < file_size; i += chunk_size) {
        size_t handle_size = (file_size - i) > chunk_size ? chunk_size : (file_size - i);
        
        processFrame(file_data + i, handle_size);
        //file_data是首地址，然后+i是加偏移量
        }
    }


    
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
}   //end of func handle_chunk



}   //end of namespace ghz





