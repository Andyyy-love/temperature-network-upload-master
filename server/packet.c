#include "packet.h"

#include <stdio.h>
#include <string.h>

#include "cJSON.h"

int packet_from_json(packet_t *pack, const char *json)
{
    cJSON *root;
    cJSON *id;
    cJSON *time;
    cJSON *temperature;

    if (!pack || !json)
    {
        return -1;
    }

    root = cJSON_Parse(json);
    if (!root)
    {
        return -2;
    }

    id = cJSON_GetObjectItem(root, "id");
    time = cJSON_GetObjectItem(root, "time");
    temperature = cJSON_GetObjectItem(root, "temperature");

    if (!id || !time || !temperature)
    {
        cJSON_Delete(root);
        return -3;
    }

    memset(pack, 0, sizeof(*pack));
    strncpy(pack->id, id->valuestring, sizeof(pack->id) - 1);
    strncpy(pack->time, time->valuestring, sizeof(pack->time) - 1);
    pack->temperature = (float)temperature->valuedouble;

    cJSON_Delete(root);
    return 0;
}
