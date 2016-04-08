#include "Profiler.h"
#include "exceptionutils.h"

#include <sstream>
#include <fstream>
#include <sys/time.h>

using namespace nifutil;
using namespace std;

Profiler Profiler::instance_;
bool     Profiler::noop_ = false;

Profiler::Counter::Counter()
{
    currentCounts_ = 0;
    deltaCounts_   = 0;

    currentUsec_   = 0;
    deltaUsec_     = 0;

    state_ = STATE_DONE;
    errorCountUninitiated_  = 0;
    errorCountUnterminated_ = 0;
}

void Profiler::Counter::start(int64_t usec, unsigned count)
{
    // Only set the time if the last trigger is done
    
    if(state_ == STATE_DONE) {
        currentUsec_   = usec;
        state_ = STATE_TRIGGERED;
    } else {
        errorCountUnterminated_++;
    }
    
    currentCounts_ = count;
}

void Profiler::Counter::stop(int64_t usec, unsigned count)
{
    // Only increment this counter if it was in fact triggered prior
    // to this call
    
    if(state_ == STATE_TRIGGERED) {
        deltaUsec_ += (usec - currentUsec_);
        
        // Keep track of the number of times the Profiler registers
        // have been accessed since the current counter was started.
        
        deltaCounts_ += (count - currentCounts_);

        state_ = STATE_DONE;
    } else {
        errorCountUninitiated_++;
    }
}

/**.......................................................................
 * Constructor.
 */
Profiler::Profiler() 
{
    nAccessed_  = 0;
    counter_    = 0;
    
    setPrefix("/tmp/");
}

/**.......................................................................
 * Destructor.
 */
Profiler::~Profiler() 
{
    std::ostringstream os;
    os << prefix_ << "/" << this << "_profile.txt";
    dump(os.str());
}

Profiler::Counter& Profiler::getCounter(std::string& label, bool perThread)
{
    pthread_t id = perThread ? pthread_self() : 0x0;
    return countMap_[label][id];
}

/**.......................................................................
 * Start a named counter
 */
unsigned Profiler::start(std::string& label, bool perThread)
{
    unsigned count = 0;

    mutex_.Lock();

    Counter& counter = getCounter(label, perThread);
    count = ++counter_;
    counter.start(getCurrentMicroSeconds(), count);

    mutex_.Unlock();
    
    return count;
}

/**.......................................................................
 * Stop a named counter
 */
void Profiler::stop(std::string& label, bool perThread)
{
    mutex_.Lock();

    Counter& counter = getCounter(label, perThread);
    counter.stop(getCurrentMicroSeconds(), counter_);

    // counter_ now serves as both a unique incrementing counter,
    // and a count of the number of times the Profiler registers
    // have been accessed (which is why we increment it here)
    
    ++counter_;
    
    mutex_.Unlock();
}

std::vector<pthread_t> Profiler::getThreadIds()
{
    std::map<pthread_t, pthread_t> tempMap;
    std::vector<pthread_t> threadVec;

    // Iterate over all labeled counters and threads to construct a
    // unique list of threads
    
    for(std::map<std::string, std::map<pthread_t, Profiler::Counter> >::iterator iter = countMap_.begin();
        iter != countMap_.end(); iter++) {
        std::map<pthread_t, Profiler::Counter>& threadMap = iter->second;
        for(std::map<pthread_t, Profiler::Counter>::iterator iter2 = threadMap.begin(); iter2 != threadMap.end(); iter2++) {
            tempMap[iter2->first] = iter2->first;
        }
    }

    // Now iterate over the map to construct the return vector
    
    for(std::map<pthread_t, pthread_t>::iterator iter = tempMap.begin(); iter != tempMap.end(); iter++)
        threadVec.push_back(iter->first);

    return threadVec;
}

/**.......................................................................
 * Dump out a text file with profiler stats
 */
void Profiler::dump(std::string fileName)
{
    std::fstream outfile;                                               
    outfile.open(fileName.c_str(), std::fstream::out);
    outfile << formatStats(false);
    outfile.close();
}

