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

#include <string>
#include <map>
#include <vector>

#include <stdint.h>
#include <inttypes.h>

#include "Mutex.h"
#include "RingPartition.h"

//------------------------------------------------------------
// Platform-specific defs go here
//------------------------------------------------------------


#include "export.h"

namespace profiler {

    class ProfilerImpl;

    class Profiler {
    public:

        PROFILER_API static void noop(bool makeNoop);
        PROFILER_API static int64_t getCurrentMicroSeconds();

        PROFILER_API static unsigned profile(std::string command, bool perThread, bool always);
        PROFILER_API static unsigned profile(std::string command, std::string value, bool perThread, bool always);
        PROFILER_API static unsigned profileChar(const char* command, const char* value, bool perThread=false, bool always=false);

        //------------------------------------------------------------
        // Time-resolved atomic counters
        //------------------------------------------------------------


        PROFILER_API static void addRingPartition(uint64_t ptr, std::string leveldbFile);
        
        PROFILER_API static void initializeAtomicCounters(std::map<std::string, std::string>& nameMap,
                                             unsigned int bufferSize, uint64_t intervalMs,
                                             std::string fileName);

        PROFILER_API static void incrementAtomicCounter(uint64_t partPtr, std::string counterName);

        PROFILER_API static void printString(std::string str);
        PROFILER_API static void printStringRef(std::string& str);
        PROFILER_API static void printChar(const char* str);

        //------------------------------------------------------------
        // End public API        
        //------------------------------------------------------------

        virtual ~Profiler();

    private:

        Profiler();

    }; // End class Profiler

} // End namespace profiler



#endif // End #ifndef PROFILER_PROFILER_H
