#ifndef PROFILER_MUTEX_H
#define PROFILER_MUTEX_H

#include "export.h"
  
namespace profiler {
    
    // Forward declaration of the impl class

    class MutexImpl;

    //------------------------------------------------------------
    // Autoinitializing mutex object
    //------------------------------------------------------------

    class Mutex
    {
    public:

        PROFILER_API Mutex();
        PROFILER_API ~Mutex();
        PROFILER_API void Lock();
        PROFILER_API void Unlock();

    private:

        Mutex(const Mutex & rhs);             // no copy
        Mutex & operator=(const Mutex & rhs); // no assignment

        MutexImpl* impl_;

    };  // class Mutex

    //------------------------------------------------------------
    // Automatic lock and unlock of mutex
    //------------------------------------------------------------

    class MutexLock
    {
    protected:

        Mutex & mutex_;

    public:

        PROFILER_API MutexLock(Mutex & MutexObject);
        PROFILER_API ~MutexLock();

    private:

        MutexLock();                                  // no default constructor
        MutexLock(const MutexLock & rhs);             // no copy constructor
        MutexLock & operator=(const MutexLock & rhs); // no assignment constructor

    };  // class MutexLock

} // namespace profiler


#endif  // PROFILER_MUTEX_H
