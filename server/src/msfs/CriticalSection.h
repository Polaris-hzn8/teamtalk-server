/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: CriticalSection.h
 Update Time: Thu 15 Jun 2023 00:46:52 CST
 brief: critical section
*/

#ifndef __CRITICALSECTION_H__
#define __CRITICALSECTION_H__

#include <pthread.h>

namespace msfs {

class CriticalSection {
public:
    CriticalSection()
    {
        pthread_mutexattr_t mutex_attribute;
        pthread_mutexattr_init(&mutex_attribute);
        pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_mutex, &mutex_attribute);
        pthread_mutexattr_destroy(&mutex_attribute);
    }
    ~CriticalSection() { pthread_mutex_destroy(&m_mutex); }
    void Enter() { pthread_mutex_lock(&m_mutex); }
    bool TryEnter()
    {
        if (pthread_mutex_trylock(&m_mutex) == 0) {
            return true;
        }
        return false;
    }
    void Leave() { pthread_mutex_unlock(&m_mutex); }

private:
    pthread_mutex_t m_mutex;
};

} // namespace msfs

#endif
