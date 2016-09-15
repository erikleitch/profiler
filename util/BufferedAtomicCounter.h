// $Id: $

#ifndef NIFUTIL_BUFFEREDATOMICCOUNTER_H
#define NIFUTIL_BUFFEREDATOMICCOUNTER_H

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
#include <CoreServices/CoreServices.h>
#else
#include <asm/atomic.h>
#endif

#include <vector>

namespace nifutil {

  class BufferedAtomicCounter {
  public:

    //------------------------------------------------------------
    // A struct for managing a single 32-bit atomic counter
    //------------------------------------------------------------
    
    struct AtomicCounter {
#ifdef __APPLE__
        int counts_;
#else
        atomic_t counts_;
#endif
        AtomicCounter();
        void increment();
    };

    /**
     * Constructor.
     */
    BufferedAtomicCounter();
    BufferedAtomicCounter(uint bufferSize, uint64_t intervalMs);

    void setTo(uint bufferSize, uint64_t intervalMs);
    void increment(uint64_t currentMicroSeconds);
    
    /**
     * Destructor.
     */
    virtual ~BufferedAtomicCounter();

  private:

    uint64_t minorIntervalMs_;
    uint64_t majorIntervalMs_;
    
    std::vector<std::vector<AtomicCounter> > counters_;
    
  }; // End class BufferedAtomicCounter

} // End namespace nifutil



#endif // End #ifndef NIFUTIL_BUFFEREDATOMICCOUNTER_H
