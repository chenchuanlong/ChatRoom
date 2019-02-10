//
// Created by curtis on 2/1/19.
//

#ifndef CHAT_ROOM_COMMON_H
#define CHAT_ROOM_COMMON_H

#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888
#define EPOLL_SIZE 500
#define BUF_SIZE 0xFFFF
#define  ALL_CLIENTS 0
#define SERVER_WELCOME "SYSTEM MESSAGE: [%s] joined the chat room, welcome!"
#define SERVER_MESSAGE "[%s] say >> %s"
#define EXIT "EXIT"
#define CAUTION "SYSTEM MESSAGE: There is only one in the chat room!"
#define  CLIENT_LEAVE "SYSTEM MESSAGE: [%s] leaved the chat room"


struct Message_t{
    int sender;
    int receiver;     //  0 代表广播, 其他代表私聊
    std::string message;

    Message_t(){

    }
    Message_t(int sender_, int receiver_, std::string message_){
        sender = sender_;
        receiver = receiver_;
        message = message_;
    }
};

struct ClientInfo{
    int sockFd;
    std::string nickName;
    bool isNickNameset = false;
};

static void addfd(int epollfd, int fd, bool enable_et)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events  = EPOLLIN;
    if(enable_et)
        ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFD,0)| O_NONBLOCK);
    printf("fd added to epoll!\n");

}



#endif //CHAT_ROOM_COMMON_H
