// $Id: $

#ifndef NIFUTIL_PROFILER_H
#define NIFUTIL_PROFILER_H

/**
 * @file Profiler.h
 * 
 * Tagged: Tue Oct 27 10:44:14 PDT 2015
 * 
 * @version: $Revision: $, $Date: $
 * 
 * @author /bin/bash: username: command not found
 */
#include <string>
#include <map>
#include <vector>

#include <stdint.h>
#include <inttypes.h>

#include "mutex.h"

#include "RingPartition.h"

#define THREAD_START(fn) void* (fn)(void *arg)

namespace nifutil {

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
        

        virtual ~Profiler();

        unsigned start(std::string& label, bool perThread);
        void stop(std::string& label, bool perThread);

        std::string formatStats(bool crTerminated);
        void dump(std::string fileName);
        void setPrefix(std::string fileName);
        void debug();

        Counter& getCounter(std::string& label, bool perThread);

        static void noop(bool makeNoop);
        static int64_t getCurrentMicroSeconds();
        static Profiler* get();

        static unsigned profile(std::string command, std::string value, bool always);
        static unsigned profile(std::string command, std::string value, bool perThread, bool always);

        //------------------------------------------------------------
        // Time-resolved atomic counters
        //------------------------------------------------------------

        void startAtomicCounterTimer();
        void dumpAtomicCounters();

        static void addRingPartition(uint64_t ptr, std::string leveldbFile);
        
        static void initializeAtomicCounters(std::map<std::string, std::string>& nameMap,
                                             unsigned int bufferSize, uint64_t intervalMs,
                                             std::string fileName);

        static void incrementAtomicCounter(uint64_t partPtr, std::string counterName);

        static THREAD_START(runAtomicCounterTimer);
        
    private:

        Profiler();

        //------------------------------------------------------------
        // Members for normal counters
        //------------------------------------------------------------
        
        std::vector<pthread_t> getThreadIds();

        std::map<std::string, std::map<pthread_t, Counter> > countMap_;

        unsigned nAccessed_;

        Mutex mutex_;
        unsigned counter_;
        std::string prefix_;

        //------------------------------------------------------------
        // Members for time-resolved atomic counting
        //------------------------------------------------------------
        
        std::map<uint64_t, RingPartition> atomicCounterMap_;
        pthread_t atomicCounterTimerId_;
        uint64_t majorIntervalUs_;
        std::string atomicCounterOutput_;
        bool firstDump_;
        
        static Profiler instance_;
        static bool noop_;
        
    }; // End class Profiler

} // End namespace nifutil



#endif // End #ifndef NIFUTIL_PROFILER_H
