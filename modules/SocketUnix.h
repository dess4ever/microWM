#pragma once
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <sstream>

struct SocketMessage {
    std::string function;
    std::string jsonArguments;
};

class SocketUnix {
private:
    int sock{-1};
    int clientSock{-1};
    struct sockaddr_un addr;
    bool isServer = false;
    bool isAsync;
    int bufferSize{1000000};
    std::string path;


public:
    explicit SocketUnix(const std::string &path,bool removeFile);
    void createServer();
    void connectToServer();
    int waitForClientConnection();
    ~SocketUnix();
    void send(const SocketMessage &message);
    void send(const std::string message,const std::string arguments);
    std::vector<SocketMessage> receive();
    int getFileDescriptor();
    void changeBufferSize(int newSize);
    void startServerAsync();
    int updateConnection();
    int switchConnection();
};