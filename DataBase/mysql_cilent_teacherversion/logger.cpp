#include <iostream>
#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PatternLayout.hh>
#include <string>
#include <mutex>
#include <sys/stat.h>  // 用于创建目录
#include <sys/types.h>

class MyLogger {
public:
    // 获取单例实例
    static MyLogger& getInstance() {
        static std::mutex mutex;
        std::lock_guard<std::mutex> guard(mutex);
        static MyLogger instance;
        return instance;
    }

    // 记录警告日志
    void warn(const std::string& msg) {
        category.warn(msg);
    }

    // 记录错误日志
    void error(const std::string& msg) {
        category.error(msg);
    }

    // 记录信息日志（常规操作记录）
    void info(const std::string& msg) {
        category.info(msg);
    }

private:
    // 私有构造函数，初始化日志配置
    MyLogger() : category(log4cpp::Category::getRoot()) { // 在初始化列表中初始化引用成员
        // 确保上级目录的log文件夹存在
        createLogDirectory("../log");

        // 创建PatternLayout以获得更详细的日志格式
        log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
        layout->setConversionPattern("%d [%p] %m%n");  // 日期 日志级别 消息

        // 创建文件Appender，日志文件路径设为../log/server.log
        log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", "../log/server.log");
        appender->setLayout(layout);

        // 配置日志分类
        category.addAppender(appender);
        category.setPriority(log4cpp::Priority::INFO);
    }

    // 私有拷贝构造函数和赋值运算符，防止复制
    MyLogger(const MyLogger&) = delete;
    MyLogger& operator=(const MyLogger&) = delete;

    // 创建日志目录（如果不存在）
    void createLogDirectory(const std::string& path) {
            mkdir(path.c_str(), 0755);
    }

    // log4cpp日志分类对象
    log4cpp::Category& category;
};

