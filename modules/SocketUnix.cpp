#include "SocketUnix.h"

SocketUnix::SocketUnix(const std::string &path, bool removeFile) {
    if(removeFile && std::filesystem::exists(path)) {
        std::filesystem::remove(path);
    }

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        throw std::runtime_error("socket error");
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
    this->path=path;
}

void SocketUnix::createServer() {
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        throw std::runtime_error("bind error");
    }

    const int backlog = 5;
    if (listen(sock, backlog) == -1) {
        throw std::runtime_error("listen error");
    }

    isServer = true;
}

int SocketUnix::switchConnection()
{
    clientSock=sock;
        const int backlog = 5;
        if (listen(sock, backlog) == -1) {
        throw std::runtime_error("listen error");
    }
    return sock;
}

void SocketUnix::connectToServer() {
    while(1)
    {
        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) != -1) {
            break;
         }        
    }
}

int SocketUnix::waitForClientConnection() {
    if (!isServer) return sock;

    if ((clientSock = accept(sock, NULL, NULL)) == -1) {
        throw std::runtime_error("accept error");
    }

    return clientSock;
}

SocketUnix::~SocketUnix() {
    if(isServer) {
        close(sock);
        std::filesystem::remove(path);
    } else {
        close(clientSock);
    }
}

void SocketUnix::send(const SocketMessage &message) {
    std::string strToSend = message.function + " " + message.jsonArguments + "\n";
    int fd = isServer ? clientSock : sock; 
    if (write(fd, strToSend.c_str(), strToSend.size()) == -1) {
        throw std::runtime_error("write error");
    }
}

void SocketUnix::send(const std::string message,const std::string arguments)
{
    SocketMessage smessage;
    smessage.function=message;
    smessage.jsonArguments=arguments;
    send(smessage);
}

std::vector<SocketMessage> SocketUnix::receive() {
    char buf[bufferSize];
    int fd = isServer ? clientSock : sock;
    int res = read(fd, buf, sizeof(buf));
    if (res <= 0) {
        return std::vector<SocketMessage>();
    }

    std::string received(buf, 0, res);
    std::vector<SocketMessage> ret;
    std::istringstream iss(received);
    std::string line;
    while (std::getline(iss, line)) {
        size_t posSpace = line.find(' ');
        SocketMessage socketMessage = {line.substr(0, posSpace), line.substr(posSpace + 1)};
        ret.push_back(socketMessage);
    }

    return ret;
}

int SocketUnix::getFileDescriptor() {
    if (clientSock!=-1)
        return clientSock;
    else
        return sock;
}

void SocketUnix::changeBufferSize(int size)
{
    this->bufferSize=size;
}
