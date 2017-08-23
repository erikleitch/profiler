#include "stdafx.h"
#include "Mutex.h"

#ifndef _WIN32
#include <pthread.h>
typedef pthread_mutex_t HANDLE;

#else

#include <windows.h>

#endif

namespace profiler {
    class MutexImpl {
        
    public:
        
        MutexImpl();
        ~MutexImpl();
        void Lock();
        void Unlock();
            
    protected:
            
        HANDLE mutex_;
    };
}
 
using namespace profiler;

MutexImpl::MutexImpl() {
#ifndef _WIN32 
    pthread_mutex_init(&mutex_, NULL);
#else
    mutex_ = CreateMutex(NULL, FALSE, NULL);
#endif
};
        
MutexImpl::~MutexImpl() {
#ifndef _WIN32
    pthread_mutex_destroy(&mutex_);
#else
    CloseHandle(mutex_);
#endif
};
        
void MutexImpl::Lock() {
#ifndef _WIN32
    pthread_mutex_lock(&mutex_);
#else
    WaitForSingleObject(mutex_, INFINITE);
#endif
};
        
void MutexImpl::Unlock() {
#ifndef _WIN32
    pthread_mutex_unlock(&mutex_);
#else
    ReleaseMutex(mutex_);
#endif
};

Mutex::Mutex() {
    impl_ = 0;
    impl_ = new MutexImpl();
}

Mutex::~Mutex() {
    if(impl_)
        delete impl_;
};

void Mutex::Lock() {
    impl_->Lock();
}

void Mutex::Unlock() {
    impl_->Unlock();
}

MutexLock::MutexLock(Mutex& mutex) : mutex_(mutex)
{
    mutex_.Lock();
}

MutexLock::~MutexLock() 
{
    mutex_.Unlock();
}

