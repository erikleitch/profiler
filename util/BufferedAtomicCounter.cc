#include "BufferedAtomicCounter.h"
#include "exceptionutils.h"

using namespace std;

using namespace nifutil;

//=======================================================================
// BufferedAtomicCounter::AtomicCounter
//=======================================================================

BufferedAtomicCounter::AtomicCounter::AtomicCounter()
{
#ifdef __APPLE__
    counts_ = 0;
#else
    atomic_set(&counts_);
#endif
}

void BufferedAtomicCounter::AtomicCounter::increment()
{
#ifdef __APPLE__
    IncrementAtomic(&counts_);
#else
    atomic_inc(&counts_);
#endif
}

//=======================================================================
// BufferedAtomicCounter
//=======================================================================

/**.......................................................................
 * Constructor.
 */
BufferedAtomicCounter::BufferedAtomicCounter()
{
    setTo(0, 0);
}

BufferedAtomicCounter::BufferedAtomicCounter(unsigned int bufferSize, uint64_t intervalMs)
{
    setTo(bufferSize, intervalMs);
}

void BufferedAtomicCounter::setTo(unsigned int bufferSize, uint64_t intervalMs)
{
    counters_.resize(2);
    counters_[0].resize(bufferSize);
    counters_[1].resize(bufferSize);

    minorIntervalMs_ = intervalMs;
    majorIntervalMs_ = intervalMs * bufferSize;
}

/**.......................................................................
 * Destructor.
 */
BufferedAtomicCounter::~BufferedAtomicCounter() {}

/**.......................................................................
 * Increment the right counter for the current time
 */
void BufferedAtomicCounter::increment(uint64_t currentMicroSeconds)
{
    //------------------------------------------------------------
    // Do nothing if not initialized
    //------------------------------------------------------------
    
    if(majorIntervalMs_ == 0)
        return;
    
    //------------------------------------------------------------
    // Which buffer are we currently incrementing? (Are we in an even
    // or odd major interval since an absolute second boundary?)
    //------------------------------------------------------------

    uint majorInd = (currentMicroSeconds / majorIntervalMs_ ) % 2;

    //------------------------------------------------------------
    // Which minor interval in the current major interval are we in?
    //------------------------------------------------------------

    uint minorInd = (currentMicroSeconds % majorIntervalMs_) / minorIntervalMs_;

    COUT("Current MS = " << currentMicroSeconds << " major = " << majorIntervalMs_ << " majorInd = " << majorInd << " minorInd = " << minorInd);


    //------------------------------------------------------------
    // Increment the appropriate counter
    //------------------------------------------------------------
    
    counters_[majorInd][minorInd].increment();
}


    
