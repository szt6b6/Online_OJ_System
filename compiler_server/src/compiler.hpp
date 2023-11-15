#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <iostream>
#include "../../comm/utils.hpp"
#include <Windows.h>
#include "../../comm/log.hpp"



namespace ns_compiler
{
    class Compiler
    {
    public:
        Compiler() {}
        ~Compiler() {}
        // 负责编译
        static bool Compile(const std::string& file_name)
        {
            /*
                return value:
                true: 编译成功
                false: 编译失败
            */

            // 重定向编译错误到编译错误文件
            std::string _compiler_error_file_name = ns_util::PathUtil::CompileError(file_name);
            FILE* compiler_errorStream; 
            freopen_s(&compiler_errorStream, _compiler_error_file_name.c_str(), "w", stderr);
            if (compiler_errorStream == NULL)
            {
                std::cout << GetLastError() << std::endl;
                return false;
            }

            // g++ -o target src -std=c++11 构造编译命令 并执行
            std::string compile_command = "g++ -o " + ns_util::PathUtil::Exe(file_name) + \
                + " " + ns_util::PathUtil::Src(file_name).c_str() + " -std=c++20";
            int res = system(compile_command.c_str());

            // 判断是否编译成功
            if (res == 0 && ns_util::FileUtil::IsFileExists(ns_util::PathUtil::Exe(file_name)))
            {
                fclose(compiler_errorStream);
                LOG(INOF) << " 编译成功\n";
                return true;
            }

            LOG(INOF) << "编译失败\n";

            // 重定向标准错误回来
            fflush(compiler_errorStream);
            fclose(compiler_errorStream);
            freopen_s(&compiler_errorStream, "CON", "w", stderr);
            return false;
        }
    };
}

