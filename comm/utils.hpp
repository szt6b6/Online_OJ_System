#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <iostream>
#include <sys/stat.h>
#include <chrono>
#include <ctime>
#include <fstream>
#include <regex>

namespace ns_util
{
    const std::string temp_path_root = "out/"; // 临时文件路径

    // 路径工具
    class PathUtil
    {
    public:
        // 获得添加了后缀的文件路径
        static std::string AddSuffix(const std::string& file_name, const std::string& suffix)
        {
            std::string path_name = temp_path_root;
            path_name += file_name;
            path_name += suffix;
            return path_name;
        }

    public:
        // cpp 待编译文件路径
        static std::string Src(const std::string& file_name)
        {
            return AddSuffix(file_name, ".cpp");
        }
        // exe 编译生成文件路径
        static std::string Exe(const std::string& file_name)
        {
            return AddSuffix(file_name, ".exe");
        }

        // compile_error 编译错误文件路径
        static std::string CompileError(const std::string& file_name)
        {
            return AddSuffix(file_name, ".compile_error");
        }

        // stderr 标准错误文件路径
        static std::string Stderr(const std::string& file_name)
        {
            return AddSuffix(file_name, ".stderr");
        }

        // stdin 标准输入文件路径
        static std::string Stdin(const std::string& file_name)
        {
            return AddSuffix(file_name, ".stdin");
        }

        // stdout 标准输出文件路径
        static std::string Stdout(const std::string& file_name)
        {
            return AddSuffix(file_name, ".stdout");
        }
    };

    // 时间工具
    class TimeUtil {
    public:
        // 获得固定格式的时间
        static std::string GetTimeStamp() {
            // 获得系统当前时间
            auto now = std::chrono::system_clock::now();
            std::time_t now_time = std::chrono::system_clock::to_time_t(now);

            // 转化为估计格式字符串
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S");

            return oss.str();
        }

        // 获得毫秒形式的时间
        static std::string GetTimeStampMs() {
            // 获得系统当前
            auto now = std::chrono::system_clock::now();

            // 转化为毫秒形式输出
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
            auto milliseconds = duration.count();
            std::string str_milliseconds = std::to_string(milliseconds);

            return str_milliseconds;
        }
    };

    // 文件工具
    class FileUtil
    {
    public:
        // 判断文件是否存在
        static bool IsFileExists(const std::string& path_name)
        {
            struct stat st;
            if (stat(path_name.c_str(), &st) == 0)
            {
                return true;
            }
            return false;
        }

        // 利用获得毫秒时间来获得单一的文件名
        static std::string UniqFileName() {
            return TimeUtil::GetTimeStampMs();
        }

        // 将content 写入到 target文件中
        static bool WriteFile(const std::string& target, const std::string& content) {
            std::ofstream out(target);
            if (!out.is_open()) { return false; }

            out.write(content.c_str(), content.size());
            out.close();
            return true;
        }

        // 从target文件 读数据到content中，keep表示是否保留换行符
        static bool ReadFile(const std::string& target, std::string* content, bool keep = false){
            (*content).clear();
            std::ifstream in(target);
            if (!in.is_open()) { return false; }

            std::string line;
            while (std::getline(in, line)) {
                (*content) += line;
                (*content) += keep ? "\n" : "";
            }

            in.close();
            return true;
        }

        // 读取tail文件 需要去掉 #iclude "header.cpp"行
        static bool ReadTailFile(const std::string& target, std::string* content, bool keep = false) {
            (*content).clear();
            std::ifstream in(target);
            if (!in.is_open()) { return false; }

            std::string line;
            while (std::getline(in, line)) {
                if (line == R"(#include "header.cpp")") continue; // 跳过 #iclude "header.cpp"行
                (*content) += line;
                (*content) += keep ? "\n" : "";
            }

            in.close();
            return true;
        }

        // 清除产生的临时文件
        static void RemoveTempFiles(const std::string& file_name) {

            std::string src_file_name = PathUtil::Src(file_name);
            if (FileUtil::IsFileExists(src_file_name))
                remove(src_file_name.c_str());
            std::string compiler_error_file_name = PathUtil::CompileError(file_name);
            if (FileUtil::IsFileExists(compiler_error_file_name))
                remove(compiler_error_file_name.c_str());

            std::string execute_file_name = PathUtil::Exe(file_name);
            if (FileUtil::IsFileExists(execute_file_name))
                remove(execute_file_name.c_str());

            std::string stdin_file_name = PathUtil::Stdin(file_name);
            if (FileUtil::IsFileExists(stdin_file_name))
                remove(stdin_file_name.c_str());

            std::string stdout_file_name = PathUtil::Stdout(file_name);
            if (FileUtil::IsFileExists(stdout_file_name))
                remove(stdout_file_name.c_str());

            std::string stderr_file_name = PathUtil::Stderr(file_name);
            if (FileUtil::IsFileExists(stderr_file_name))
                remove(stderr_file_name.c_str());
        }
    };

    // 字符串类工具
    class StringUtil {
    public:
        /*
            str: 待分割字符串
            target: 返回分割后的字符串向量
            sep: ָ分割符号
        */
        // 利用正则匹配来分割字符
        static void SplitString(const std::string& str, std::vector<std::string>& target, const std::string& seq) {
            std::regex reg("("+seq+")" + "+");
            std::sregex_token_iterator pos(str.begin(), str.end(), reg, -1); // 0表示正选  -1表示反选
            decltype(pos) end;
            for (; pos != end; pos++) {
                target.push_back(pos->str());
            }
        }
    };

} // namespace ns_util