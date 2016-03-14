#include "Profiler.h"
#include "exceptionutils.h"

#include <sstream>
#include <fstream>
#include <sys/time.h>

using namespace std;

using namespace nifutil;


Profiler Profiler::instance_;

/**.......................................................................
 * Constructor.
 */
Profiler::Profiler() 
{
    resize(100);

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
    append(os.str());

    for(std::map<pthread_t, std::vector<int64_t>* >::iterator iter=currCounts_.begin(); iter != currCounts_.end(); iter++) {
        delete iter->second;
    }

    for(std::map<pthread_t, std::vector<int64_t>* >::iterator iter=deltaCounts_.begin(); iter != deltaCounts_.end(); iter++) {
        delete iter->second;
    }

    for(std::map<pthread_t, std::vector<int64_t>* >::iterator iter=usecCurr_.begin(); iter != usecCurr_.end(); iter++) {
        delete iter->second;
    }

    for(std::map<pthread_t, std::vector<int64_t>* >::iterator iter=deltas_.begin(); iter != deltas_.end(); iter++) {
        delete iter->second;
    }

    for(std::map<pthread_t, std::vector<string>* >::iterator iter=labels_.begin(); iter != labels_.end(); iter++) {
        delete iter->second;
    }
}

void Profiler::resize(unsigned n)
{
    size_ = n;
}

std::vector<int64_t>* Profiler::getCurrCounts()
{
    pthread_t id = pthread_self();
    id = 0x0;
    bool first = (currCounts_.find(id) == currCounts_.end());
    if(first) {
        currCounts_[id] = new std::vector<int64_t>(size_, 0);
    }

    std::vector<int64_t>* v = currCounts_[id];
    return v;
}

std::vector<int64_t>* Profiler::getDeltaCounts()
{
    pthread_t id = pthread_self();
    id = 0x0;
    bool first = (deltaCounts_.find(id) == deltaCounts_.end());
    if(first) {
        deltaCounts_[id] = new std::vector<int64_t>(size_, 0);
    }

    std::vector<int64_t>* v = deltaCounts_[id];
    return v;
}

std::vector<int64_t>* Profiler::getCurr()
{
    pthread_t id = pthread_self();
    id = 0x0;
    bool first = (usecCurr_.find(id) == usecCurr_.end());
    if(first) {
        usecCurr_[id] = new std::vector<int64_t>(size_, 0);
    }

    std::vector<int64_t>* v = usecCurr_[id];
    return v;
}

std::vector<int64_t>* Profiler::getDeltas()
{
    pthread_t id = pthread_self();
    id = 0x0;
    bool first = (deltas_.find(id) == deltas_.end());

    if(first) {
        deltas_[id] = new std::vector<int64_t>(size_, 0);
    }

    std::vector<int64_t>* v = deltas_[id];
    return v;
}

std::vector<string>* Profiler::getLabels()
{
    pthread_t id = pthread_self();
    id = 0x0;
    bool first = (labels_.find(id) == labels_.end());

    if(first) {
        labels_[id] = new std::vector<string>(size_, "");
    }
    
    std::vector<string>* v = labels_[id];
    return v;
}

unsigned Profiler::start(unsigned index)
{
    unsigned count = 0;
    if(index < size_) {
        
        mutex_.Lock();

        std::vector<int64_t>* curr = getCurr();
        std::vector<int64_t>* currCounts = getCurrCounts();
        int64_t usec = getCurrentMicroSeconds();
        count = ++counter_;

        curr->at(index) = usec;
        currCounts->at(index) = count;
        
        mutex_.Unlock();
    }

    return count;
}

unsigned Profiler::start(unsigned index, std::string label)
{
    unsigned count = 0;
    if(index < size_) {
        
        mutex_.Lock();
        std::vector<int64_t>* curr = getCurr();
        std::vector<int64_t>* currCounts = getCurrCounts();
        std::vector<string>* labels = getLabels();

        int64_t usec = getCurrentMicroSeconds();
        count = ++counter_;

        curr->at(index)   = usec;
        currCounts->at(index) = count;
        
        labels->at(index) = label;

        mutex_.Unlock();
    }

    return count;
}

void Profiler::stop(unsigned index, unsigned count)
{
    std::vector<int64_t>* curr   = getCurr();
    std::vector<int64_t>* deltas = getDeltas();

    std::vector<int64_t>* currCounts  = getCurrCounts();
    std::vector<int64_t>* deltaCounts = getDeltaCounts();

    if(index < size_) {
        mutex_.Lock();

        int64_t usec = getCurrentMicroSeconds();

        deltas->at(index) += (usec - curr->at(index));

        // Keep track of the number of times the Profiler registers
        // have been accessed since the current counter was started.
        // We decrement the count by one because we don't want to
        // include the access that started this counter
        
        deltaCounts->at(index) += (counter_ - currCounts->at(index) - 1);

        // Counter now serves as both a unique incrementing counter,
        // and a count of the number of times the Profiler registers
        // have been accessed (which is why we increment it here)
        
        ++counter_;
             
        mutex_.Unlock();
    }
}

void Profiler::append(std::string fileName)
{
    std::fstream outfile;                                               
    outfile.open(fileName.c_str(), std::fstream::out|std::fstream::app);
    
    mutex_.Lock();

    if(labels_.size() > 0) {
        for(std::map<pthread_t, std::vector<string>* >::iterator iter = labels_.begin();
            iter != labels_.end(); iter++) {
            outfile << "label" << " " << iter->first << " ";
            for(unsigned i=0; i < size_; i++) {
                outfile << "'" << iter->second->at(i) << "'";
                if(i < size_-1)
                    outfile << " ";
            }
            outfile << std::endl;
        }
    }
    
    for(std::map<pthread_t, std::vector<int64_t>* >::iterator iter = deltaCounts_.begin();
        iter != deltaCounts_.end(); iter++) {

        outfile << iter->first << " ";
        for(unsigned i=0; i < size_; i++) {
            outfile << iter->second->at(i);
            if(i < size_-1)
                outfile << " ";
        }
        outfile << std::endl;
    }

    for(std::map<pthread_t, std::vector<int64_t>* >::iterator iter = deltas_.begin();
        iter != deltas_.end(); iter++) {

        outfile << iter->first << " ";
        for(unsigned i=0; i < size_; i++) {
            outfile << iter->second->at(i);
            if(i < size_-1)
                outfile << " ";
        }
        outfile << std::endl;
    }
    
    mutex_.Unlock();

    outfile.close();
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

Profiler* Profiler::get()
{
    return &instance_;
}

void Profiler::setPrefix(std::string prefix)
{
    prefix_ = prefix;
}
