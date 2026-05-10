#include "cJSON.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char g_json[512];
static cJSON g_root;
static cJSON g_items[8];
static int g_item_index;

cJSON *cJSON_Parse(const char *value)
{
    if (!value)
    {
        return NULL;
    }

    memset(g_json, 0, sizeof(g_json));
    strncpy(g_json, value, sizeof(g_json) - 1);
    g_item_index = 0;
    return &g_root;
}

cJSON *cJSON_GetObjectItem(const cJSON *object, const char *string)
{
    cJSON *item;
    char key[64];
    char *pos;
    char *val;
    char *end;
    size_t len;

    (void)object;

    if (!string)
    {
        return NULL;
    }

    snprintf(key, sizeof(key), "\"%s\"", string);
    pos = strstr(g_json, key);
    if (!pos)
    {
        return NULL;
    }

    val = strchr(pos + strlen(key), ':');
    if (!val)
    {
        return NULL;
    }

    val++;
    while (*val && isspace((unsigned char)*val))
    {
        val++;
    }

    item = &g_items[g_item_index++ % 8];
    memset(item, 0, sizeof(*item));

    if (*val == '"')
    {
        val++;
        end = strchr(val, '"');
        if (!end)
        {
            return NULL;
        }
        len = (size_t)(end - val);
        if (len >= sizeof(item->valuestring))
        {
            len = sizeof(item->valuestring) - 1;
        }
        memcpy(item->valuestring, val, len);
        return item;
    }

    item->valuedouble = atof(val);
    return item;
}

void cJSON_Delete(cJSON *item)
{
    (void)item;
}
