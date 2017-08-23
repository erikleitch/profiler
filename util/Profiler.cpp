#include "stdafx.h"

#include "Profiler.h"
#include "ProfString.h"

#include "exceptionutils.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <signal.h>

//-----------------------------------------------------------------------
// Platform specific code
//-----------------------------------------------------------------------

#ifdef _WIN32

#include <windows.h>
#include <Winsock2.h>

#define THREAD_START(fn) DWORD (fn)(LPVOID arg)
#define thread_self GetCurrentThreadId
typedef DWORD thread_id;

#else

#include <sys/select.h>
#include <pthread.h>

#define THREAD_START(fn) void* (fn)(void *arg)
#define thread_self pthread_self
typedef pthread_t thread_id;

#endif

//-----------------------------------------------------------------------
// End platform-specific code
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// The implementation of the Profiler class
//-----------------------------------------------------------------------


namespace profiler {
    class ProfilerImpl {

        enum CounterState {
            STATE_TRIGGERED,
            STATE_DONE
        };
        
        struct Counter {
            int64_t currentCounts_;
            int64_t deltaCounts_;

            int64_t currentUsec_;
            int64_t deltaUsec_;

            CounterState state_;

            unsigned errorCountUninitiated_;
            unsigned errorCountUnterminated_;

            void start(int64_t usec, unsigned count);
            void stop(int64_t usec, unsigned count);
            
            Counter();
        };

    public: 
        
        static void noop(bool makeNoop);
        static int64_t getCurrentMicroSeconds();
        static ProfilerImpl* get();

        static unsigned profile(std::string command, bool perThread=false, bool always=false);
        static unsigned profile(std::string command, std::string value, bool perThread=false, bool always=false);

        //------------------------------------------------------------
        // Time-resolved atomic counters
        //------------------------------------------------------------


        static void addRingPartition(uint64_t ptr, std::string leveldbFile);
        
        static void initializeAtomicCounters(std::map<std::string, std::string>& nameMap,
                                             unsigned int bufferSize, uint64_t intervalMs,
                                             std::string fileName);

        static void incrementAtomicCounter(uint64_t partPtr, std::string counterName);

        unsigned start(std::string& label, bool perThread);
        void stop(std::string& label, bool perThread);
        
        std::string formatStats(bool crTerminated);
        void dump(std::string fileName);
        void setPrefix(std::string fileName);
        void debug();
        
        Counter& getCounter(std::string& label, bool perThread);
        
        void startAtomicCounterTimer();
        void dumpAtomicCounters();
        static THREAD_START(runAtomicCounterTimer);
        
        virtual ~ProfilerImpl();

    private:
        
        ProfilerImpl();
        std::vector<thread_id> getThreadIds();
        std::map<std::string, std::map<thread_id, Counter> > countMap_;
        thread_id atomicCounterTimerId_;
        
        //------------------------------------------------------------
        // Members for normal counters
        //------------------------------------------------------------
        
        unsigned nAccessed_;
        
        Mutex mutex_;
        unsigned counter_;
        std::string prefix_;
        
        //------------------------------------------------------------
        // Members for time-resolved atomic counting
        //------------------------------------------------------------
        
        std::map<uint64_t, RingPartition> atomicCounterMap_;
        
        uint64_t majorIntervalUs_;
        std::string atomicCounterOutput_;
        bool firstDump_;
        
        static ProfilerImpl instance_;
        static bool noop_;
        
    };
};

using namespace profiler;
using namespace std;

ProfilerImpl ProfilerImpl::instance_;
bool         ProfilerImpl::noop_ = false;


//=======================================================================
// ProfilerImpl
//=======================================================================

/**.......................................................................
 * Constructor.
 */
ProfilerImpl::ProfilerImpl() 
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
ProfilerImpl::~ProfilerImpl() 
{
#ifndef _WIN32
    std::ostringstream os;
    os << prefix_ << "/" << this << "_profile.txt";
    dump(os.str());

    if(atomicCounterTimerId_ != 0) {
        pthread_kill(atomicCounterTimerId_, SIGKILL);
    }
#else
    std::ostringstream os;
    os << prefix_ << "\\" << this << "_profile.txt";
    dump(os.str());
#endif
}

ProfilerImpl::Counter& ProfilerImpl::getCounter(std::string& label, bool perThread)
{
    thread_id id = perThread ? thread_self() : 0x0;
    return countMap_[label][id];
}

/**.......................................................................
 * Start a named counter
 */
unsigned ProfilerImpl::start(std::string& label, bool perThread)
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
void ProfilerImpl::stop(std::string& label, bool perThread)
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

