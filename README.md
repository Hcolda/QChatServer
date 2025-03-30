[English](README_EN.md)
# 氢聊聊天服务器
这是一个轻量的氢聊聊天软件，  
如果你想下载氢聊的客户端，请打开[QingLiaoChatClient](https://github.com/Hcolda/QingLiaoChatClient)

## 帮助我们完善项目
**如果有大侠帮忙，我们就能更快实现这个目标！**  
目前在实现以下功能：
- [x] Network (with TLS-1.3)
- [ ] Manager (Database manager yet)
- [x] User
- [x] Room (private room and group room)
- [ ] SQL database connection
- [ ] Voice chat
- [ ] File transport
- [ ] Permission
- [ ] Chat bot library

## 构建方法
### 前置工具
你需要先拥有以下工具，才能正常构建：
1. c++编译器
2. CMake
3. [vcpkg](https://github.com/microsoft/vcpkg)

### 在CMake构建系统下构建
```cmd
cmake -S . -B build
cmake --build build --config Release
```

## 使用方法
### 1. 请用cmd打开服务器程序，之后会出现如下的文件  
**config/config.ini**
```ini
[server]
host=0.0.0.0 ;这是主机的地址
port=55555 ;这是主机端口
[ssl] ;为了服务器安全，强制开启SSL1.3协议
certificate_file=certs.pem ;证书pem文件
password= ;如果有密码就填密码，没有就不填
key_file=key.pem ;证书对应的私钥pem文件
dh_file=dh.pem ;可以不填，后面会删掉这个key
[mysql] ;sql服务器
host=127.0.0.1 ;sql服务器ip地址
port=3306 ;sql服务器端口
username= ;sql服务器的用户名
password= ;sql服务器的密码
```

### 2. 重新用cmd打开服务器程序
如果显示如下：
```cmd
[22:44:48][INFO]Server log system started successfully!
[22:44:48][INFO]The local endianness of the server is little-endian
[22:44:48][INFO]Reading configuration file...
[22:44:48][INFO]Certificate file path: certs.pem
[22:44:48][INFO]Password: empty
[22:44:48][INFO]Key file path: key.pem
[22:44:48][INFO]DH file path: dh.pem
[22:44:48][INFO]TLS configuration set successfully
[22:44:48][INFO]Configuration file read successfully!
[22:44:48][INFO]Loading serverManager...
[22:44:48][INFO]serverManager loaded successfully!
[22:44:48][INFO]Server command line starting...
[22:44:48][INFO]Server listener starting at address: 0.0.0.0:55555
```
则说明成功配置服务器
- 注意，如果证书不正确配置，可能也会成功启动服务器，但是客户端无法正常连接上服务器
- 证书尽量与服务器的域名一致，否则客户端会报警告

### 3. 打开客户端，即可使用（客户端现在跟不上服务端的开发进度，急需qt大佬帮忙）

## 文档
- [FormatForDataPackage.md](doc/FormatForDataPackage.md)
- ~[Website.md](doc/Website.md)~ (已经弃用)
