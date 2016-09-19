// $Id: $

#ifndef NIFUTIL_RINGPARTITION_H
#define NIFUTIL_RINGPARTITION_H

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

namespace nifutil {

    class RingPartition {
    public:

        /**
         * Constructor.
         */
        RingPartition();

        /**
         * Destructor.
         */
        virtual ~RingPartition();

        void incrementCounter(std::string name, uint64_t currentUs);
        std::string dumpCounters(uint64_t currentUs);
        std::string listTags();
        
        std::string leveldbFile_;
        std::map<std::string, BufferedAtomicCounter> counterMap_;
        
    }; // End class RingPartition

} // End namespace nifutil



#endif // End #ifndef NIFUTIL_RINGPARTITION_H
