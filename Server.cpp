#include <iostream>

#include "Server.h"


using namespace std;

Server::Server() {
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    listenFd = 0;
    epfd = 0;
    pthread_rwlock_init(&rwlock_, NULL);
}

Server::~Server() {
    pthread_rwlock_destroy(&rwlock_);
}

void Server::Init() {
    cout << "Init Server .." << endl;

    listenFd = socket(PF_INET, SOCK_STREAM, 0);
    if(listenFd < 0){
        perror("listenFd");
        exit(-1);
    }

    if( bind(listenFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
        perror("bind error");
        exit(-1);
    }

    int ret = listen(listenFd, 5);
    if(ret < 0){
        perror("listen error");
        exit(-1);
    }

    cout << "Start to listen: " << SERVER_IP << endl;
    epfd = epoll_create(EPOLL_SIZE);
    if(epfd < 0){
        perror("epfd error");
        exit(-1);
    }

    addfd(epfd, listenFd, true);

}

void Server::Close() {
    close(listenFd);

    close(epfd);
}

int Server::recvMessage(int clientfd) {
    char buf[BUF_SIZE], message[BUF_SIZE];
    bzero(buf, BUF_SIZE);
    bzero(message, BUF_SIZE);

    cout << "read from client(clientID = " << clientfd << ")" << endl;
    int len = static_cast<int>( recv(clientfd, buf, BUF_SIZE, 0) );

    //移除连接
    if(len == 0){
        pthread_rwlock_wrlock(&rwlock_);
        string nickName_ = clients[clientfd].nickName;
        close(clientfd);
        clients.erase(clientfd);
        cout << "ClientID = " << clientfd
             << " closed.\n Now there are "
             << clients.size()
             << " client in the chat room"
             << endl;
        pthread_rwlock_unlock(&rwlock_);

        nickname_clientfd.erase(nickName_);

        sprintf(message, CLIENT_LEAVE, nickName_.c_str());

        Message_t message_recv;
        message_recv.sender = clientfd;
        message_recv.receiver = ALL_CLIENTS;
        message_recv.message = message;
        messageQueue.put(message_recv);

    }else{




        //判断是不是客户端第一次发送的消息，是的话将其设置为昵称
        if(!clients[clientfd].isNickNameset){

            pthread_rwlock_wrlock(&rwlock_);
            clients[clientfd].nickName = buf;
            clients[clientfd].isNickNameset = true;
            pthread_rwlock_unlock(&rwlock_);

            nickname_clientfd[buf] = clientfd;

            sprintf(message, SERVER_WELCOME, clients[clientfd].nickName.c_str() );
            Message_t message_recv(listenFd, ALL_CLIENTS, message);
            messageQueue.put(message_recv);
            return  len;

        }


        if(clients.size()==1){
            send(clientfd, CAUTION, strlen(CAUTION), 0);
            return len;
        }


        //如果是私聊 @nick message
        string buf_str = buf;
        string::size_type pos_at = buf_str.find('@');
        if(pos_at != string::npos){
            string::size_type pos_space = buf_str.find(' ');
            assert(pos_space!=string::npos);
            string receiver_nickName = buf_str.substr(pos_at+1, pos_space-(pos_at+1));
            string message_ = buf_str.substr(pos_space+1, buf_str.size()-(pos_space+1));
            cout<<receiver_nickName<<":"<<message_<<endl;

            sprintf(message, SERVER_MESSAGE, clients[clientfd].nickName.c_str(), message_.c_str());
            int receiver_fd = nickname_clientfd[receiver_nickName];
            Message_t message_recv(clientfd, receiver_fd, message);
            messageQueue.put(message_recv);
            return  len;
        }

        sprintf(message, SERVER_MESSAGE, clients[clientfd].nickName.c_str(), buf);
        Message_t message_recv(clientfd, ALL_CLIENTS, message);
        messageQueue.put(message_recv);
    }
    return len;
}


void * Server::BroadcastMessage(void *arg) {

    //类成员函数不能作为pthread_create函数参数，所以通过传入this指针给static函数解决问题
    Server * pServer = (Server *) arg;
    assert(pServer != NULL);

    while(true){

        Message_t message_toSend = pServer->messageQueue.take();

        string message = message_toSend.message;
        const char * message_cstr = message.c_str();

        printf("thread-tid:%ld is broadcasting:%s\n",(long int) gettid(), message_cstr);

        pthread_rwlock_rdlock(& (pServer->rwlock_) );
        for(auto it = pServer->clients.begin(); it != pServer->clients.end(); ++it){
            if(it->first == message_toSend.sender) continue;   //发送者是自己
            if(it->first== message_toSend.receiver || message_toSend.receiver==ALL_CLIENTS)
                send(it->first, message_cstr, BUF_SIZE, 0);
        }
        pthread_rwlock_unlock(& (pServer->rwlock_) );

    }
}

void Server::CreateBrocastThreads(int threadNum){
    pthread_t pthreads[threadNum];
    for(int i=0; i<threadNum; ++i){
        pthread_create(&pthreads[i], NULL, BroadcastMessage, (void *)this);
    }
}


void Server::Start() {
    static struct epoll_event events[EPOLL_SIZE];

    Init();

    //Create consumer threads
     CreateBrocastThreads(3);

    while (true){

        int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);

        if(epoll_events_count < 0){
            perror("epoll failure");
            break;
        }

        cout << "epoll_events_count = " << epoll_events_count << endl;

        for(int i=0; i < epoll_events_count; ++i){

            int sockfd = events[i].data.fd;

            if(sockfd == listenFd){
                struct sockaddr_in client_address;
                socklen_t client_addrLength = sizeof(struct sockaddr_in);
                int clientfd = accept(listenFd, (struct sockaddr *)&client_address, &client_addrLength);

                cout << "client connection from: "
                     << inet_ntoa(client_address.sin_addr) << ":"
                     << ntohs(client_address.sin_port) << ", clientfd = "
                     << clientfd << endl;
                
                addfd(epfd, clientfd, true);

                ClientInfo clientInfo;
                clientInfo.sockFd = clientfd;
                pthread_rwlock_wrlock(&rwlock_);
                clients[clientfd] = clientInfo;
                cout << "Add new clientfd =  " << clientfd << " to poll" << endl;
                cout << "There are " << clients.size() << " clients in the chat room" << endl;
                pthread_rwlock_unlock(&rwlock_);
            }
            else{
                int ret = recvMessage(sockfd);
                if(ret < 0){
                    perror("Broadcast error");
                    Close();
                    exit(-1);
                }

            }

        }

    }

    Close();
}



