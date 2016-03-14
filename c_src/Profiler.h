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

        /**
         * Destructor.
         */
        virtual ~Profiler();

        void resize(unsigned n);
        unsigned start(unsigned index);
        unsigned start(unsigned index, std::string label);

        void stop(unsigned index, unsigned count=0);
        void append(std::string fileName);
        void setPrefix(std::string fileName);

        static int64_t getCurrentMicroSeconds();
        static Profiler* get();

    private:

        Profiler();

        std::map<pthread_t, std::vector<int64_t>* >     currCounts_;
        std::map<pthread_t, std::vector<int64_t>* >     deltaCounts_;

        std::map<pthread_t, std::vector<int64_t>* >     usecCurr_;
        std::map<pthread_t, std::vector<int64_t>* >     deltas_;

        std::map<pthread_t, std::vector<std::string>* > labels_;

        std::vector<int64_t>*     getCurrCounts();
        std::vector<int64_t>*     getDeltaCounts();

        std::vector<int64_t>*     getCurr();
        std::vector<int64_t>*     getDeltas();

        std::vector<std::string>* getLabels();

        unsigned size_;
        unsigned nAccessed_;

        static Profiler instance_;

        Mutex mutex_;
        unsigned counter_;
        std::string prefix_;

    }; // End class Profiler

} // End namespace nifutil



#endif // End #ifndef NIFUTIL_PROFILER_H
