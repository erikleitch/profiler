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

#include "mutex.h"

namespace nifutil {

    class Profiler {
    public:

        struct Counter {
            int64_t currentCounts_;
            int64_t deltaCounts_;

            int64_t currentUsec_;
            int64_t deltaUsec_;
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

    private:

        Profiler();
        std::vector<pthread_t> getThreadIds();

        std::map<std::string, std::map<pthread_t, Counter> > countMap_;

        unsigned nAccessed_;

        static Profiler instance_;

        Mutex mutex_;
        unsigned counter_;
        std::string prefix_;

        static bool noop_;
        
    }; // End class Profiler

} // End namespace nifutil



#endif // End #ifndef NIFUTIL_PROFILER_H
