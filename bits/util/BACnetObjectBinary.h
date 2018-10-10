/****************************************************************************************
*
*   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.

*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.If not, see <http://www.gnu.org/licenses/>.
*
*   For more information : info@bac-test.com
*
*   For access to source code :
*
*       info@bac-test.com
*           or
*       www.github.com/bacnettesting/bacnet-stack
*
****************************************************************************************/

#pragma once

#include "bacenum.h"
#include <stdint.h>
// #include <string>

#if (INTRINSIC_REPORTING == 1)
#include "nc.h"
#endif

#include "BACnetObject.h"

#define RELINQUISH_DEFAULT_BINARY BINARY_INACTIVE

#if 0
class BACnetBinaryObject : public BACnetObject
{
public:
    BACnetBinaryObject(uint32_t nInstance, std::string &name, std::string &description) :
        BACnetObject(nInstance, name, description),
        Present_Value (BINARY_INACTIVE)
    {}

    BACNET_BINARY_PV    Present_Value ;
};


class AbstractBinary : public BACnetObjectWithPV
{
public:

    AbstractBinary(uint32_t nInstance, std::string &name, std::string &description) :
        BACnetObjectWithPV(nInstance, name, description),
        Present_Value(BINARY_INACTIVE),
        Polarity(POLARITY_NORMAL)
    {}

    BACNET_BINARY_PV Present_Value;
    BACNET_POLARITY Polarity;

    std::string description;

#if (INTRINSIC_REPORTING == 1)
    BACNET_EVENT_STATE Event_State;
    uint32_t Time_Delay;
    uint32_t Notification_Class;
    float High_Limit;
    float Low_Limit;
    float Deadband;
    unsigned Limit_Enable:2;
    unsigned Event_Enable:3;
    BACNET_NOTIFY_TYPE Notify_Type; // unsigned Notify_Type:1;
    ACKED_INFO Acked_Transitions[MAX_BACNET_EVENT_TRANSITION];
    BACNET_DATE_TIME Event_Time_Stamps[MAX_BACNET_EVENT_TRANSITION];
    /* time to generate event notification */
    uint32_t Remaining_Time_Delay;
    /* AckNotification informations */
    ACK_NOTIFICATION Ack_notify_data;
#endif

#if 0
    BACNET_BINARY_PV localValue;
    BACNET_BINARY_PV localCOVvalue;
    BACNET_POLARITY Polarity;

    AbstractBinary()
    {
    }

    BACNET_BINARY_PV Present_Value_Get_Binary()
    {
        //if (lonInitOK) {
        //    return Read_From_Lon();
        //}
        return localValue;
    }

    // override
    bool Present_Value_Set(BACNET_BINARY_PV value)
    {
        if (localValue != value) Change_Of_Value = true;
        localValue = value;
        //if (lonInitOK) {
        //    // if we get here, the Lon side value has not been changed by someone else, so we can overwrite happily,
        //    return Write_To_Lon(localValue);
        //}
        return false;
    }

    bool Change_Of_Value_Detect()
    {
        if (Change_Of_Value) return true;
        BACNET_BINARY_PV newval = Present_Value_Get_Binary();
        if (localCOVvalue != newval) {
            localCOVvalue = newval;
            return true;
        }
        return false;
    }

    void SetOutOfService(bool svc)
    {
        if (Out_Of_Service != svc) {
            Change_Of_Value = true;
        }
        Out_Of_Service = svc;
    }

    //bool Write_To_Lon(BACNET_BINARY_PV value)
    //{
    //    unsigned int val;
    //    int state;

    //    DataArea dataArea;


    //    if (lonInitOK) {
    //        if (scaling->_func == oSNVT_switch && scaling->lengthInBits == 0 ) {
    //            // an exception (Snvt Switch)

    //            if (value == BINARY_ACTIVE) {
    //                val = 200;
    //                state = 1;
    //            } else if (value == BINARY_INACTIVE) {
    //                val = 0;
    //                state = 0;
    //            } else {
    //                tldReport(TLDP_User_Error, "m0018 - Can only write ACTIVE/INACTIVE to Binary Object");
    //                return false;
    //            }

