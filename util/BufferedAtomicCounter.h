// $Id: $

#ifndef PROFILER_BUFFEREDATOMICCOUNTER_H
#define PROFILER_BUFFEREDATOMICCOUNTER_H

/**
 * @file BufferedAtomicCounter.h
 * 
 * Tagged: Tue Sep 13 17:23:27 PDT 2016
 * 
 * @version: $Revision: $, $Date: $
 * 
 * @author /bin/bash: username: command not found
 */
#ifdef __APPLE__
#include <libkern/OSAtomic.h>
#endif

#include <string>
#include <vector>
#include <inttypes.h>

#include "export.h"

#ifdef PROFILER_EXPORTS
#include <windows.h>
#endif

namespace profiler {

    class BufferedAtomicCounter {
    public:

        //------------------------------------------------------------
        // A struct for managing a single 32-bit atomic counter
        //------------------------------------------------------------
    
        struct PROFILER_API AtomicCounter {
#ifdef __APPLE__
            int counts_;
#elif defined _WIN32

#ifdef PROFILER_EXPORTS
            volatile LONG counts_;
#else
            uint32_t counts_;
#endif

#else
            uint32_t counts_;
#endif
            AtomicCounter();
            void increment();
        };

        /**
         * Constructor.
         */
        PROFILER_API BufferedAtomicCounter();
        PROFILER_API BufferedAtomicCounter(unsigned int bufferSize, uint64_t intervalMs);

        PROFILER_API void setTo(unsigned int bufferSize, uint64_t intervalMs);
        PROFILER_API void increment(uint64_t currentMicroSeconds);
        PROFILER_API std::string dump(uint64_t currentMicroSeconds);
    
        /**
         * Destructor.
         */
        PROFILER_API virtual ~BufferedAtomicCounter();

    private:

        uint64_t minorIntervalMs_;
        uint64_t majorIntervalMs_;
    
        std::vector<std::vector<AtomicCounter> > counters_;
    
    }; // End class BufferedAtomicCounter

} // End namespace profiler



#endif // End #ifndef PROFILER_BUFFEREDATOMICCOUNTER_H
