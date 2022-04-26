#include <stdio.h>
#include "cJSON.h"

// Using: https://github.com/DaveGamble/cJSON


int main ( void )
{

    // create some jSON

    cJSON *root;
    cJSON *fmt;
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
    cJSON_AddItemToObject(root, "format", fmt = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt, "type", "rect");
    cJSON_AddNumberToObject(fmt, "width", 1920);
    cJSON_AddNumberToObject(fmt, "height", 1080);
    cJSON_AddFalseToObject (fmt, "interlace");
    cJSON_AddNumberToObject(fmt, "frame rate", 24);

    char *rendered = cJSON_Print(root);

    printf("JSON output: %s\n", rendered);

    // 'read' it back

    cJSON * root2 = cJSON_Parse( rendered );

    cJSON *format = cJSON_GetObjectItemCaseSensitive(root2, "format");
    cJSON *framerate_item = cJSON_GetObjectItemCaseSensitive(format, "frame rate");
    double framerate = 0;
    if (cJSON_IsNumber(framerate_item))
    {
      framerate = framerate_item->valuedouble;
    }

    printf("framerate = %f\n", framerate);

}
