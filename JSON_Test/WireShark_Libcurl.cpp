#include <curl/curl.h> 
#include "../clonmann/json.hpp"
#include "../clonmann/json_fwd.hpp"
#include <stdio.h> 

#include <string>

using std::string;


//需求：需要加请求头部，请求头部应该是默认加的，其他的应该都没问题了

//现在这个暂时是能跑起来的，并且能打印出来
//所以做一切封装之前，都在这个main文件中进行测试完了封装进去
int main() {
    CURL *curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL,"http://192.168.5.222/xsw/api/record/list?beginTime=1755426141631&channel=0&endTime=1755426367632&t=1755432860&token=fcd565e348de205603ec9ca0e7301394");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);  // 跟随重定向

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    return 0;
}




//进行读操作的回调函数
size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {


    //这个应该是修改char*
    const char *buffer = static_cast<const char*>(userp);
    buffer->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}


void handle_curlToHTTP(string url){
    CURL *curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL,url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);  // 跟随重定向

        //进行头部的设置
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        //进行回调函数的设置
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    return ;
}


//还需要写json的解析，这就需要在这里再使用clomann了写了

//这里直接使用这个本地文件的clohmann就行了
//等问问组长怎么放进去的？








