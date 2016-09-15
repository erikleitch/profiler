#include "RingPartition.h"

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

