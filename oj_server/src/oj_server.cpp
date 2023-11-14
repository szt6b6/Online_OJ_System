#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "httplib.h"
#include "oj_control.hpp"
#include "../../comm/log.hpp"
#include "../../comm/utils.hpp"
#include <ctemplate/template.h>


using namespace std;
using namespace httplib;
using namespace ns_util;
using namespace ns_control;

int main(int argc, char* argv[]) {

#if 0
	// 测试分割字符串
	vector<string> strList;
	string str("Thisabisabaababtest");
	StringUtil::SplitString(str, strList, "ab");
	for (auto s : strList)
		cout << s << "*" << endl;
	cout << endl;

#endif

#if 0
	// 测试html ctemplate使用
	std::string in_html = "html/one_question.html";
	std::string value = "你好";

	ctemplate::TemplateDictionary root("test"); // 类似unordered_map
	root.SetValue("title", value);                // test.insert({})

	ctemplate::Template* tpl = ctemplate::Template::GetTemplate(in_html, ctemplate::DO_NOT_STRIP);

	// 渲染out_html
	std::string out_html;
	tpl->Expand(&out_html, &root);
	std::cout << out_html << std::endl;
	return 0;

#endif



#if 1
	
	Server server;
	Control controller;

	// 查看首页内容
	server.Get(R"(/|(/index))", [&](const Request& req, Response& resp) {
		string html;
		controller.index(&html);
		resp.set_content(html, "text/html;");
		});

	// 查看所有问题
	server.Get("/all_questions", [&](const Request& req, Response& resp) {
		string html;
		controller.AllQuestions(&html);
		resp.set_content(html, "text/html;");
		});

	// 查看某一问题
	server.Get(R"(/question/(\d+))", [&](const Request& req, Response& resp) {
		string number = req.matches[1]; // 取出后面的数字
		// string number = req.matches[0]; // 数字0结果是 /question/(\d+) 整体匹配
		string html;
		controller.OneQuestion(number, &html);
		resp.set_content(html, "text/html;");
		});

	// 编译和运行某问题
	server.Post(R"(/judge/(\d+))", [&](const Request& req, Response& resp) {
		string number = req.matches[1];
		string in_json = req.body;
		string html;
		controller.Judge(number, in_json, &html);
		resp.set_content(html, "application/json;");
		});

	// 开启http服务 监听
	server.listen("127.0.0.1", 8080);
	return 1;
#endif

}