    //            dataArea.bytes[0] = (uint8_t)val;
    //            dataArea.bytes[1] = (uint8_t)state;
    //        } else {
    //            switch (scaling->scaleDataType) {
    //            case smInteger2:
    //                // 2 byte network variables, with scaling
    //                dataArea.int16v = (short int)(( (float) value - scaling->C) / pow(10.f, scaling->B) / scaling->A);
    //                break;

    //            case smUshort:
    //                dataArea.byte = (unsigned char)(((float) value - scaling->C) / pow(10.f, scaling->B) / scaling->A);
    //                break;

    //            case smSshort:
    //                dataArea.int8v = (char)(((float) value - scaling->C) / pow(10.f, scaling->B) / scaling->A);
    //                break;

    //            case smSlong:
    //                dataArea.int16v = (short int)(((float) value - scaling->C) / pow(10.f, scaling->B) / scaling->A);
    //                break;

    //            case smUlong:
    //                dataArea.uint16v = (unsigned short int)(((float) value - scaling->C) / pow(10.f, scaling->B) / scaling->A);
    //                break;

    //            // 4 byte network variables, with scaling
    //            // case smInteger4:
    //            case smS_32:
    //                dataArea.int32v = (int)(((float) value - scaling->C) / pow(10.f, scaling->B) / scaling->A);
    //                break;

    //            case smUquad:
    //                dataArea.uint32v = (unsigned int)(((float) value - scaling->C) / pow(10.f, scaling->B) / scaling->A);
    //                break;

    //            // 2 byte network variables, Raw
    //            //                    dataArea.int16v = (short int) value;
    //            //                    break;

    //            // 4 byte network variables, Float
    //            case smFloat4:
    //                dataArea.floatv = (float) value;
    //                break;

    //            // 1 byte network variables (mostly enums)
    //            case smEnum:
    //            case smRaw1:
    //            case smRaw2:
    //            case smRaw8:
    //                // printf("writing = %d\n\r", (unsigned char) value ) ;
    //                dataArea.byte = (unsigned char) value;
    //                break;

    //            default:
    //                ReportUnimplementedScaling("m0273");
    //                //                        tldReport(TLDP_Inform_HQ, "m0003 - Unimplemented Binary Scaling for write [%s] for [%s]", scaling->typeName.c_str(), objectName.c_str());
    //                return false;
    //            }
    //        }

    //        if (scaling->lengthInBits == 0) {
    //            // no shifting required
    //            if (ProtectedWrite(iLonAccess, dataArea.bytes, scaling->objectByteSize) != OK) {
    //                tldReport(TLDP_Inform_HQ, "m0331 - Write failed");
    //                return false;
    //            }
    //        } else {
    //            // todonext - is there a way we can lock the iLon variable to prevent race condition?
    //            // read the whole structured snvt, patch in the new value, and write it all back again
    //            DataArea shadowData;
    //            if (!ProtectedRead(iLonAccess, shadowData.bytes, scaling->objectByteSize, true, 60000)) {
    //                // now we have to shift in our new data
    //                ShiftData(shadowData.bytes, scaling->offsetInBits, dataArea.bytes, 0, scaling->lengthInBits);

    //                // and write it back
    //                if (ProtectedWrite(iLonAccess, shadowData.bytes, scaling->objectByteSize) != OK) {
    //                    tldReport(TLDP_Inform_HQ, "m0332 - Write failed");
    //                    return false;
    //                }
    //            } else {
    //                tldReport(TLDP_Inform_HQ, "m0333 - Read failed");
    //                return false;
    //            }
    //        }

    //    } else {
    //        // myExceptReturnBool();
    //    }

    //    return true;
    //}

    //BACNET_BINARY_PV Read_From_Lon()
    //{
    //    unsigned int value;
    //    int state;

    //    DataArea dataArea;
    //    DataArea shiftedArea =
    //    {   0};

