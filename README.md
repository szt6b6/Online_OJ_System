# Online_OJ_System

## 项目依赖 httplib, nlohmann_json, ctemplate
可以使用vcpkg安装后面两个依赖库  
使用`vcpkg install integrate`集成到 vs  


## 启动
1. 查看"oj_server\conf\service_machine.conf"文件中配置端口 在项目compiler_server根目录 启动负责编译和运行的服务  
   `.\x64\Debug\compiler_server.exe 8888`
2. 在项目oj_server根目录启动oj_server后端  
   `.\x64\Debug\oj_server.exe`
