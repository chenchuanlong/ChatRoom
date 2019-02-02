//
//    阻塞队列
//

#ifndef CHAT_ROOM_BLOCKINGQUEUE_H
#define CHAT_ROOM_BLOCKINGQUEUE_H


#include <deque>
#include <pthread.h>
#include <assert.h>


template <typename T>
class BlockingQueue{
public:

    BlockingQueue(){
        pthread_mutex_init(&mutex_, NULL);
        pthread_cond_init(&cond_, NULL);
    }

    ~BlockingQueue(){
        pthread_mutex_destroy(&mutex_);
        pthread_cond_destroy(&cond_);
    }

    void put(const T x){
        pthread_mutex_lock(&mutex_);
        deque_.push_back(x);
        pthread_mutex_unlock(&mutex_);
        pthread_cond_signal(&cond_);
    }

    T take(){
        pthread_mutex_lock(&mutex_);

        while(deque_.empty()){ // 使用while是为了防止虚假唤醒
            pthread_cond_wait(&cond_, &mutex_);
        }
        assert(deque_.empty()== false);
        T ret = deque_.front();
        deque_.pop_front();
        pthread_mutex_unlock(&mutex_);

        return  ret;
    }

    size_t size()  {
        pthread_mutex_lock(&mutex_);
        size_t sz = deque_.size();
        pthread_mutex_unlock(&mutex_);
        return  sz;
    }


private:
    pthread_mutex_t mutex_;
    pthread_cond_t cond_;
    std::deque<T>  deque_;

};



#endif //CHAT_ROOM_BLOCKINGQUEUE_H
