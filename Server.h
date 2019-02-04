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

#include "Common.h"
#include "BlockingQueue.h"



using namespace std;

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
    int SendBroadcastMessage(int clientfd);
    struct sockaddr_in serverAddr;
    int listener;
    int epfd;
    pthread_rwlock_t rwlock_;  //
    list<int> clients_list;  // 连接列表 Guard by rwlock_
    BlockingQueue<string> messageQueue; //消息队列

};


#endif //CHAT_ROOM_SERVER_H
