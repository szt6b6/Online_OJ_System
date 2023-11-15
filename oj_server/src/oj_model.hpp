#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <cassert>
#include "../../comm/log.hpp"
#include "../../comm/utils.hpp"


namespace ns_model
{
    using namespace ns_log;
    using namespace ns_util;

    struct Question
    {
    public:
        std::string __number; // 题目号
        std::string __title;  // 题目名称
        std::string __star;   // 题目难度等级 简单 中等 困难
        int __cpu_limit;      // 耗时限制 单位MB
        int __mem_limit;      // 内存使用限制 单位ms
        // 上面五个是question.list文件中一行的内容 以空格为分隔符
        std::string __desc;   // 题目描述 如 desc.txt
        std::string __header; // 模板文件 如 header.cpp
        std::string __tail;   // 测试代码文件 如 tail.cpp
    };

    const std::string question_list_root = "questions/question.list";
    const std::string question_path = "questions/";

    class Model
    {
    private:
        // 题目号 题目
        std::unordered_map<std::string, Question> __questions;

    public:
        Model()
        {
            assert(LoadQuestionList(question_list_root));
        }
        ~Model() {}

    public:
        // 从question.list文件中载入所有的问题
        bool LoadQuestionList(const std::string& question_list)
        {
            std::ifstream in(question_list);
            if (!in.is_open())
            {
                LOG(FATAL) << "打开文件" << question_list_root << "失败"
                    << "\n";
                return false;
            }

            std::string line;
            while (std::getline(in, line))
            {
                // 读入文件的每行
                // 每个每行信息 添加题目
                std::vector<std::string> tokens;
                StringUtil::SplitString(line, tokens, " ");
                if (tokens.size() != 5)
                {
                    LOG(WARNING) << "分割 " << line << " 失败"
                        << "\n";
                    continue;
                }
                Question q;
                q.__number = tokens[0];
                q.__title = tokens[1];
                q.__star = tokens[2];
                q.__cpu_limit = atoi(tokens[3].c_str());
                q.__mem_limit = atoi(tokens[4].c_str());

                std::string question_number_path = question_path;
                question_number_path += q.__number + "/";

                FileUtil::ReadFile(question_number_path + "desc.txt", &(q.__desc), true);
                FileUtil::ReadFile(question_number_path + "header.cpp", &(q.__header), true);
                // 这里q.__tail跳过了读入ail.cpp文件中的#include "header.cpp"
                /*另一种方法是tail.cpp 这样include, 然后编译的时候命令加上参数 -D COMPILE_ONLINE
                * #ifndef COMPILE_ONLINE
                * #include "header.cpp"
                * #endif
                */
                FileUtil::ReadTailFile(question_number_path + "tail.cpp", &(q.__tail), true);

                __questions.insert({ q.__number, q });
                LOG(INFO) << "添加题目 " << q.__number + "成功\n";
            }

            in.close();
            return true;
        }

        // 从__questions中获得所有题目
        bool GetAllQuestions(std::vector<Question>* out)
        {
            if (__questions.size() == 0)
            {
                LOG(ERROR) << "题目为空"
                    << "\n";
                return false;
            }
            for (const auto& q : __questions)
            {
                out->push_back(q.second);
            }

            return true;
        }

        // 根据题号number从__questions中获得对应题目
        bool GetOneQuestions(const std::string& number, Question* q)
        {
            const auto& iter = __questions.find(number);
            if (iter == __questions.end())
            {
                LOG(ERROR) << "题库中没有题目: " << number << "\n";
                return false;
            }
            (*q) = iter->second;
            return true;
        }
    };
};