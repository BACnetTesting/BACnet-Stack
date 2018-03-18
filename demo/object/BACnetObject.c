#include "BACnetObject.h"
#include "debug.h"

BACnetObject *Generic_Instance_To_Object(
    std::vector<BACnetObject *> objects,
    uint32_t object_instance)
{
    std::vector<BACnetObject *>::iterator it;
    for (it = objects.begin(); it != objects.end(); ++it) {
        if (object_instance == (*it)->instance) return *it;
    }
    dbTraffic(DB_BTC_ERROR, "Illegal index, %s, %d", __FILE__, __LINE__);
    return NULL;
}
