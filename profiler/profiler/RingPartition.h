// $Id: $

#ifndef PROFILER_RINGPARTITION_H
#define PROFILER_RINGPARTITION_H

/**
 * @file RingPartition.h
 * 
 * Tagged: Wed Sep 14 16:07:57 PDT 2016
 * 
 * @version: $Revision: $, $Date: $
 * 
 * @author /bin/bash: username: command not found
 */
#include "BufferedAtomicCounter.h"

#include <map>
#include <string>
#include <inttypes.h>

#include "export.h"

namespace profiler {

    class RingPartition {
    public:

        /**
         * Constructor.
         */
        PROFILER_API RingPartition();

        /**
         * Destructor.
         */
        PROFILER_API virtual ~RingPartition();

        PROFILER_API void incrementCounter(std::string name, uint64_t currentUs);
        PROFILER_API std::string dumpCounters(uint64_t currentUs);
        PROFILER_API std::string listTags();
        
        std::string leveldbFile_;
        std::map<std::string, BufferedAtomicCounter> counterMap_;
        
    }; // End class RingPartition

} // End namespace profiler



#endif // End #ifndef PROFILER_RINGPARTITION_H
