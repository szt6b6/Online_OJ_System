#define _CRT_SECURE_NO_WARNINGS
#pragma once

#include <iostream>
#include <string>
#include "oj_model.hpp"
#include "oj_view.hpp"
#include "dmalb.hpp"
#include "httplib.h"
#include "../../comm/log.hpp"
#include "../../comm/utils.hpp"
#include <nlohmann/json.hpp>

namespace ns_control
{
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_model;
    using namespace ns_view;
    using namespace nlohmann;

    // controller
    class Control
    {
    private:
        Model __model;
        View __view;

    public:

        // 用户请求首页时 返回首页渲染的html
        void index(std::string* html) {
            std::vector<Question> all_questions;
            __view.IndexHtml(html);
        }

        // 用户请求所有题目时 先获得所有的题目 然后渲染成html格式返回
        void AllQuestions(std::string* html) {
            std::vector<Question> all_questions;
            if (__model.GetAllQuestions(&all_questions)) {
                sort(all_questions.begin(), all_questions.end(), [](const Question& q1, const Question& q2) {
                    return atoi(q1.__number.c_str()) < atoi(q1.__number.c_str());
                    });
                // 根据模板html 将all_questions的内容渲染进去
                __view.AllExpandHtml(all_questions, html);
            }
            else {
                LOG(ERROR) << "获取所有题目失败..\n";
            }
        }

        // 用户请求一道题目时 先根据题号number获得该题目 然后渲染成html格式返回
        void OneQuestion(const std::string& number, std::string* html) {
            Question q;
            if (__model.GetOneQuestions(number, &q)) {
                __view.OneExpandHtml(q, html);
            }
            else {
                LOG(ERROR) << "获得题目 " << number << " 失败..\n";
            }
        }

        // 用户提交判题请求时 先根据题号从所有题目列表中获得题目相关信息 将用户提交的代码和文件tail中的代码
        // 一起组合成整体代码 加上额外信息组装成json 再转化成字符串 利用负载均衡 发送给编译运行服务的机器上编译和运行
        // 返回的结果存入out_json中
        void Judge(const std::string &number, const std::string in_json, std::string* out_json)
        {
            /*
                number: 题号
                in_json: 用户提交信息:
                    # 1. code: #include ...
                    # 2. input: ""
                out_json: 返回字符串格式的html
            */

            // 根据题号获得题目
            Question q;
            __model.GetOneQuestions(number, &q);
            
            // 从in_json中读消息
            json in_value = json::parse(in_json);

            // 拼装代码加上额外信息成compile_value
            json compile_value;
            compile_value["code"] = std::string(in_value["code"]) + q.__tail;
            compile_value["input"] = in_value["input"];
            compile_value["mem_limit"] = q.__mem_limit;
            compile_value["cpu_limit"] = q.__cpu_limit;

            // 将 compile_value 转成字符串 compile_str
            std::string compile_str = compile_value.dump();
            // 选择一个运行编译和运行服务的机器 将compile_str发送过去
            while (true) {
                int id = 0;
                Machine m;
                if (!loadBalance.int_select(&id, m)) {
                    loadBalance.OnlineMachine(); // 尝试重新上线所有主机
                    break;
                }
                LOG(INFO) << "发送消息, 主机id: " << id << " 主机号: "<< m.__ip << ":" << m.__port << "\n";

                // 以http的方式 把字符串发给机器
                httplib::Client compile_machine(m.__ip, m.__port);
                m.IncLoad();
                if (auto res = compile_machine.Post("/compile_and_run", compile_str, "application/json;")) {
                    // 将机器返回的内容塞进out_json里面
                    *out_json = res->body;
                    m.DecLoad();
                    break;
                }
                else {
                    // 机器没有回应
                    LOG(INFO) << "机器 id: " << id << "地址: "
                        << m.__ip << ":" << m.__port << " 为下线状态"
                        << "\n";
                    loadBalance.OfflineMachine(id); // 将没有回应的机器添加到offline中
                }
            }
        }
    };
} // namespace ns_control