    //    if (lonInitOK) {
    //        // use status OK to avoid ambiguity here.. todonext
    //        if (!ProtectedRead(iLonAccess, dataArea.bytes, scaling->objectByteSize, true, 60000)) { // We don't care if the data is up to 1 minute old.. todonext - make this a configuration choice
    //            FPM::Dp::PointStatus ps = iLonAccess.GetDpPropertyAsPointStatus(FPM::Dp::dataUCPTstatus);

    //            ResolveReliability(ps);

    //            if (scaling->_func == oSNVT_switch && scaling->lengthInBits == 0 ) {
    //                // an exception
    //                // from the docs, the first byte is the unsigned value, and the second is the signed state. Do you know which doc? "SNVT.pdf" in dropbox.
    //                value = dataArea.bytes[0];
    //                state = dataArea.bytes[1];

    //                // for our simple, default case
    //                if (value > 0 && state == 1) {
    //                    // I know this breaks the iLon rules ( the || above, should be && ), but let us play it safe for testing - ekhtodo
    //                    return BINARY_ACTIVE;
    //                }
    //            } else {

    //                if (scaling->lengthInBits == 0) // todo, need to implement this in reading, writing and binaries.

    //                {
    //                    // no shifting
    //                    memcpy(shiftedArea.bytes, dataArea.bytes, sizeof(DataArea));
    //                } else {
    //                    // structured variable, we have to shift the data before scaling
    //                    ShiftData(shiftedArea.bytes, 0, dataArea.bytes, scaling->offsetInBits, scaling->lengthInBits);
    //                }

    //                switch (scaling->scaleDataType) {
    //                case smInteger2:
    //                    // this is a BINARY mapping attached to an analog iLon variable. Do our best!

    //                {
    //                    // tldReport(TLDP_Configuration_Error, "Analog Scaling [%s] not recommended for Binary BACnet variable [%s]", scaling->name.c_str(), bacnetDescription.c_str());
    //                    float rval2 = (float)shiftedArea.int16v * scaling->A * pow(10.f, scaling->B) + scaling->C;
    //                    if (fabs(rval2) > 0.001) return BINARY_ACTIVE;
    //                }
    //                return BINARY_INACTIVE;

    //                case smRaw2:
    //                case smRaw8:
    //                case smEnum:
    //                case smUshort:
    //                case smSshort:
    //                    if (shiftedArea.bytes[0] != 0) return BINARY_ACTIVE;
    //                    break;

    //                default:
    //                    ReportUnimplementedScaling("m0285");
    //                    //                            tldReport(TLDP_Inform_HQ, "m0021 - Unimplemented Binary Scaling [%s] for [%s]", scaling->typeName.c_str(), objectName.c_str());
    //                    return BINARY_INACTIVE;
    //                }
    //            }
    //        } else {
    //            tldReport(TLDP_Configuration_Error, "Read Failed for [%s]", objectName.c_str());
    //            return BINARY_INACTIVE;
    //        }
    //    } else {
    //        tldReport(TLDP_User_Message, "m0005 - binary point offline [%s]", objectName.c_str());
    //        // todo, add a flag here indicating 'offline' or something...
    //        // printf("Binary Object iLon initialization was not OK\n");
    //        // myExceptMsg("Binary Object initialization not OK");
    //    }

    //    return BINARY_INACTIVE;
    //}

    //bool ReadForClientSide(BACNET_APPLICATION_DATA_VALUE *value)
    //{
    //    if (lonInitOK) {
    //        BACNET_BINARY_PV rVal = Read_From_Lon();

    //        // KC - refer to "bacapp_parse_application_data" in "bacapp.cpp" for filling the data structure
    //        value->tag = BACNET_APPLICATION_TAG_ENUMERATED;
    //        value->type.Enumerated = rVal;
    //        value->next = NULL;
    //        value->context_specific = false;

    //        return true;
    //    } else {
    //        return false;
    //    }
    //}