std::string Profiler::formatStats(bool term)
{
    std::ostringstream os;
    
    mutex_.Lock();

    //------------------------------------------------------------
    // Write the total count at the top of the file
    //------------------------------------------------------------
    
    OSTERM(os, term, true, false, "totalcount" << " " << counter_ << std::endl);

    //------------------------------------------------------------
    // Write the list of labels
    //------------------------------------------------------------
    
    OSTERM(os, term, true, false, "label" << " ");
    
    for(std::map<std::string, std::map<pthread_t, Profiler::Counter> >::iterator iter = countMap_.begin();
        iter != countMap_.end(); iter++) {
        os << "'" << iter->first << "'" << " ";
    }
    
    OSTERM(os, term, false, true, std::endl);

    //------------------------------------------------------------
    // Now iterate over all known threads, writing the count for each
    // label
    //------------------------------------------------------------

    std::vector<pthread_t> threadVec = getThreadIds();
    
    for(unsigned iThread=0; iThread < threadVec.size(); iThread++) {

        pthread_t id = threadVec[iThread];
        
        OSTERM(os, term, true, false, "count" << " " << "0x" << std::hex << (int64_t)id << std::dec << " ");
        
        for(std::map<std::string, std::map<pthread_t, Profiler::Counter> >::iterator iter = countMap_.begin();
            iter != countMap_.end(); iter++) {
            std::map<pthread_t, Profiler::Counter>& threadCountMap = iter->second;


            if(threadCountMap.find(id) == threadCountMap.end()) {
                os << "0" << " ";
            } else {
                Profiler::Counter& counter = threadCountMap[id];
                os << counter.deltaCounts_ << " ";
            }
        }
        
        OSTERM(os, term, false, true, std::endl);
    }
    
    //------------------------------------------------------------
    // Now iterate over all known threads, writing the elapsed usec
    // for each label
    //------------------------------------------------------------
    
    for(unsigned iThread=0; iThread < threadVec.size(); iThread++) {

        pthread_t id = threadVec[iThread];
        
        OSTERM(os, term, true, false, "usec" << " " << "0x" << std::hex << (int64_t)id << std::dec << " ");
        
        for(std::map<std::string, std::map<pthread_t, Profiler::Counter> >::iterator iter = countMap_.begin();
            iter != countMap_.end(); iter++) {
            std::map<pthread_t, Profiler::Counter>& threadCountMap = iter->second;

            if(threadCountMap.find(id) == threadCountMap.end()) {
                os << "0" << " ";
            } else {
                Profiler::Counter& counter = threadCountMap[id];
                os << counter.deltaUsec_ << " ";
            }
        }
        
        OSTERM(os, term, false, true, std::endl);
    }
    
    //------------------------------------------------------------
    // Finally, write out any errors
    //------------------------------------------------------------

    for(unsigned iThread=0; iThread < threadVec.size(); iThread++) {

        pthread_t id = threadVec[iThread];
        
        for(std::map<std::string, std::map<pthread_t, Profiler::Counter> >::iterator iter = countMap_.begin();
            iter != countMap_.end(); iter++) {
            std::map<pthread_t, Profiler::Counter>& threadCountMap = iter->second;
            
            if(threadCountMap.find(id) != threadCountMap.end()) {
                Profiler::Counter& counter = threadCountMap[id];

                if(counter.errorCountUninitiated_ > 0) {
                    OSTERM(os, term, false, true, "WARNING:" 
                           << " thread 0x" << std::hex << (int64_t)id
                           << " attempted " << counter.errorCountUninitiated_ << (counter.errorCountUninitiated_ > 1 ? " terminations" : " termination")
                           << " of counter '" << iter->first << "' without initiation" << std::endl);
                }

                if(counter.errorCountUnterminated_ > 0) {
                    OSTERM(os, term, false, true, "WARNING: " 
                           << "thread 0x" << std::hex << (int64_t)id
                           << " attempted " << counter.errorCountUnterminated_ << (counter.errorCountUnterminated_ > 1 ? " initiations" : " initiation")
                           << " of counter '" << iter->first << "' without termination" << std::endl);
                }

                if(counter.state_ != STATE_DONE) {
                    OSTERM(os, term, false, true, "WARNING: " 
                           << "thread 0x" << std::hex << (int64_t)id
                           << " left counter '" << iter->first << "' unterminated" << std::endl);
                }
            }
        }
        
        OSTERM(os, term, false, true, std::endl);
    }

    mutex_.Unlock();

    return os.str();
}

int64_t Profiler::getCurrentMicroSeconds()
{
#if _POSIX_TIMERS >= 200801L

struct timespec ts;

// this is rumored to be faster that gettimeofday(), and sometimes
// shift less ... someday use CLOCK_MONOTONIC_RAW

 clock_gettime(CLOCK_MONOTONIC, &ts);
 return static_cast<uint64_t>(ts.tv_sec) * 1000000 + ts.tv_nsec/1000;

#else

struct timeval tv;
gettimeofday(&tv, NULL);
return static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;

#endif
}

/**.......................................................................
 * Return the static instance of this class
 */
Profiler* Profiler::get()
{
    return &instance_;
}

/**.......................................................................
 * Set the prefix path for storing profiler output
 */
void Profiler::setPrefix(std::string prefix)
{
    prefix_ = prefix;
}

/**.......................................................................
 * Setting noop to true makes the various Profiler::profile()
 * interfaces no-ops by default.  Note that this behaviur can be
 * overridden by calling those interfaces with always=true
 *
 * Note also that although this is a static variable whose state can
 * change, it is not mutex protected.  The expectation is that this is
 * something that will be toggled once on a per-process basis, either
 * on module initialization, to disable profiling based on the state
 * of an erlang -define(), or manually, for debugging purposes.  In
 * normal operation, we do not want to add additional mutex
 * locking/unlocking on top of the mutex controlled access to the
 * counters that we already have
 */
void Profiler::noop(bool makeNoop)
{
    noop_ = makeNoop;
}

/**.......................................................................
 * Print debug information
 */
void Profiler::debug()
{
    COUT("Prefix is: "  << GREEN << "'" << instance_.prefix_ << "'" << std::endl << NORM);
    COUT("Noop is:   "  << GREEN << noop_ << std::endl << NORM);

    COUT("Stats: " << GREEN << std::endl << std::endl << formatStats(true) << NORM);
}

unsigned Profiler::profile(std::string command, std::string value, bool always)
{
    unsigned retval=0;

    if(noop_ && !always)
        return retval;

    if(command == "prefix")
        instance_.setPrefix(value);
    else if(command == "dump")
        instance_.dump(value);
    else if(command == "start")
        retval = instance_.start(value, true);
    else if(command == "stop")
        instance_.stop(value, true);

    return retval;
}

unsigned Profiler::profile(std::string command, std::string value, bool perThread, bool always)
{
    unsigned retval=0;
    
    if(noop_ && !always)
        return retval;

    if(command == "start")
        retval = instance_.start(value, perThread);
    else if(command == "stop")
        instance_.stop(value, perThread);

    return retval;
}

