#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <iostream>
#include <mutex>
#include <cassert>
#include <fstream>
#include "../../comm/log.hpp"


/*Distribute Machine And Load Balance Function*/
// 负责编译和运行服务的机器
class Machine
{
public:
    std::string __ip;  // ip
    int __port;        // port
    uint64_t __load;   // 负载数量
    std::mutex* __mtx; // C++ mutex 
public:
    Machine() : __ip(""), __port(0), __load(0), __mtx(nullptr) {}
    Machine(std::string ip, int port) : __ip(ip), __port(port), __load(0), __mtx(new std::mutex()) {}
    ~Machine() {}
    // 增加负载
    void IncLoad()
    {

        __mtx->lock();
        __load++;
        __mtx->unlock();
    }
    // 较少负载
    void DecLoad()
    {

        __mtx->lock();
        __load--;
        __mtx->unlock();
    }
    // 获得当前负载
    uint64_t GetLoad()
    {
        uint64_t cur_load = 0;

        __mtx->lock();
        cur_load = this->__load;
        __mtx->unlock();
        return cur_load;
    }
};


/* 负责编译和运行服务的及其列表文件配置路径 格式为 ip:port */
const std::string service_machine_path = "conf/service_machine.conf";
// 负载均衡处理类
class LoadBalance
{
private:
    std::vector<Machine> __machines; // 所有编译运行机器 下标就是id
    std::vector<int> __online;       // 在线机器id
    std::vector<int> __offline;      // 离线机器id
    std::mutex* __mtx;
public:
    LoadBalance(){
        assert(this->LoadConf(service_machine_path));
    }
    ~LoadBalance() {}

public:
    // 读入conf文件 初始化__machines
    bool LoadConf(const std::string& machine_conf) {
        __mtx = new std::mutex();

        std::ifstream in(machine_conf);
        if (!in.is_open()) {
            LOG(FATAL) << "加载配置: " << machine_conf << "失败" << "\n";
            return false;
        }

        std::string line;
        while (std::getline(in, line)) {
            std::vector<std::string> machine_infos;
            ns_util::StringUtil::SplitString(line, machine_infos, ":"); // 根据:分割每行配置获得ip和端口
            if (machine_infos.size() != 2) {
                LOG(WARNING) << "分割" << line << "失败\n";
                continue;
            }

            // 分割正确 加入机器列表中
            Machine m(machine_infos[0], atoi(machine_infos[1].c_str()));
            __online.push_back(__machines.size()); // 下表就是机器id
            __machines.push_back(m);

        }
        in.close();
        return true;
    }
    // 智能选择机器
    bool int_select(int* id, Machine& m){
        // 1.选择负载最小的

        /*
            id: 结果选择id
            m: 结果选择机器
        */

        __mtx->lock();

        int online_num = __online.size();
        if (online_num == 0) {
            __mtx->unlock();
            LOG(FATAL) << "无在线机器可能，运维人员请检查负责编译运行服务的机器是否正常工作\n";
            return false;
        }

        *id = __online[0];
        m = __machines[__online[0]];
        uint64_t min_load = m.GetLoad();
        for (int i = 1; i < __online.size(); i++) {
            uint64_t temp_load = __machines[__online[i]].GetLoad();
            if (min_load > temp_load) {
                min_load = temp_load;
                *id = __online[i];
                m = __machines[__online[i]];
            }
        }
        __mtx->unlock();
        return true;
    }

    // 使得id的机器离线
    void OfflineMachine(int id){
        __mtx->lock();
        for (auto iter = __online.begin(); iter != __online.end(); iter++) {
            if (*iter == id) {
                __online.erase(iter);
                __offline.push_back(id);
                break;
            }
        }
        __mtx->unlock();
        
    }
    // 将所有主机上线
    void OnlineMachine(){
        LOG(INFO) << "尝试重新将离线主机加入到在线主机列表，运维人员请尝试重新上线下列主机是否在线\n";

        __mtx->lock();
        for (auto offline_num : __offline) std::cout << __machines[offline_num].__ip << ":" << __machines[offline_num].__port << "\n";
        __online.insert(__online.end(), __offline.begin(), __offline.end());
        __offline.erase(__offline.begin(), __offline.end());
        __mtx->unlock();
    }
};

LoadBalance loadBalance;