#include "Profiler.h"
#include "String.h"

#include "exceptionutils.h"

#include <sstream>
#include <fstream>
#include <signal.h>

#include <sys/time.h>
#include <sys/select.h>

using namespace nifutil;
using namespace std;

Profiler Profiler::instance_;
bool     Profiler::noop_ = false;

//=======================================================================
// Profiler::Counter
//=======================================================================

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

//=======================================================================
// Profiler
//=======================================================================

/**.......................................................................
 * Constructor.
 */
Profiler::Profiler() 
{
    nAccessed_            = 0;
    counter_              = 0;
    atomicCounterTimerId_ = 0;
    majorIntervalUs_      = 0;
    firstDump_            = true;
    
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

    if(atomicCounterTimerId_ != 0) {
        pthread_kill(atomicCounterTimerId_, SIGKILL);
    }
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

void Profiler::addRingPartition(uint64_t ptr, std::string leveldbFile)
{
    instance_.mutex_.Lock();
    
    gcp::util::String str(leveldbFile);
    gcp::util::String base = str.findNextInstanceOf("./data/leveldb/", true, " ", false);

    FOUT("About  to add counter with base = " << base);

    if(!base.isEmpty()) {
        instance_.atomicCounterMap_[ptr].leveldbFile_ = base.str();
        FOUT("Added counter with base = " << base << " size = " << instance_.atomicCounterMap_.size() << " instance = " << &instance_);
    }

    instance_.mutex_.Unlock();
}

void Profiler::initializeAtomicCounters(std::map<std::string, std::string>& nameMap,
                                        unsigned int bufferSize, uint64_t intervalUs, std::string fileName)
{
    instance_.mutex_.Lock();

    FOUT("About to initializeAtomicCounter with id = " <<  instance_.atomicCounterTimerId_ << " and filename = " << fileName);
    
    if(instance_.atomicCounterTimerId_== 0) {
        
        for(std::map<uint64_t, RingPartition>::iterator part = instance_.atomicCounterMap_.begin();
            part != instance_.atomicCounterMap_.end(); part++) {
            
            for(std::map<std::string,std::string>::iterator iter = nameMap.begin(); iter != nameMap.end(); iter++) {
                part->second.counterMap_[iter->first].setTo(bufferSize, intervalUs);
            }
        }
        
        instance_.atomicCounterOutput_ = fileName;
        instance_.majorIntervalUs_ = bufferSize * intervalUs;
        
        // And start the timer
        
        instance_.startAtomicCounterTimer();
    }
    
    instance_.mutex_.Unlock();
}

void Profiler::incrementAtomicCounter(uint64_t partPtr, std::string counterName)
{
    if(instance_.atomicCounterMap_.find(partPtr) != instance_.atomicCounterMap_.end())
        instance_.atomicCounterMap_[partPtr].incrementCounter(counterName, getCurrentMicroSeconds());
}

void Profiler::startAtomicCounterTimer()
{
    std::fstream outfile;
    outfile.open("/tmp/profilerMetaData.txt", std::fstream::out|std::fstream::app);
    outfile << pthread_self() << " Initiating timer thread" << std::endl;
    outfile.close();

    FOUT("Creating timer thread with instance = " << &instance_ << " instance size = " << instance_.atomicCounterMap_.size());

    
    if(pthread_create(&atomicCounterTimerId_, NULL, &runAtomicCounterTimer, &instance_) != 0)
        ThrowRuntimeError("Unable to create timer thread");
}

void Profiler::dumpAtomicCounters()
{
    try {

        FOUT("Inside dumpAtomicCounters" << (atomicCounterMap_.begin() == atomicCounterMap_.end()) << " size = " << instance_.atomicCounterMap_.size() << " this = " << this);
        
        if(atomicCounterMap_.begin() != atomicCounterMap_.end()) {
            std::fstream outfile;
            std::ostringstream os;

            FOUT("About to open output file: " << atomicCounterOutput_);
            outfile.open(atomicCounterOutput_.c_str(), std::fstream::out|std::fstream::app);
            FOUT("About to open output file: " << atomicCounterOutput_ << ".. success");
            uint64_t timestamp = (getCurrentMicroSeconds()/majorIntervalUs_ - 1) * majorIntervalUs_;
            
            //------------------------------------------------------------
            // If this is the first time we've written to the output file,
            // generate a header
            //------------------------------------------------------------
            
            if(firstDump_) {
                
                outfile << "partitions: ";
                for(std::map<uint64_t, RingPartition>::iterator iter=atomicCounterMap_.begin();
                    iter != atomicCounterMap_.end(); iter++)
                    outfile << iter->second.leveldbFile_ << " ";
                outfile << std::endl;
                
                outfile << "tags: " << atomicCounterMap_.begin()->second.listTags() << std::endl;
                
                firstDump_ = false;
            }
            
            //------------------------------------------------------------
            // Now dump out all counters for this timestamp
            //------------------------------------------------------------
            
            outfile << timestamp << ": ";
            for(std::map<uint64_t, RingPartition>::iterator iter=atomicCounterMap_.begin();
                iter != atomicCounterMap_.end(); iter++)
                outfile << iter->second.dumpCounters(timestamp);
            outfile << std::endl;
            
            outfile.close();
        }
        
    } catch(...) {
        FOUT("About to open output file: " << atomicCounterOutput_ << ".. error");
    }
}

THREAD_START(Profiler::runAtomicCounterTimer)
{
    Profiler* prof = (Profiler*)arg;
    bool first = true;
    struct timeval timeout;
    
    do {

        // If this is not the first time through the loop, dump out the counters
        
        if(!first)
            prof->dumpAtomicCounters();
    
        uint64_t currentMicroSeconds = prof->getCurrentMicroSeconds();

        // How much time remains in the current major interval?
        
        uint64_t remainUs = prof->majorIntervalUs_ - (currentMicroSeconds % prof->majorIntervalUs_);

        // We will sleep to the end of this major interval, and halfway through the next.
        
        uint64_t sleepUs  = remainUs + prof->majorIntervalUs_/2;
            
        timeout.tv_sec  = sleepUs / 1000000;
        timeout.tv_usec = sleepUs % 1000000;

        first = false;

    } while(select(0, NULL, NULL, NULL, &timeout) == 0);

    return 0;
}