    //  // phew, all this rigmarole to create an abstract base class...
    //  // todo - make creative macros
    //  virtual void makeAbstract( void ) = 0 ;
    virtual ~AbstractBinary()
    {}
#endif
};


class AbstractBinaryCommandable : public AbstractBinary
{
#if 0
public:
    BACNET_BINARY_PV values[BACNET_MAX_PRIORITY];
    bool relinquished[BACNET_MAX_PRIORITY];
    BACNET_BINARY_PV relinquishDefault;
    BACNET_BINARY_PV clientShadowCopy;

    // note that c++ does NOT initialize variables, doing so here two ways.

    AbstractBinaryCommandable() : changeFlagInitialized(false), markedForWrite(false)
    {
        for (int i = 0; i < BACNET_MAX_PRIORITY; i++) {
            values[i] = BINARY_INACTIVE;
            relinquished[i] = true;
        }
        relinquishDefault = BINARY_INACTIVE;
        clientShadowCopy = BINARY_INACTIVE;
    }

    bool WriteToPriorityArray(BACNET_BINARY_PV value, unsigned priority)
    {
        bool status = false;
        // Priority should be between 1 and 16, but not 6
        if (priority && (priority <= BACNET_MAX_PRIORITY) &&
            (priority != 6 /* reserved */)) {
            // set the value and relinquish flag
            values[priority - 1] = value;
            relinquished[priority - 1] = false;
            status = true;
            // PrintPriorityArray();
        }
        return status;
    }

    bool RelinquishPriority(unsigned priority)
    {
        bool status = false;
        if (priority && (priority <= BACNET_MAX_PRIORITY) &&
            (priority != 6 /* reserved */)) {
            relinquished[priority - 1] = true;
            /* Note: you could set the physical output here to the next
             highest priority, or to the relinquish default if no
             priorities are set.
             However, if Out of Service is TRUE, then don't set the
             physical output.  This comment may apply to the
             main loop (i.e. check out of service before changing output) */

            localValue = GetResolvedBinaryValueFromPriorityArrayOrRelinquishDefault();
            // tldReport(TLDP_Inform_HQ, "Relinquishing to ** todo ** for type %s", typeid(*this).name());
            //if (lonInitOK) {
            //    Write_To_Lon(localValue);
            //}

            status = true;
        }
        return status;
    }

    // Diagnostic/Debugging Function
    void PrintPriorityArray()
    {
        int k = 0;

        printf("Priority Array Breakdown:\n");

        for (k = 0; k < BACNET_MAX_PRIORITY; k++) {
            printf("\tPriority %d, ", k + 1);

            //              printf("Value: %f, ", values[k]);

            if (relinquished[k]) {
                printf("Relinquished");
            } else {
                printf("Active");
            }

            printf("\n");
        }
        //          printf("\tRelinquish Default: %f\n", relinquishDefault);
    }

    BACNET_BINARY_PV Level_Get(unsigned int priority)
    {
        if (priority <= BACNET_MAX_PRIORITY) {
            return values[priority - 1];
        } else {
            return values[BACNET_MAX_PRIORITY - 1];
        }
    }

    bool Level_Relinquished(unsigned int priority)
    {
        if (priority <= BACNET_MAX_PRIORITY) {
            return relinquished[priority - 1];
        } else {
            return relinquished[BACNET_MAX_PRIORITY - 1];
        }
    }

