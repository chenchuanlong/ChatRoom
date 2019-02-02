#include <iostream>

#include "Server.h"


using namespace std;

Server::Server() {
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    listener = 0;
    epfd = 0;
}

void Server::Init() {
    cout << "Init Server .." << endl;

    listener = socket(PF_INET, SOCK_STREAM, 0);
    if(listener < 0){
        perror("listener");
        exit(-1);
    }

    if( bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
        perror("bind error");
        exit(-1);
    }

    int ret = listen(listener, 5);
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

    addfd(epfd, listener, true);

}

void Server::Close() {
    close(listener);

    close(epfd);
}

int Server::SendBroadcastMessage(int clientfd) {
    char buf[BUF_SIZE], message[BUF_SIZE];
    bzero(buf, BUF_SIZE);
    bzero(message, BUF_SIZE);

    cout << "read from client(clientID = " << clientfd << ")" << endl;
    int len = recv(clientfd, buf, BUF_SIZE, 0);

    if(len == 0){
        close(clientfd);
        clients_list.remove(clientfd);
        cout << "ClientID = " << clientfd
             << " closed.\n Now there are "
             << clients_list.size()
             << " client in the chat room"
             << endl;
    }else{
        if(clients_list.size()==1){
            send(clientfd, CAUTION, strlen(CAUTION), 0);
            return len;
        }

        sprintf(message, SERVER_MESSAGE, clientfd, buf);
        list<int>::iterator it;
        for(it = clients_list.begin(); it != clients_list.end(); ++it){
            if(*it != clientfd){
                if( send(*it, message, BUF_SIZE, 0) < 0){
                    return  -1;
                }
            }
        }

    }
    return len;
}

void Server::Start() {
    static struct epoll_event events[EPOLL_SIZE];

    Init();

    while (true){

        int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);

        if(epoll_events_count < 0){
            perror("epoll failure");
            break;
        }

        cout << "epoll_events_count = " << epoll_events_count << endl;

        for(int i=0; i < epoll_events_count; ++i){

            int sockfd = events[i].data.fd;

            if(sockfd == listener){
                struct sockaddr_in client_address;
                socklen_t client_addrLength = sizeof(struct sockaddr_in);
                int clientfd = accept(listener, (struct sockaddr *)&client_address, &client_addrLength);

                cout << "client connection from: "
                     << inet_ntoa(client_address.sin_addr) << ":"
                     << ntohs(client_address.sin_port) << ", clientfd = "
                     << clientfd << endl;

                addfd(epfd, clientfd, true);
                clients_list.push_back(clientfd);
                cout << "Add new clientfd =  " << clientfd << " to poll" << endl;
                cout << "There are " << clients_list.size() << " clients in the chat room" << endl;

                cout << "welcome message" << endl;
                char message[BUF_SIZE];
                bzero(message, BUF_SIZE);
                sprintf(message, SERVER_WELCOME, clientfd);
                int ret = send(clientfd, message, BUF_SIZE, 0);
                if(ret < 0){
                    perror("send error!");
                    Close();
                    exit(-1);
                }
            }
            else{
                int ret = SendBroadcastMessage(sockfd);
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



