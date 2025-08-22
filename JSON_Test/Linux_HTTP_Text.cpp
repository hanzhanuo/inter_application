
/*
现在能实现的功能：
获取responce的响应头部，然后对这个响应头进行处理
进行post类型的请求的发送(这个就是必须要加请求头固定参数字段)

*/

//所以现在的重点：一个是加请求响应的头部，一个是加回调函数应该显示在一个string中，然后把string进行传递解析
//然后写一个json专门用来解析这个string


#include <curl/curl.h>
#include <iostream>
#include <cjson/cJSON.h>
#include <string>

using std::cout;
using std::endl;
using std::string;




void Analy_cjson_str(string str){
    //cJSON_Error error;

    printf("start to Analy_cjson_str\n");
    
    const char *json_str =str.c_str();
    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL) {

        
        const char *error_ptr = cJSON_GetErrorPtr();
        
        printf("解析错误: %s\n", error_ptr);
        return;
    }

    // 提取数据：根据json中的key字段，实现对对应value的内容进行提取(value提取结果就是这个函数的返回值)
    cJSON *code = cJSON_GetObjectItemCaseSensitive(root, "code");
    cJSON *msg = cJSON_GetObjectItemCaseSensitive(root, "msg");
    
    //这上下几行代码是最重要需要处理的

    //对上面获取的value数据进行打印的操作。所以这个其实是否封装进函数，对这个value的接收和处理效果完全没有影响
    if (cJSON_IsNumber(code)) {
        printf("Code: %d\n", code->valueint);
    }
    if (cJSON_IsString(msg) && msg->valuestring != NULL) {
        printf("Message: %s\n", msg->valuestring);
    }

    //然后再进一步根据返回的数据，进行不同情况的处理判断(即根据不同的字段进行对Qt客户端进行不同的反馈的if-else逻辑)

    // 释放内存
    cJSON_Delete(root);
}



// 回调函数：处理接收到的数据
size_t write_callback(char *ptr, size_t size, size_t nmemb, std::string *data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

//除了这种responce回调函数以外，还可以在这个write_callback函数中写直接写到文件中的处理方式
//所以这个内部的代码逻辑是最多变的，也是最重要的

int main() {
    CURL *curl = curl_easy_init();
    if (curl) {
        std::string response_data;

        // 设置请求URL
        curl_easy_setopt(curl, CURLOPT_URL, "www.baidu.com");
        //http://192.168.5.222/xsw/api/ptz/control?speed=69&stop=1&t=1755482139&token=fa9e44399cf95ea134ed396e462aeabf&value=u

        // 设置数据接收回调
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);


        //下面这段申请请求头好像没什么用，所以可以去掉

        // 创建请求头链表
        struct curl_slist *headers = NULL;
        //老师写的是发送json的格式，我这里使用的是默认格式
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        //添加请求头选项
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        


        // 执行请求
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "请求失败: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "响应数据:\n" << response_data << std::endl;
        }

        Analy_cjson_str(response_data);

        // 清理资源
        curl_slist_free_all(headers);   //释放请求头链表
        curl_easy_cleanup(curl);
    }
    return 0;
}



