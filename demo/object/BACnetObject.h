/*
 * BACnetObjectInterface.h
 *
 *  Created on: Dec 3, 2011
 *      Author: Ed Hague
 */

#pragma once

#include <vector>
#include "bacstr.h"


class BACnetObject
{
public:
#if 0
    std::string bacnetDescription;
    std::string objectName;
    BACNET_OBJECT_TYPE bacnetType;
#endif

    BACnetObject(uint32_t nInstance, std::string &pName, std::string &pDescription) :
        instance(nInstance),
        name(pName),
        description(pDescription)
        //Reliability2(RELIABILITY_NO_FAULT_DETECTED)
    {
        // get those other initializers
        // BACnetObject();
    }

    explicit BACnetObject() // :
    // Out_Of_Service(false),
    {}

    // BACNET_OBJECT_TYPE  type;
    uint32_t            instance = 0 ;
    bool                Out_Of_Service = false;
    bool                Changed = false;
    std::string         name;
    std::string         description;
    BACNET_RELIABILITY  Reliability = RELIABILITY_NO_FAULT_DETECTED;

    static bool Valid_Instance(
        std::vector<BACnetObject *> *objects,
        uint32_t object_instance)
    {
        std::vector<BACnetObject *>::iterator it;
        for (it = objects->begin(); it != objects->end(); ++it) {
            if (object_instance == (*it)->instance) {
                return true;
            }
        }
        return false;
    }


    static BACnetObject *Instance_To_Object(
        std::vector<BACnetObject *> *objects,
        uint32_t object_instance)
    {
        for (std::vector<BACnetObject *>::iterator it = objects->begin(); it != objects->end(); ++it) {
            if (object_instance == (*it)->instance) {
                return *it;
            }
        }
// todo2        dbTraffic(DB_BTC_ERROR, "Illegal index, %s, %d", __FILE__, __LINE__);
        return NULL;
    }

    static bool Object_Name(
        std::vector<BACnetObject *> *objects,
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name)
    {
        for (std::vector<BACnetObject *>::iterator it = objects->begin(); it != objects->end(); ++it) {
            if (object_instance == (*it)->instance) {
                characterstring_init_ansi(object_name, (*it)->name.c_str());
                return true;
            }
        }
        return false ;
    }
#if 0    
    typedef enum {
        RP_SEARCH_MORE,
        RP_FOUND_OK,
        RP_FAILED
    } RP_RETURN ;
#endif
#if 0
    virtual RP_RETURN Read_Property(BACNET_READ_PROPERTY_DATA * rpdata, int * const len )
    {
        int apdu_len = 0; /* return value */
        BACNET_BIT_STRING bit_string;
        BACNET_CHARACTER_STRING char_string;
        // unsigned object_index;
        uint8_t *apdu = NULL;
        // const int *pRequired = NULL, *pOptional = NULL, *pProprietary = NULL;
        RP_RETURN   rc = RP_FOUND_OK;

        if ((rpdata == NULL) || (rpdata->application_data == NULL) || (rpdata->application_data_len == 0)) {
            *len = 0;   // really. 0. todonext2 - why 0 Steve? why not error code??
            return RP_FAILED;
        }

        rpdata->error_class = ERROR_CLASS_OBJECT;
        rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;

        apdu = rpdata->application_data;
        switch ((int)rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(&apdu[0], bacnetType, rpdata->object_instance);
            break;

        case PROP_OBJECT_NAME:
            /* note: object name must be unique in our device */
            characterstring_init_ansi(&char_string, objectName.c_str());
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;

        case PROP_DESCRIPTION:
            characterstring_init_ansi(&char_string, bacnetDescription.c_str());
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;

        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], bacnetType);
            break;

        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, Out_Of_Service);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;

        case PROP_OUT_OF_SERVICE:
            apdu_len = encode_application_boolean(&apdu[0], Out_Of_Service);
            break;

        default:
            rc = RP_SEARCH_MORE ;
            break;
        }

        *len = apdu_len;
        return rc;
    }
#endif

#if 0
    virtual RP_RETURN Write_Property(BACNET_WRITE_PROPERTY_DATA *wp_data, BACNET_APPLICATION_DATA_VALUE *value, bool * const boolReturn)
    {
        switch ((int)wp_data->object_property) {
        case PROP_OUT_OF_SERVICE:
            *boolReturn = WPValidateArgType(value, BACNET_APPLICATION_TAG_BOOLEAN, &wp_data->error_class, &wp_data->error_code);
            if (!*boolReturn) {
                // WPValidateArgType sets wp_data error class/code
                return RP_FAILED;
            }
            Out_Of_Service = value->type.Boolean ;
            return RP_FOUND_OK;
        }
        return RP_SEARCH_MORE;
    }
#endif

};

class BACnetObjectWithPV : public BACnetObject
{

public:
    BACnetObjectWithPV(uint32_t nInstance, std::string &name, std::string &description) : BACnetObject(nInstance, name, description)
    {
    }

    bool Change_Of_Value = false ;

    void Out_Of_Service_Set(bool value)
    {
        if (Out_Of_Service != value) {
            Change_Of_Value = true;
        }
        Out_Of_Service = value;
    }

#if 0
    // todonext, can this be private?

    BACnetObjectWithPV()
    {
        Out_Of_Service = false;
        Change_Of_Value = false;
    }

    virtual RP_RETURN Read_Property(BACNET_READ_PROPERTY_DATA * rpdata, int * const apdu_len)
    {
        uint8_t *apdu ;

        apdu = rpdata->application_data;

        switch ( rpdata->object_property) {
        case PROP_RELIABILITY:
            *apdu_len = encode_application_enumerated(&apdu[0], Reliability );
            return RP_FOUND_OK;

            // todo 1 - this is part of the 'property read chain'. Check it out during testing!
        }

        return BACnetObject::Read_Property(rpdata, apdu_len);
    }
#endif
};

//BACnetObject *Generic_Instance_To_Object(
//    std::vector<BACnetObject *> objects,
//    uint32_t object_instance);
