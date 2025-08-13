#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/PatternLayout.hh>

// 定义日志宏
#define LOG(priority, message) \
    log4cpp::Category::getRoot() << log4cpp::Priority::priority \
    << "[" << __FILE__ << ":" << __LINE__ << "][" << __FUNCTION__ << "] " << message

#if 0
int main() {
    // 1. 配置 log4cpp
    log4cpp::Appender* appender = new log4cpp::FileAppender("default", "app.log");
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d{%Y-%m-%d %H:%M:%S} %p: %m%n"); // %m 包含文件、行号、函数名
    appender->setLayout(layout);

    log4cpp::Category& root = log4cpp::Category::getRoot();
    root.setPriority(log4cpp::Priority::DEBUG);
    root.addAppender(appender);

    // 2. 使用宏记录日志
    LOG(INFO, "User logged in");  // 自动注入文件、行号、函数名
    LOG(ERROR, "Database connection failed");

    // 3. 清理
    log4cpp::Category::shutdown();
    return 0;
}

#endif