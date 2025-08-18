#include <stdio.h>
#include <cjson/cJSON.h>
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;


/*
思路分析：现在的主要需求是对json的字符串进行解析
但是对于json字符串进行生成的各种操作也必须要会，等后面再回来对这个测试用例进行进一步的学习

但是最好还是使用nlohmann_json,因为现在主流使用的都是这个，所以到时候自己实现的时候也要使用这个
*/



//生成cjson字符串
void gen_cjson_str(){
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", "Bob");  //进行json键值对的添加
    cJSON_AddNumberToObject(root, "age", 30);

    // 生成字符串（自动分配内存）
    char *json_str = cJSON_Print(root);
    printf("%s\n", json_str);

    // 释放内存
    cJSON_free(json_str);
    cJSON_Delete(root);
}

// 解析JSON字符串
void Analy_cjson_str(){
    //cJSON_Error error;
    
    const char *json_str = "{\"name\":\"Alice\",\"age\":25}";
    
    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL) {

        
        const char *error_ptr = cJSON_GetErrorPtr();
        
        printf("解析错误: %s\n", error_ptr);
        return;
    }

    // 提取数据
    cJSON *name = cJSON_GetObjectItemCaseSensitive(root, "name");
    cJSON *age = cJSON_GetObjectItemCaseSensitive(root, "age");

    if (cJSON_IsString(name) && name->valuestring != NULL) {
        printf("Name: %s\n", name->valuestring);
    }
    if (cJSON_IsNumber(age)) {
        printf("Age: %d\n", age->valueint);
    }

    // 释放内存
    cJSON_Delete(root);
}



void Analy_cjson_str(const char * str){
    const char *json_str = str;




    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        printf("解析错误: %s\n", error_ptr);
        return;
    }

    // 提取数据
    cJSON *name = cJSON_GetObjectItemCaseSensitive(root, "name");
    cJSON *age = cJSON_GetObjectItemCaseSensitive(root, "age");

    if (cJSON_IsString(name) && name->valuestring != NULL) {
        printf("Name: %s\n", name->valuestring);
    }
    if (cJSON_IsNumber(age)) {
        printf("Age: %d\n", age->valueint);
    }

    // 释放内存
    cJSON_Delete(root);
}


int main() {
    
    gen_cjson_str();

    cout<<"上面是生成json格式字符串"<<endl;

    cout<<"----------------------------------"<<endl;
    
    cout<<"下面是解析json格式字符串"<<endl;

    
    Analy_cjson_str();

    
    return 0;
}