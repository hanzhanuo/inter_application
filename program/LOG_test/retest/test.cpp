#include "logger_myversion.cpp"

int main(){

    MyLogger& logger = MyLogger::getInstance();

    LOG(INFO, "User logged in");  // 自动注入文件、行号、函数名
    LOG(ERROR, "Database connection failed");

    return 0;
}