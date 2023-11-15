#define _CRT_SECURE_NO_WARNINGS
#pragma once

#include "oj_model.hpp"
#include <ctemplate/template.h>

namespace ns_view {
	using namespace ns_model;

	const std::string template_path = "html/";

	class View {
	public:

        // 负责将index相关的信息渲染进入html
        void IndexHtml(std::string* html) {
            std::string src_html = template_path + "index.html";

            ctemplate::TemplateDictionary root("index");

            ctemplate::Template* tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);
            tpl->Expand(html, &root);
        }

        // 负责将questions信息渲染进入html
		void AllExpandHtml(const std::vector<ns_model::Question>& questions, std::string* html) {
            // 1. all_questions.html 模板
            std::string src_html = template_path + "all_question.html";
            // 2. 构造root
            ctemplate::TemplateDictionary root("all_questions");
            for (const auto& q : questions)
            {
                // question_list 再html模板中有关键字
                /*
                    <!--表示多次循环里面的内容-->
                   {{#question_list}}
                   <tr>
                       <td>{{number}}</td>
                       <td>{{title}}</td>
                       <td>{{star}}</td>
                   </tr>

                   {{/question_list}}
                */
                ctemplate::TemplateDictionary* sub = root.AddSectionDictionary("question_list");
                sub->SetValue("number", q.__number);
                sub->SetValue("title", q.__title);
                sub->SetValue("star", q.__star);
            }
            // 3. 用src_html构造tpl
            ctemplate::Template* tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);

            // 4. 渲染数据进入html
            tpl->Expand(html, &root);
		}

        // 负责将question信息渲染进入html
		void OneExpandHtml(const ns_model::Question question, std::string* html) {
            std::string src_html = template_path + "one_question.html";
            ctemplate::TemplateDictionary root("one_question");

            root.SetValue("number", question.__number);
            root.SetValue("title", question.__title);
            root.SetValue("star", question.__star);
            root.SetValue("desc", question.__desc);
            root.SetValue("pre_code", question.__header);

            ctemplate::Template* tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);
            tpl->Expand(html, &root);
		}
	};
}