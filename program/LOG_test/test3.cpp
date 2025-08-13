#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/Priority.hh>

#define LOG(priority, message) \
    log4cpp::Category::getRoot() << log4cpp::Priority::priority \
    << "[" << __FILE__ << ":" << __LINE__ << "][" << __FUNCTION__ << "] " << message


    
int main() {
    // 1. 创建 Layout（定义日志格式）
    log4cpp::PatternLayout* fileLayout = new log4cpp::PatternLayout();
    fileLayout->setConversionPattern("%d{%Y-%m-%d %H:%M:%S.%l} [%p] %c: %m%n");

    log4cpp::PatternLayout* consoleLayout = new log4cpp::PatternLayout();
    consoleLayout->setConversionPattern("%d{%H:%M:%S.%l} [%p] %m%n"); // 简化的屏幕格式

    // 2. 创建 Appender
    log4cpp::Appender* fileAppender = new log4cpp::FileAppender("fileAppender", "app.log");
    fileAppender->setLayout(fileLayout);

    log4cpp::Appender* consoleAppender = new log4cpp::OstreamAppender("consoleAppender", &std::cout);
    consoleAppender->setLayout(consoleLayout);

    // 3. 获取根 Logger 并添加 Appender
    log4cpp::Category& root = log4cpp::Category::getRoot();
    root.setPriority(log4cpp::Priority::DEBUG); // 设置日志级别
    root.addAppender(fileAppender);
    root.addAppender(consoleAppender);

    LOG(INFO, "User logged in");  // 自动注入文件、行号、函数名
    LOG(ERROR, "Database connection failed");
    // 5. 清理资源
    log4cpp::Category::shutdown();
    return 0;
}