#include "stdafx.h"
#include "Mutex.h"

using namespace profiler;

Mutex::Mutex() {
#ifndef _WIN32 
   pthread_mutex_init(&mutex_, NULL);
#else
   mutex_ = CreateMutex(NULL, FALSE, NULL);
#endif
    }

Mutex::~Mutex() {
#ifndef _WIN32
    pthread_mutex_destroy(&mutex_);
#else
    CloseHandle(mutex_);
#endif
};

void Mutex::Lock() {
#ifndef _WIN32
    pthread_mutex_lock(&mutex_);
#else
    WaitForSingleObject(mutex_, INFINITE);
#endif
}

void Mutex::Unlock() {
#ifndef _WIN32
    pthread_mutex_unlock(&mutex_);
#else
    ReleaseMutex(mutex_);
#endif
}

MutexLock::MutexLock(Mutex& mutex) : mutex_(mutex)
{
    mutex_.Lock();
}

MutexLock::~MutexLock() 
{
    mutex_.Unlock();
}

