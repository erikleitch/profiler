// $Id: $

#ifndef PROFILER_PROFILER_H
#define PROFILER_PROFILER_H

/**
 * @file Profiler.h
 * 
 * Tagged: Tue Oct 27 10:44:14 PDT 2015
 * 
 * @version: $Revision: $, $Date: $
 * 
 * @author /bin/bash: username: command not found
 */
#include <windows.h>
#include <string>
#include <map>
#include <vector>

#include <stdint.h>
#include <inttypes.h>

#include "Mutex.h"

#include "RingPartition.h"

#ifndef _WIN32

#define THREAD_START(fn) void* (fn)(void *arg)
#define thread_self pthread_self
typedef pthread_t thread_id;

#else

#define THREAD_START(fn) DWORD WINAPI (fn)(LPVOID arg)
typedef DWORD thread_id;
#define thread_self GetCurrentThreadId

#endif


#include "export.h"

namespace profiler {

    class Profiler {
    public:

        enum CounterState {
            STATE_TRIGGERED,
            STATE_DONE
        };
        
        struct Counter {
            int64_t currentCounts_;
            int64_t deltaCounts_;

            int64_t currentUsec_;
            int64_t deltaUsec_;

            CounterState state_;

            unsigned errorCountUninitiated_;
            unsigned errorCountUnterminated_;

            void start(int64_t usec, unsigned count);
            void stop(int64_t usec, unsigned count);
            
            Counter();
        };
        

        PROFILER_API virtual ~Profiler();

        PROFILER_API unsigned start(std::string& label, bool perThread);
        PROFILER_API void stop(std::string& label, bool perThread);

        PROFILER_API std::string formatStats(bool crTerminated);
        PROFILER_API void dump(std::string fileName);
        PROFILER_API void setPrefix(std::string fileName);
        PROFILER_API void debug();

        PROFILER_API Counter& getCounter(std::string& label, bool perThread);

        PROFILER_API static void noop(bool makeNoop);
        PROFILER_API static int64_t getCurrentMicroSeconds();
        PROFILER_API static Profiler* get();

        PROFILER_API static unsigned profile(std::string command, bool perThread=false, bool always=false);
        PROFILER_API static unsigned profile(std::string command, std::string value, bool perThread=false, bool always=false);

        //------------------------------------------------------------
        // Time-resolved atomic counters
        //------------------------------------------------------------

        PROFILER_API void startAtomicCounterTimer();
        PROFILER_API void dumpAtomicCounters();

        PROFILER_API static void addRingPartition(uint64_t ptr, std::string leveldbFile);
        
        PROFILER_API static void initializeAtomicCounters(std::map<std::string, std::string>& nameMap,
                                             unsigned int bufferSize, uint64_t intervalMs,
                                             std::string fileName);

        PROFILER_API static void incrementAtomicCounter(uint64_t partPtr, std::string counterName);

#ifndef _WIN32
        static THREAD_START(runAtomicCounterTimer);
#else
        static DWORD WINAPI runAtomicCounterTimer(LPVOID arg);
#endif
        
    private:

        Profiler();

        //------------------------------------------------------------
        // Members for normal counters
        //------------------------------------------------------------
        
        std::vector<thread_id> getThreadIds();
        std::map<std::string, std::map<thread_id, Counter> > countMap_;

        unsigned nAccessed_;

        Mutex mutex_;
        unsigned counter_;
        std::string prefix_;

        //------------------------------------------------------------
        // Members for time-resolved atomic counting
        //------------------------------------------------------------
        
        std::map<uint64_t, RingPartition> atomicCounterMap_;
#ifndef _WIN32
        pthread_t atomicCounterTimerId_;
#else
        DWORD atomicCounterTimerId_;
#endif
        uint64_t majorIntervalUs_;
        std::string atomicCounterOutput_;
        bool firstDump_;
        
        static Profiler instance_;
        static bool noop_;
        
    }; // End class Profiler

} // End namespace profiler



#endif // End #ifndef PROFILER_PROFILER_H
