#pragma once

#include "bacenum.h"
#include "bacdef.h"
#include "BACnetObject.h"

#include <vector>

/* String Lengths - excluding any nul terminator */
#define MAX_DEV_NAME_LEN 32
#define MAX_DEV_LOC_LEN  64
#define MAX_DEV_MOD_LEN  32
#define MAX_DEV_VER_LEN  16
#define MAX_DEV_DESC_LEN 64


/** Structure to define the Object Properties common to all Objects. */
typedef struct commonBacObj_s {

    /** The BACnet type of this object (ie, what class is this object from?).
    * This property, of type BACnetObjectType, indicates membership in a
    * particular object type class. Each inherited class will be of one type.
    */
    BACNET_OBJECT_TYPE mObject_Type;

    /** The instance number for this class instance. */
    uint32_t Object_Instance_Number;

    /** Object Name; must be unique.
    * This property, of type CharacterString, shall represent a name for
    * the object that is unique within the BACnet Device that maintains it.
    */
    char Object_Name[MAX_DEV_NAME_LEN];
    /** Device Description. */
    char Description[MAX_DEV_DESC_LEN];

} COMMON_BAC_OBJECT;


/** Structure to define the Properties of Device Objects which distinguish
*  one instance from another.
*  This structure only defines fields for properties that are unique to
*  a given Device object.  The rest may be fixed in device.c or hard-coded
*  into the read-property encoding.
*  This may be useful for implementations which manage multiple Devices,
*  eg, a Gateway.
*/

typedef enum {
    LSDT_Gateway,
    LSDT_Site,
    LSDT_Group
} LSDT ;

typedef struct devObj_s {
    /** The BACnet Device Address for this device; ->len depends on DLL type. */
    BACNET_PATH bacDevAddr;

    /** Structure for the Object Properties common to all Objects. */
    COMMON_BAC_OBJECT bacObj;   // todo 3 rename 'commonProperties' ?

    /** Device Description. */
// ekh -> moving to object description    char Description[MAX_DEV_DESC_LEN];

    /** The upcounter that shows if the Device ID or object structure has changed. */
    uint32_t Database_Revision;

    // what is this device virtualizing
    LSDT lsDeviceType;
    int id2;

    std::vector<BACnetObject *> analogInputs;
    std::vector<BACnetObject *> analogValues;
    std::vector<BACnetObject *> notificationClasses;
    std::vector<BACnetObject *> alertEnrollments;

    /* note: the disable and time are not expected to survive
    over a power cycle or reinitialization. */
    /* note: time duration is given in Minutes, but in order to be accurate,
    we need to count down in seconds. */
    /* infinite time duration is defined as 0 */
    uint32_t DCC_Time_Duration_Seconds;
    BACNET_COMMUNICATION_ENABLE_DISABLE DCC_Enable_Disable = COMMUNICATION_ENABLE;
    /* password is optionally supported */
} DEVICE_OBJECT_DATA;




