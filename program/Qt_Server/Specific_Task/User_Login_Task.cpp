#include "User_Login_Task.hpp"
#include "Message.hpp" 
#include "MySQLClient.hpp"
#include <iostream>
#include <random>

#include "MyLogger.hpp"

using std::cout;
using std::endl;

namespace wd {

void UserLoginSection1::process() {
    // 从数据包提取用户名
    string username = _packet.msg;
    TLV response;

    // 查询数据库检查用户名是否存在并获取密码哈希
    MySQLClient mysql;
    mysql.connect("192.168.10.50", 3306, "root", "ljc521", "ljc");

    // 修改SQL查询：获取密码哈希而非仅计数
    string sql = "SELECT encrypt FROM user WHERE name = '" + username + "'";
    auto result = mysql.readOperationQuery(sql);
    if (!result.empty() && result.size() > 1) {
        string passwordHash = result[1][0]; // 获取密码哈希字段
        if (!passwordHash.empty()) {
            // 提取盐值部分（格式：$id$salt$hash -> 提取$id$salt）
            size_t firstDollar = passwordHash.find('$');
            size_t secondDollar = passwordHash.find('$', firstDollar + 1);
            size_t thirdDollar = passwordHash.find('$', secondDollar + 1);
            
            if (thirdDollar != string::npos) {
                // 提取第二个$和第三个$之间的盐值部分，并截取前8个字符
                size_t saltStart = secondDollar + 1;
                string salt = passwordHash.substr(saltStart, 8); // 仅获取8个字符盐值
                response.type = TASK_TYPE_LOGIN_SECTION1_RESP_OK;
                strncpy(response.data, salt.c_str(), salt.length());  // 设置盐值到响应消息
                response.length = salt.length();
            } else {
                // 哈希格式错误
                response.type = TASK_TYPE_LOGIN_SECTION1_RESP_ERROR;
                response.length = 0;
            }
        } else {
            // 密码哈希为空
            response.type = TASK_TYPE_LOGIN_SECTION1_RESP_ERROR;
            response.length = 0;
        }
    } else {
        // 用户名不存在
        response.type = TASK_TYPE_LOGIN_SECTION1_RESP_ERROR;
        response.length = 0;
    }
    _conn->sendInLoop(response);
    // 添加查询所有姓名的代码
    string allNamesSql = "SELECT * FROM user";
    vector<vector<string>> allNamesResult = mysql.readOperationQuery(allNamesSql);
    cout << "[数据库所有用户信息] 查询结果: " << endl;
    mysql.dump(allNamesResult);
}

void UserLoginSection2::process() {
    
    // 从登录数据包中提取用户名和密码（格式为"username:password"）
    string data = _packet.msg;
    size_t colonPos = data.find(':');
    if (colonPos == string::npos) {
        // 数据包格式错误
        TLV response;
        response.type = TASK_TYPE_LOGIN_SECTION2_RESP_ERROR;
        response.length = 0;
        _conn->sendInLoop(response);
        return;
    }
    string username = data.substr(0, colonPos);
    string encryptedPassword = data.substr(colonPos + 1);
    
    // // 创建密码验证器实例
    // PasswordValidator validator;
    // ValidateResult result = validator.validate(username, password);
    
    // // 转换为Message.hpp中定义的枚举值
    // TaskType responseType = validator.convertToTaskType(result);
    // 查询数据库中的加密密码
    MySQLClient mysql;
    mysql.connect("192.168.10.50", 3306, "root", "ljc521", "ljc");
    string sql = "SELECT encrypt FROM user WHERE name = '" + username + "'";
    auto result = mysql.readOperationQuery(sql);

    if (result.empty() || result.size() <= 1) {
        // 查询失败或用户不存在
        TLV response;
        response.type = TASK_TYPE_LOGIN_SECTION2_RESP_ERROR;
        response.length = 0;
        _conn->sendInLoop(response);
        return;
    }
    // 验证加密密码
    string storedPassword = result[1][0];
    TLV response;
    if (storedPassword == encryptedPassword) {
        response.type = TASK_TYPE_LOGIN_SECTION2_RESP_OK;
    } else {
        response.type = TASK_TYPE_LOGIN_SECTION2_RESP_ERROR;
    }
    response.length = 0;
    _conn->sendInLoop(response);
    //response.type = responseType;
    
    // switch(responseType) {
    //     case TASK_TYPE_LOGIN_SECTION2_RESP_OK:
    //         response.length = sizeof(ValidateResult);
    //         memcpy(response.data, &result, sizeof(ValidateResult));
    //         _conn->sendInLoop(response);
    //         break;
    //     default:
    //         // 处理系统错误
    //         response.type = TASK_TYPE_LOGIN_SECTION2_RESP_ERROR;
    //         response.length = 0;
    //         _conn->sendInLoop(response);
    //         break;
    // }

}

// 注册阶段1：检查用户名是否可用
void UserRegisterSection1::process() {
        // 从数据包提取用户名
        string username = _packet.msg;
        TLV response;

        // 查询数据库检查用户名是否存在
        MySQLClient mysql;
        mysql.connect("192.168.10.50", 3306, "root", "ljc521", "ljc");

        string sql = "SELECT COUNT(*) FROM user WHERE name='" + username + "'";
        auto result = mysql.readOperationQuery(sql);
        if (!result.empty() && result.size() > 1) {
            int count = stoi(result[1][0]); // 跳过表头行
            if (count == 0) {
                // 用户名可用，生成8位随机盐值
                string salt;
                const string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
                std::random_device rd;
                std::mt19937 generator(rd());
                std::uniform_int_distribution<> distr(0, chars.size() - 1);
                for (int i = 0; i < 8; ++i) {
                    salt += chars[distr(generator)];
                }

                // 发送盐值响应
                response.type = TASK_TYPE_REGISTER1_RESP_OK;
                response.length = salt.size();
                strncpy(response.data, salt.c_str(), salt.size());
            } else {
                // 用户名已存在
                response.type = TASK_TYPE_REGISTER1_RESP_ERROR;
                response.length = 0;
            }
        } else {
            // 查询失败
            response.type = TASK_TYPE_REGISTER1_RESP_ERROR;
            response.length = 0;
        }

        _conn->sendInLoop(response);
}


// 注册阶段2：密码存储
void UserRegisterSection2::process() {
        // 从数据包提取用户名和密码（格式为"username:password"）
        string data = _packet.msg;
        size_t colonPos = data.find(':');
        if (colonPos == string::npos) {
            // 数据包格式错误
            TLV response;
            response.type = TASK_TYPE_REGISTER2_RESP_ERROR;
            response.length = 0;
            _conn->sendInLoop(response);
            return;
        }

        string username = data.substr(0, colonPos);
        string encryptedCode = data.substr(colonPos + 1); // 直接使用客户端加密结果


        // 存储到数据库
        MySQLClient mysql;
        mysql.connect("192.168.10.50", 3306, "root", "ljc521", "ljc");

        string sql = "INSERT INTO user (name, encrypt) VALUES ('" + username + "', '" + encryptedCode + "')";
        if (mysql.writeOperationQuery(sql)) {
            // 注册成功
            TLV response;
            response.type = TASK_TYPE_REGISTER2_RESP_OK;
            response.length = 0;
            _conn->sendInLoop(response);
        } else {
            // 注册失败
            TLV response;
            response.type = TASK_TYPE_REGISTER2_RESP_ERROR;
            response.length = 0;
            _conn->sendInLoop(response);
        }
}

void CommonMessageTask::process() {
        MyLogger::getInstance().info("处理普通消息: %s", _packet.msg.c_str());
    
        // 构造TLV响应
        TLV response;
        string msg = "对于要求我已处理: " + _packet.msg;
        response.type = TASK_TYPE_COMMON_MESSAGE;
        response.length = msg.size();
        memcpy(response.data, msg.c_str(), msg.size());
        
        _conn->sendInLoop(response);
    }

} // end of namespace wd