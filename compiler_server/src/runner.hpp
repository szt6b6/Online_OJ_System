#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <iostream>
#include <string>
#include "../../comm/utils.hpp"
#include <Windows.h>
#include "../../comm/log.hpp"
#include <io.h>


namespace ns_runner {
	using namespace ns_util;

    enum run_state {
        ERR,
        OUTOFMEMORY,
        OUTOFTIME,
        SUCCESS
    };

    // window上执行cmd的另一种方式 能够限制时间和内存的使用
    int createProcess(const std::string& command, int time_limit, int memory_limit) {
        
        HANDLE hJob = CreateJobObject(NULL, NULL);
        if (hJob == NULL) {
            std::cerr << "CreateJobObject failed with error: " << GetLastError() << std::endl;
            return run_state::ERR;
        }

        
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobLimits = { 0 };
        jobLimits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_JOB_MEMORY;
        jobLimits.JobMemoryLimit = memory_limit * 1024 * 1024; // 限制 memory_limit 但闻MB  

        if (!SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jobLimits, sizeof(jobLimits))) {
            std::cerr << "SetInformationJobObject failed with error: " << GetLastError() << std::endl;
            CloseHandle(hJob);
            return run_state::OUTOFMEMORY;
        }

        
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        std::wstring wideStr = std::wstring(command.begin(), command.end());

        int bufferSize = wideStr.length() + 1;
        LPWSTR lpwstr = new wchar_t[bufferSize];
        wcscpy_s(lpwstr, bufferSize, wideStr.c_str());
        if (!CreateProcess(NULL, lpwstr, NULL, NULL, false, 0, NULL, NULL, &si, &pi)) {
            std::cerr << "CreateProcess failed with error: " << GetLastError() << std::endl;
            CloseHandle(hJob);
            delete[](lpwstr);
            return run_state::ERR;
        }
        delete[](lpwstr);

        
        if (AssignProcessToJobObject(hJob, pi.hProcess) == 0) {
            std::cerr << "AssignProcessToJobObject failed with error: " << GetLastError() << std::endl;
            CloseHandle(hJob);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return run_state::ERR;
        }

        // 限制时间
        int finish_before_timeout = WaitForSingleObject(pi.hProcess, time_limit);

         
        CloseHandle(hJob);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (finish_before_timeout == 0) {
            return run_state::SUCCESS;
        }
        else {
            return run_state::OUTOFTIME;
        }
    }

	class Runner {
	public:
		Runner() {};
		~Runner() {};

		// 执行编译生成的exe文件
		static int Run(const std::string& file_name, int time_limit, int memory_limit) {
			std::string _execute_file_name = PathUtil::Exe(file_name);
			std::string _stdin_file_name = PathUtil::Stdin(file_name);
			std::string _stdout_file_name = PathUtil::Stdout(file_name);
			std::string _stderr_file_name = PathUtil::Stderr(file_name);


            // 重定向运行时标准输出 输入 错误到各自的文件中
            FILE* stdoutStream; freopen_s(&stdoutStream, _stdout_file_name.c_str(), "w", stdout);
            FILE* stderrStream; freopen_s(&stderrStream, _stderr_file_name.c_str(), "w", stderr);
            FILE* stdinStream; freopen_s(&stdinStream, _stdin_file_name.c_str(), "r", stdin);

			if (stdinStream == NULL || stdoutStream == NULL || stderrStream == NULL) return -1;

			// 运行exe文件
			//int run_status = system(_execute_file_name.c_str());
            int run_status = createProcess(_execute_file_name, time_limit, memory_limit);
            
            fflush(stdoutStream);
            fflush(stderrStream);
            

            // 把上面重定向到文件的流给重定向回来
            freopen_s(&stdoutStream, "CON", "w", stdout); 
            freopen_s(&stdinStream, "CON", "r", stdin);
            freopen_s(&stderrStream, "CON", "w", stderr);

            if (run_status == run_state::SUCCESS) LOG(INFO) << " 运行成功\n";
            return run_status;
		};
	};
}