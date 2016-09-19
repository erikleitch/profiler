#include "BufferedAtomicCounter.h"
#include "exceptionutils.h"

#include <sstream>

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
    __sync_fetch_and_sub(&counts_, 0);
#endif
}

void BufferedAtomicCounter::AtomicCounter::increment()
{
#ifdef __APPLE__
    IncrementAtomic(&counts_);
#else
    __sync_fetch_and_add(&counts_, 1);
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

    unsigned int majorInd = (currentMicroSeconds / majorIntervalMs_ ) % 2;

    //------------------------------------------------------------
    // Which minor interval in the current major interval are we in?
    //------------------------------------------------------------

    unsigned int minorInd = (currentMicroSeconds % majorIntervalMs_) / minorIntervalMs_;

    COUT("Current MS = " << currentMicroSeconds << " major = " << majorIntervalMs_ << " majorInd = " << majorInd << " minorInd = " << minorInd);


    //------------------------------------------------------------
    // Increment the appropriate counter
    //------------------------------------------------------------
    
    counters_[majorInd][minorInd].increment();
}

std::string BufferedAtomicCounter::dump(uint64_t currentMicroSeconds)
{
    //------------------------------------------------------------
    // Which buffer are we currently dumping? (Are we in an even
    // or odd major interval since an absolute second boundary?)
    //------------------------------------------------------------

    std::ostringstream os;
    
    unsigned int majorInd = (currentMicroSeconds / majorIntervalMs_ + 1) % 2;
    std::vector<AtomicCounter>& vec = counters_[majorInd];
    for(unsigned i=0; i < vec.size(); i++) {
        os << vec[i].counts_ << " ";

        // And zero the counter
        
        vec[i].counts_ = 0;
    }

    return os.str();
}

    
