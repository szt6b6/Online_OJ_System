#include "httplib.h" // httplib.h include得放在Windows.h前面
#include "compiler.hpp"
#include "../../comm/utils.hpp"
#include "../../comm/log.hpp"
#include "runner.hpp"
#include <nlohmann/json.hpp>

using namespace std;
using namespace ns_util;
using namespace ns_compiler;
using namespace ns_runner;
using namespace httplib;
using namespace nlohmann;


std::string CodeToDesc(int code);
void Start(const std::string& in_json, std::string* out_json);



/*  负责编译和执行
    输入参数:
        input: 用户给自己提交的代码对应的输入
        code: 用户提交的代码对应的输入，不做处理
        cpu_limit: 时间要求
        mem_limit: 空间要求
    输出参数:
        status: 状态码
        reason: 请求结果
        stdout: 我的程序运行完的结果
        stderr: 运行完的错误结果
*/
void Start(const std::string& in_json, std::string* out_json) {
    json in_value = json::parse(in_json); // 最后再处理差错问题
    // 代码和输入
    std::string code = in_value["code"];
    std::string input = in_value["input"];
    // 时间限制和空间限制
    int cpu_limit = in_value["cpu_limit"];
    int mem_limit = in_value["mem_limit"];

    int status_code = 0;
    int runner_rt_code = 0;

    // 形成一个唯一文件名，然后把code写到临时文件里面去
    std::string file_name = FileUtil::UniqFileName();

    if (code.size() == 0)
    {
        // 用户没有提交代码 文件为空
        status_code = -1; 
        goto END;
    }

    if (!FileUtil::WriteFile(PathUtil::Src(file_name), code) || !FileUtil::WriteFile(PathUtil::Stdin(file_name), input))
    {
        // 未形成临时src源文件 或者 没形成in输入文件
        status_code = -2;
        goto END;
    }

    if (!Compiler::Compile(file_name))
    {
        // 未编译成功
        status_code = -3;
        goto END;
    }

    runner_rt_code = Runner::Run(file_name, cpu_limit, mem_limit);
    switch (runner_rt_code) {
        case run_state::ERR:
            status_code = -2;
            goto END;
        case run_state::OUTOFMEMORY:
            status_code = 1;
            goto END;
        case run_state::OUTOFTIME:
            status_code = 2;
            goto END;
        case run_state::SUCCESS:
            status_code = 0;
            goto END;
    }

END:
    json out_value;
    out_value["status"] = status_code;
    out_value["reason"] = CodeToDesc(status_code);

    if (status_code == 0) {
        // 流程全部成功
        std::string stdout_content;
        if(FileUtil::ReadFile(PathUtil::Stdout(file_name), &stdout_content, true))
            out_value["stdout"] = stdout_content;

        std::string stderr_content;
        if (FileUtil::ReadFile(PathUtil::Stderr(file_name), &stderr_content, true))
            out_value["stderr"] = stderr_content;
    }
    *out_json = out_value.dump();

    // 清理临时文件
    FileUtil::RemoveTempFiles(file_name);
}

std::string CodeToDesc(int code)
{
    // 状态码 -> 对应的描述
    std::string desc;
    switch (code)
    {
    case 0:
        desc = "编译和运行成功";
        break;
    case -1:
        desc = "提交代码为空";
        break;
    case -2:
        desc = "未知错误";
        break;
    case -3:
        desc = "编译错误";
        break;
    case 2:
        desc = "内存超出限制";
        break;
    case 1:
        desc = "时间超出限制";
        break;
    default:
        desc = "未知错误(code: " + std::to_string(code) + ")";
        break;
    }
    return desc;
}



int main(int argc, char* argv[]) {

    if (argc != 2) {
        cerr << "Usage: "
            << "\n\t" << argv[0] << " port" << std::endl;
        return 0;
    }

#if 0
    // R"()" raw string
    std::string in_json, out_json;
    json in_value;
    in_value["code"] = 
        R"(
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
using namespace std;


class Solution
{
public:
    bool isPalindrome(int x)
    {
        // write code here
        return true;
    }
};

int main(){
    cout << "你好" << endl;
    return 1;
}
)";
    in_value["input"] = "";
    in_value["cpu_limit"] = 100;
    in_value["mem_limit"] = 10;

    in_json = in_value.dump();

    LOG(INFO) << " input json: " << in_json << endl;

    Start(in_json, &out_json);

    LOG(INFO) << " output json: " << out_json << endl;
#endif
    

#if 1
    Server server;
    server.Get("/hello", [](const Request& req, Response& resp) {
        // 这里content-type: text/plain; //冒号后面的空格很重要 没有的效果是下载名字为hello内容为你好的文件
        resp.set_content("你好", "content-type: text/plain;");
        });

    server.Post("/compile_and_run", [](const Request& req, Response& resp) {
        string in_json = req.body;
        string out_json;

        if (!in_json.empty()) {
            Start(in_json, &out_json);
            resp.set_content(out_json, "content-type: application/json;");
       }
       });

    // 命令行 oj_server.exe 8888
    server.listen("127.0.0.1", atoi(argv[1]));
    //server.listen("127.0.0.1", 8888);
#endif
    
    return 1;
}