#pragma once
#include <iostream>
#include <string>
#include "utils.hpp"


#define LOG(level) ns_log::Log(#level, __FILE__, __LINE__)

enum // 日志等级
{
    INFO,
    DEBUG,
    WARNING,
    ERR,
    FATAL,
};

namespace ns_log
{

    // Log() << "message" 输出固定格式的log
    std::ostream& Log(const std::string& level, const std::string& file_name, int line)
    {
        // 等级
        std::string message = "[";
        message += level;
        message += "]";
        // 文件
        message += "[";
        message += file_name;
        message += "]";
        // 行号
        message += "[";
        message += std::to_string(line);
        message += "]";
        // 时间
        message += "[";
        message += ns_util::TimeUtil::GetTimeStamp();
        message += "]";

        std::cout << message; 

        return std::cout;
    }
}