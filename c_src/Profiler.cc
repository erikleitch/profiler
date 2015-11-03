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
    resize(20);

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

    for(std::map<pthread_t, std::vector<int64_t>* >::iterator iter=usecCurr_.begin(); iter != usecCurr_.end(); iter++) {
        delete iter->second;
    }

    for(std::map<pthread_t, std::vector<int64_t>* >::iterator iter=deltas_.begin(); iter != deltas_.end(); iter++) {
        delete iter->second;
    }
}

void Profiler::resize(unsigned n)
{
    size_ = n;
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

unsigned Profiler::start(unsigned index)
{
    unsigned count = 0;
    if(index < size_) {
        
        mutex_.Lock();

        std::vector<int64_t>* curr = getCurr();
        int64_t usec = getCurrentMicroSeconds();
        count = ++counter_;

//        if(index < 7) {
//            FOUT(pthread_self() << " Accessing curr = " << curr << " with index = " << index << " and usec = " << usec 
//                 << " and curr->at(index) = " << curr->at(index) << " count = " << count);
//        }
        curr->at(index) = usec;
        mutex_.Unlock();
    }

    return count;
}

void Profiler::stop(unsigned index, unsigned count)
{
    std::vector<int64_t>* curr   = getCurr();
    std::vector<int64_t>* deltas = getDeltas();
    if(index < size_) {
        mutex_.Lock();

        int64_t usec = getCurrentMicroSeconds();

//        if(index < 7) {
//            FOUT(pthread_self() << " Accessing deltas = " << deltas << " curr = " << curr << " with index = " << index << " and usec = " << usec << " and curr->at(index) = " << curr->at(index) << " and deltas->at(index) = " << deltas->at(index) << " count = " << count);
//        }

        deltas->at(index) += (usec - curr->at(index));

        mutex_.Unlock();
    }
}

void Profiler::append(std::string fileName)
{
    std::fstream outfile;                                               
    outfile.open(fileName.c_str(), std::fstream::out|std::fstream::app);
    
    mutex_.Lock();

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
