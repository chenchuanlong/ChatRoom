//
// Created by curtis on 2/1/19.
//

#ifndef CHAT_ROOM_SERVER_H
#define CHAT_ROOM_SERVER_H

#define gettid() syscall(__NR_gettid)

#include <string>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#include <map>

#include "Common.h"
#include "BlockingQueue.h"




class Server {
public:
    Server();
    ~Server();
    void  Init();
    void  Close();
    void Start();
    void CreateBrocastThreads(int threadNum);
    static void * BroadcastMessage(void * arg);
private:
    int recvMessage(int clientfd);
    struct sockaddr_in serverAddr;
    int listenFd;
    int epfd;
    pthread_rwlock_t rwlock_;
    std::map<int, ClientInfo> clients; // 连接列表 Guard by rwlock_
    std::map<std::string, int> nickname_clientfd; // 昵称到clientfd的映射
    BlockingQueue<Message_t> messageQueue; //消息队列

};


#endif //CHAT_ROOM_SERVER_H
