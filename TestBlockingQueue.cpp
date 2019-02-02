
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "BlockingQueue.h"

#define gettid() syscall(__NR_gettid)

using  namespace std;

BlockingQueue<int> bq;

void * producer(void * arg){
    srand(time(NULL));
    while(1){
        int randNum = rand()%100;
        printf("tid:%ld produce:%d ; queue left:%ld \n", (long int) gettid(), randNum, (long int)bq.size());
        bq.put(randNum);
        sleep(1);
    }
}

void * consumer(void * arg){
    while (1){
        int takeNum = bq.take();
        printf("tid:%ld consume:%d ; queue left:%ld \n", (long int) gettid(), takeNum, (long int)bq.size());
        sleep(rand()%3);
    }
}

int main()
{
    pthread_t pthread_a, pthread_b;
    printf("hello\n");

    pthread_create(&pthread_b, NULL, producer, NULL);
    pthread_create(&pthread_a, NULL, consumer, NULL);
    pthread_join(pthread_a, NULL);
    pthread_join(pthread_b, NULL);

    return 0;
}