std::vector<thread_id> ProfilerImpl::getThreadIds()
{
    std::map<thread_id, thread_id> tempMap;
    std::vector<thread_id> threadVec;

    // Iterate over all labeled counters and threads to construct a
    // unique list of threads
    
    for(std::map<std::string, std::map<thread_id, ProfilerImpl::Counter> >::iterator iter = countMap_.begin();
        iter != countMap_.end(); iter++) {
        std::map<thread_id, ProfilerImpl::Counter>& threadMap = iter->second;
        for(std::map<thread_id, ProfilerImpl::Counter>::iterator iter2 = threadMap.begin(); iter2 != threadMap.end(); iter2++) {
            tempMap[iter2->first] = iter2->first;
        }
    }

    // Now iterate over the map to construct the return vector
    
    for(std::map<thread_id, thread_id>::iterator iter = tempMap.begin(); iter != tempMap.end(); iter++)
        threadVec.push_back(iter->first);

    return threadVec;
}

/**.......................................................................
 * Dump out a text file with profiler stats
 */
void ProfilerImpl::dump(std::string fileName)
{
    COUT("Dumping to file: " << fileName);
    std::fstream outfile;                                               
    outfile.open(fileName.c_str(), std::fstream::out);
    outfile << formatStats(false);
    outfile.close();
}

