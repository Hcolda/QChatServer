[简体中文](README.md)
# QingLiao Chat Server
This is a lightweight chat server.  
If you want the client, see [QingLiaoChatClient](https://github.com/Hcolda/QingLiaoChatClient)

## Build
### Essential tools
1. [vcpkg](https://github.com/microsoft/vcpkg)

### Build with cmake
```cmd
cmake -S . -B build
cmake --build build --config Release
```

## TODO
- [x] Network (with TLS-1.3)
- [ ] Manager (Database manager yet)
- [x] User
- [x] Room (private room and group room)
- [ ] SQL database connection
- [ ] Voice chat
- [ ] File transport
- [ ] Permission
- [ ] Chat bot library

## Documents
- [FormatForDataPackage.md](doc/FormatForDataPackage_EN.md)