    BACNET_BINARY_PV GetResolvedBinaryValueFromPriorityArrayOrRelinquishDefault()
    {
        int i = 0;
        for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
            if (!relinquished[i]) {
                return values[i];
            }
        }
        return relinquishDefault;
    }

    unsigned Present_Value_Priority()
    {
        int i;
        for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
            if (!relinquished[i]) {
                return i + 1;
            }
        }
        return 0;
    }

    bool Relinquish_Default_Set(BACNET_BINARY_PV value)
    {
        relinquishDefault = value;
        // Update the LON side with the present value
        localValue = GetResolvedBinaryValueFromPriorityArrayOrRelinquishDefault();
        //if (lonInitOK) {
        //    return Write_To_Lon(localValue);
        //}
        return true;
    }

    bool changeFlagInitialized;
    bool markedForWrite;

    // this method is used by Outputs and Values. (Commandable objects)

    //bool StoreFromClientSideBinary(BACNET_BINARY_PV value)
    //{

    //    if (variableType == CLIENT_WRITE) {
    //        // we may choose not to overwrite this value if it has been changed / marked for write..
    //        if (markedForWrite) {
    //            //                printf("Marked for write, ignoring\n");
    //            return false;
    //        }
    //    }

    //    relinquishDefault = value;
    //    localValue = GetResolvedBinaryValueFromPriorityArrayOrRelinquishDefault();

    //    if (lonInitOK) {
    //        // _Just_ before writing our new value to the iLon side, read it back to detect any possible changes, and if changed, ignore our write
    //        // since someone else has obviously updated it since our last go around...

    //        if (variableType == CLIENT_WRITE) {
    //            BACNET_BINARY_PV ourVal = Read_From_Lon();
    //            if (changeFlagInitialized) {
    //                if (ourVal != clientShadowCopy) {
    //                    // printf("Store client side float %f %f  %f\n", value, ourVal, clientShadowCopy );
    //                    // printf("Ignoring change\n");
    //                    // Lon-side change detected, do not overwrite
    //                    clientShadowCopy = ourVal;
    //                    // save some time for the (expensive) write detection logic later
    //                    markedForWrite = true;
    //                    return false;
    //                }
    //            } else {
    //                clientShadowCopy = ourVal;
    //                changeFlagInitialized = true;
    //                // and we have no option but to overwrite.. so proceed.
    //            }

    //            // _This_ is a bit subtle: We don't want to trigger an immediate write because we have just done a readback... (to this object)
    //            // So we update the shadow copy to avoid that.
    //            // clientShadowCopy = localValue;
    //        }

    //        // printf ("Storing iLon value\n");

    //        return Write_To_Lon(localValue);
    //    }
    //    return true;
    //}

    //bool StoreFromClientSideUnsigned(uint16_t value)
    //{
    //    return StoreFromClientSideBinary((value) ? BINARY_ACTIVE : BINARY_INACTIVE);
    //}

    //bool StoreFromClientSideFloat(float value)
    //{
    //    return StoreFromClientSideBinary((fabs(value) < 0.001) ? BINARY_ACTIVE : BINARY_INACTIVE);
    //}

    //bool ClientNeedsUpdate()
    //{
    //    // either this is a virtual variable, or change has been detected elsewhere, either way, a shortcut...
    //    if (markedForWrite) {
    //        tldReport(TLDP_Trace_1, "Marked for write detected in Binary");
    //        clientShadowCopy = Read_From_Lon();
    //        markedForWrite = false;
    //        return true;
    //    }

    //    if (lonInitOK) {
    //        BACNET_BINARY_PV freshValue = Read_From_Lon();

    //        if (freshValue != clientShadowCopy) {
    //            // printf("change detected %f %f\n", freshValue, clientShadowCopy);
    //            clientShadowCopy = freshValue;
    //            markedForWrite = false;
    //            return true;
    //        } else {
    //            return false;
    //        }
    //    } else {
    //        // quite possible that lon variable not used (esp. in testing)
    //        // myExceptReturnBool(); // default to false if iLon not initialized
    //        return false;
    //    }
    //}

    bool Present_Value_Set(BACNET_BINARY_PV value, unsigned priority)
    {
        if (localValue != value) Change_Of_Value = true;

        bool status = WriteToPriorityArray(value, priority);
        if (!status) return false;

        markedForWrite = true;

        // Update the LON side with the present value
        localValue = GetResolvedBinaryValueFromPriorityArrayOrRelinquishDefault();
        //if (lonInitOK) {

        //    // if we get here, the Lon side value has not been changed by someone else, so we can overwrite happily,
        //    return Write_To_Lon(localValue);
        //}
        return true;
    }

    bool Present_Value_Relinquish(unsigned priority)
    {
        if (!RelinquishPriority(priority)) return false;
        // Update the LON side with the present value

        markedForWrite = true;

        localValue = GetResolvedBinaryValueFromPriorityArrayOrRelinquishDefault();
        //if (lonInitOK) {
        //    return Write_To_Lon(localValue);
        //}
        return true;
    }
