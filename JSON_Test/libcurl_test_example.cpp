#include <curl/curl.h>  
#include <stdio.h>  

/*
libcurl开发框架的处理方法：
1. 初始化libcurl库
2. 设置请求URL
3. 设置回调函数处理响应数据
4. 执行请求
5. 清理资源
*/

/*
现在所需要的业务逻辑需求：发送对于url请求的http报文
然后我现在不确定如何获取并解析http响应报文，但是我知道响应报文要摘出json_str,然后剩下的操作就是对这个str进行json解析了

回复：现在能确定http响应报文了：
*/


//设置回调函数来处理响应数据
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) { 
    
    /*
    关于这个用于进行处理的回调函数，也是有严格的参数设置的:
    data: 收到的数据块指针
    size * nmemb: 数据块总大小
    userp: 用户自定义指针（如 QByteArray*）
    */
    
    /*
    处理响应数据（如下面这个代码效果就是打印到控制台）
    但是我现在需要在这个函数中写出来的效果就是将json_str提取出来
    然后通过tlv的格式把这个json内容传到Qt客户端
    或者直接在本地直接进行json解析，然后直接把解析结果进行对客户端的消息通知即可
    这样做好处是减少了一次不必要的数据传输验证，但是坏处是服务器客户端一对多的条件下，这样做会极大降低服务器性能
    */

    /*
    所以这里是进行拆包为json，然后直接TLV发送给Qt客户端，不需要进行什么业务逻辑处理
    然后让qt客户端自己进行json解析
    */
    fwrite(contents, size, nmemb, (FILE*)userp);
    return size * nmemb;
}  

int responce() {  
    CURL *curl = curl_easy_init();  
    FILE *fp = fopen("response.txt", "wb");  

    if (curl && fp) {  


        /*
        关于http响应报文是如何实现的：
        通过 CURLOPT_WRITEFUNCTION 指定数据处理回调，
        并通过 CURLOPT_WRITEDATA 传递用户自定义指针（如缓冲区对象）
        */

        //http请求的setopt有整整一千行，所以应该是需要什么功能再进行查询即可
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.example.com/data");  
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);   //用到了这个宏，就是进行了自定义响应数据处理函数 
        
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); 
        /*
        这行代码的执行效果是怎样的？
        我认为效果就是把获得的内容存到文件中，所以其实执行效果是怎样的，和这条语句关系不大，
        主要是和write_callback函数的实现有关，write_callback函数内部的执行效果是怎样的，这个函数传参作为缓冲区的展示结果就是怎样的
        */


        // 执行请求  
        CURLcode res = curl_easy_perform(curl);  
        if (res != CURLE_OK) {  
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res)); 
            //这个strerror是用来分析具体是什么原因导致的错误 
        }  

        curl_easy_cleanup(curl);  
        fclose(fp);
    }  
    return 0;  
}





/*
这里单纯是代码写错了，应该是写为linux版本的代码的
不过也印证了Qt完全具备包装请求报文和回收响应报文的功能，所以服务器端只负责转发即可
客户端进行对于request和response的处理

而对于http进行视频的发送，肯定都是以一个个AVPacket来进行发送的
所以关于视频的发送大小的问题不用担心
*/


size_t read_callback(void *buffer, size_t size, size_t nitems, void *userdata) {
    QFile *file = static_cast<QFile*>(userdata);
    qint64 bytesRead = file->read(static_cast<char*>(buffer), size * nitems);
    return static_cast<size_t>(bytesRead);  // 返回实际读取的字节数
}




int request(){

    // 配置（用于文件上传）
    QFile file("data.txt");
    file.open(QIODevice::ReadOnly);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &file);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);  // 启用上传模式

}



int main(){

    return 0;
}