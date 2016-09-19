#include "RingPartition.h"

#include <sstream>

using namespace std;

using namespace nifutil;

/**.......................................................................
 * Constructor.
 */
RingPartition::RingPartition() {}

/**.......................................................................
 * Destructor.
 */
RingPartition::~RingPartition() {}

void RingPartition::incrementCounter(std::string name, uint64_t currentUs)
{
    if(counterMap_.find(name) != counterMap_.end())
        counterMap_[name].increment(currentUs);
}

std::string RingPartition::dumpCounters(uint64_t currentUs)
{
    std::ostringstream os;

    for(std::map<std::string, BufferedAtomicCounter>::iterator iter=counterMap_.begin(); iter != counterMap_.end(); iter++)
        os << iter->second.dump(currentUs);

    return os.str();
}

std::string RingPartition::listTags()
{
    std::ostringstream os;
    for(std::map<std::string, BufferedAtomicCounter>::iterator iter=counterMap_.begin(); iter != counterMap_.end(); iter++)
        os << iter->first << " ";
    return os.str();
}