#endif
};


class BACnetCommandableBinaryObject : public BACnetBinaryObject
{
public:
    BACnetCommandableBinaryObject(uint32_t nInstance, std::string &name, std::string &description) :
        BACnetBinaryObject(nInstance, name, description ),
        prioritySet ( ),                                 
        priorityValues(),
        // relinquish_default3(NAN)
        relinquish_default(RELINQUISH_DEFAULT_BINARY)
    {}

    bool prioritySet[BACNET_MAX_PRIORITY];
    
    BACNET_BINARY_PV priorityValues[BACNET_MAX_PRIORITY];

    BACNET_BINARY_PV relinquish_default;

    void SweepToPresentValue(void);
};


#if 0
class BO_BinaryValue : public AbstractBinaryCommandable
{

public:

    BO_BinaryValue()
    {
        bacnetType = OBJECT_BINARY_VALUE;
        localValue = BINARY_INACTIVE;
    }
};

class BO_BinaryInput : public AbstractBinary
{
public:
    BO_BinaryInput()
    {
        bacnetType = OBJECT_BINARY_INPUT;
    };

    ~BO_BinaryInput()
    {};

    bool StoreFromClientSideUnsigned(uint16_t value)
    {
        // return StoreFromClientSideBinary((value) ? BINARY_ACTIVE : BINARY_INACTIVE);
    }

    //bool StoreFromClientSideBinary(BACNET_BINARY_PV value)
    //{
    //    localValue = value;
    //    if (lonInitOK) {
    //        Write_To_Lon(localValue);
    //    } else {
    //        //                // this is a client side operation, we are reading a remote point, but don't have a local place to store it
    //        //                // there is not a use case for this. If someone wants that AI, they can read it directly
    //        //                tldReport(TLDP_Configuration_Error, "No mapped iLon point for [%s]", bacnetDescription.c_str() );
    //        //                return false;
    //        // bah - leave the local copy, less technical support...

    //    }
    //    return true;
    //}

    //bool StoreFromClientSideFloat(float value)
    //{
    //    return StoreFromClientSideBinary((fabs(value) < 0.001) ? BINARY_ACTIVE : BINARY_INACTIVE);
    //}
};

class BO_BinaryOutput : public AbstractBinaryCommandable
{
public:
    BO_BinaryOutput()
    {
        bacnetType = OBJECT_BINARY_OUTPUT;
    };
};

#endif

#if 0
class BACnetBinaryObject : public BACnetObject
{
public:

    BACnetBinaryObject(uint32_t nInstance, std::string &name, std::string &description) :
        BACnetObject(nInstance, name, description),
        Present_Value(BINARY_INACTIVE),
        Polarity(POLARITY_NORMAL)
    {}

    BACNET_BINARY_PV Present_Value;
    BACNET_POLARITY Polarity;

    std::string description;

#if (INTRINSIC_REPORTING == 1)
    bool Change_Of_Value;
    BACNET_EVENT_STATE Event_State;
    uint32_t Time_Delay;
    uint32_t Notification_Class;
    float High_Limit;
    float Low_Limit;
    float Deadband;
    unsigned Limit_Enable : 2;
    unsigned Event_Enable : 3;
    unsigned Notify_Type : 1;
    ACKED_INFO Acked_Transitions[MAX_BACNET_EVENT_TRANSITION];
    BACNET_DATE_TIME Event_Time_Stamps[MAX_BACNET_EVENT_TRANSITION];
    /* time to generate event notification */
    uint32_t Remaining_Time_Delay;
    /* AckNotification informations */
    ACK_NOTIFICATION Ack_notify_data;
#endif

};
#endif

#endif

