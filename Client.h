#ifndef CHAT_ROOM_CLIENT_H
#define CHAT_ROOM_CLIENT_H

#include <string.h>
#include "Common.h"

using namespace std;

class Client{
public:
    Client();
    void Connect();
    void Close();
    void Start();

private:
    int sock;
    int pid;
    int epfd;
    int pipe_fd[2];
    bool isClientwork;
    char message[BUF_SIZE];
    struct  sockaddr_in serverAddr;

};



#endif //CHAT_ROOM_CLIENT_H