std::string ProfilerImpl::formatStats(bool term)
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
    
    for(std::map<std::string, std::map<thread_id, ProfilerImpl::Counter> >::iterator iter = countMap_.begin();
        iter != countMap_.end(); iter++) {
        os << "'" << iter->first << "'" << " ";
    }
    
    OSTERM(os, term, false, true, std::endl);

    //------------------------------------------------------------
    // Now iterate over all known threads, writing the count for each
    // label
    //------------------------------------------------------------

    std::vector<thread_id> threadVec = getThreadIds();
    
    for(unsigned iThread=0; iThread < threadVec.size(); iThread++) {

        thread_id id = threadVec[iThread];
        
        OSTERM(os, term, true, false, "count" << " " << "0x" << std::hex << (int64_t)id << std::dec << " ");
        
        for(std::map<std::string, std::map<thread_id, ProfilerImpl::Counter> >::iterator iter = countMap_.begin();
            iter != countMap_.end(); iter++) {
            std::map<thread_id, ProfilerImpl::Counter>& threadCountMap = iter->second;


            if(threadCountMap.find(id) == threadCountMap.end()) {
                os << "0" << " ";
            } else {
                ProfilerImpl::Counter& counter = threadCountMap[id];
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

        thread_id id = threadVec[iThread];
        
        OSTERM(os, term, true, false, "usec" << " " << "0x" << std::hex << (int64_t)id << std::dec << " ");
        
        for(std::map<std::string, std::map<thread_id, ProfilerImpl::Counter> >::iterator iter = countMap_.begin();
            iter != countMap_.end(); iter++) {
            std::map<thread_id, ProfilerImpl::Counter>& threadCountMap = iter->second;

            if(threadCountMap.find(id) == threadCountMap.end()) {
                os << "0" << " ";
            } else {
                ProfilerImpl::Counter& counter = threadCountMap[id];
                os << counter.deltaUsec_ << " ";
            }
        }
        
        OSTERM(os, term, false, true, std::endl);
    }
    
    //------------------------------------------------------------
    // Finally, write out any errors
    //------------------------------------------------------------

    for(unsigned iThread=0; iThread < threadVec.size(); iThread++) {

        thread_id id = threadVec[iThread];
        
        for(std::map<std::string, std::map<thread_id, ProfilerImpl::Counter> >::iterator iter = countMap_.begin();
            iter != countMap_.end(); iter++) {
            std::map<thread_id, ProfilerImpl::Counter>& threadCountMap = iter->second;
            
            if(threadCountMap.find(id) != threadCountMap.end()) {
                ProfilerImpl::Counter& counter = threadCountMap[id];

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

int64_t ProfilerImpl::getCurrentMicroSeconds()
{
#ifdef _WIN32
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970 

    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    long sec  = (long) ((time - EPOCH) / 10000000L);
    long usec = (long) (system_time.wMilliseconds * 1000);
    return static_cast<uint64_t>(sec) * 1000000 + usec;
    
#elif _POSIX_TIMERS >= 200801L

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
ProfilerImpl* ProfilerImpl::get()
{
    return &instance_;
}

/**.......................................................................
 * Set the prefix path for storing profiler output
 */
void ProfilerImpl::setPrefix(std::string prefix)
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
void ProfilerImpl::noop(bool makeNoop)
{
    noop_ = makeNoop;
}

/**.......................................................................
 * Print debug information
 */
void ProfilerImpl::debug()
{
    COUT("Prefix is: "  << GREEN << "'" << instance_.prefix_ << "'" << std::endl << NORM);
    COUT("Noop is:   "  << GREEN << noop_ << std::endl << NORM);

    COUT("Stats: " << GREEN << std::endl << std::endl << formatStats(true) << NORM);
}

unsigned ProfilerImpl::profile(std::string command, bool perThread, bool always)
{
    return profile(command, "", perThread, always);
}

unsigned ProfilerImpl::profile(std::string command, std::string value, bool perThread, bool always)
{
    unsigned retval=0;

    if(noop_ && !always)
        return retval;

    if(command == "prefix")
        instance_.setPrefix(value);
    else if(command == "dump")
        instance_.dump(value);
    else if(command == "debug")
        instance_.debug();
    else if(command == "start") {
        retval = instance_.start(value, perThread);
    } else if(command == "stop") {
        instance_.stop(value, perThread);
    }

    return retval;
}

void ProfilerImpl::addRingPartition(uint64_t ptr, std::string leveldbFile)
{
    instance_.mutex_.Lock();
    
    String str(leveldbFile);
    String base = str.findNextInstanceOf("./data/leveldb/", true, " ", false);

    FOUT("About  to add counter with base = " << base);

    if(!base.isEmpty()) {
        instance_.atomicCounterMap_[ptr].leveldbFile_ = base.str();
        FOUT("Added counter with base = " << base << " size = " << instance_.atomicCounterMap_.size() << " instance = " << &instance_);
    }

    instance_.mutex_.Unlock();
}

void ProfilerImpl::initializeAtomicCounters(std::map<std::string, std::string>& nameMap,
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

void ProfilerImpl::incrementAtomicCounter(uint64_t partPtr, std::string counterName)
{
    if(instance_.atomicCounterMap_.find(partPtr) != instance_.atomicCounterMap_.end())
        instance_.atomicCounterMap_[partPtr].incrementCounter(counterName, getCurrentMicroSeconds());
}

void ProfilerImpl::startAtomicCounterTimer()
{
    std::fstream outfile;
    outfile.open("/tmp/profilerMetaData.txt", std::fstream::out|std::fstream::app);
    outfile << thread_self() << " Initiating timer thread" << std::endl;
    outfile.close();

    FOUT("Creating timer thread with instance = " << &instance_ << " instance size = " << instance_.atomicCounterMap_.size());

#ifndef _WIN32
    if(pthread_create(&atomicCounterTimerId_, NULL, &runAtomicCounterTimer, &instance_) != 0)
        ThrowRuntimeError("Unable to create timer thread");
#else
    if(CreateThread(NULL, 0, runAtomicCounterTimer, &instance_, 0, &atomicCounterTimerId_) == 0)
        ThrowRuntimeError("Unable to create timer thread");
#endif

}

void ProfilerImpl::dumpAtomicCounters()
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

THREAD_START(ProfilerImpl::runAtomicCounterTimer)
{
    ProfilerImpl* prof = (ProfilerImpl*)arg;
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

//=======================================================================
// ProfilerImpl::Counter
//=======================================================================

ProfilerImpl::Counter::Counter()
{
    currentCounts_ = 0;
    deltaCounts_   = 0;

    currentUsec_   = 0;
    deltaUsec_     = 0;

    state_ = STATE_DONE;
    errorCountUninitiated_  = 0;
    errorCountUnterminated_ = 0;
}

void ProfilerImpl::Counter::start(int64_t usec, unsigned count)
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

void ProfilerImpl::Counter::stop(int64_t usec, unsigned count)
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
// Profiler class definition
//=======================================================================

unsigned Profiler::profile(std::string command, bool perThread, bool always)
{
    return ProfilerImpl::profile(command, perThread, always);
}

unsigned Profiler::profile(std::string command, std::string value, bool perThread, bool always)
{
    return ProfilerImpl::profile(command, value, perThread, always);
}

void Profiler::noop(bool makeNoop)
{
    return ProfilerImpl::noop(makeNoop);
}

int64_t Profiler::getCurrentMicroSeconds()
{
    return ProfilerImpl::getCurrentMicroSeconds();
}


void Profiler::addRingPartition(uint64_t ptr, std::string leveldbFile)
{
    return ProfilerImpl::addRingPartition(ptr, leveldbFile);
}

void Profiler::initializeAtomicCounters(std::map<std::string, std::string>& nameMap,
                 unsigned int bufferSize, uint64_t intervalMs,
                 std::string fileName)
{
    return ProfilerImpl::initializeAtomicCounters(nameMap, bufferSize, intervalMs, fileName);
}

void Profiler::incrementAtomicCounter(uint64_t partPtr, std::string counterName)
{
    return ProfilerImpl::incrementAtomicCounter(partPtr, counterName);
}
