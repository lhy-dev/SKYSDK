#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "sky.h"


//refere to https://github.com/Azure/azure-c-shared-utility
typedef struct SKY_MAP_HANDLE_DATA_TAG
{
    char** keys;
    char** values;
    size_t count;
    MAP_FILTER_CALLBACK mapFilterCallback;
}SKY_HANDLE_MAP;

typedef enum SKY_MAP_RESULT_TAG {
	MAP_OK,
	MAP_ERROR,
	MAP_INVALIDARG,
	MAP_KEYEXISTS,
	MAP_KEYNOTFOUND,
	MAP_FILTER_REJECT,
} SKY_MAP_RESULT;

//typedef struct SKY_MAP_HANDLE_DATA_TAG*  SKY_HANDLE_MAP;

static int mallocandstrcpy_s(char** destination, const char* source)
{
    int result;
	int copied_result;
    if ((destination == NULL) || (source == NULL))
    {
        /*If strDestination or strSource is a NULL pointer[...]these functions return EINVAL */
        result = EINVAL;
    }
    else
    {
        size_t l = strlen(source);
        char* temp = (char*)malloc(l + 1);

        if (temp == NULL)
        {
            result = ENOMEM;
        }
        else
        {
            *destination = temp;
            copied_result = strcpy_s(*destination, l + 1, source);
            if (copied_result < 0) /*strcpy_s error*/
            {
                free(*destination);
                *destination = NULL;
                result = copied_result;
            }
            else
            {
                result = 0;
            }
        }
    }
    return result;
}

bool sky_map_init ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
	return true;
}

void sky_map_uninit ( void )
{
	SKY_LOG_TRACE( "%s\n", __func__ );
}

SKY_API SKY_HANDLE sky_map_create(MAP_FILTER_CALLBACK mapFilterFunc)
{
    SKY_HANDLE_MAP* result = (SKY_HANDLE_MAP*)malloc(sizeof(SKY_HANDLE_MAP));
    if (result != NULL)
    {
        result->keys = NULL;
        result->values = NULL;
        result->count = 0;
        result->mapFilterCallback = mapFilterFunc;
    }
    return (SKY_HANDLE)result;
}

void sky_map_destroy(SKY_HANDLE handle)
{
    if (handle != NULL)
    {
        SKY_HANDLE_MAP* handleData = (SKY_HANDLE_MAP*)handle;
        size_t i;
      
        for (i = 0; i < handleData->count; i++)
        {
            free(handleData->keys[i]);
            free(handleData->values[i]);
        }
        free(handleData->keys);
        free(handleData->values);
        free(handleData);
    }
}

static char** map_clonevector(const char*const * source, size_t count)
{
    char** result;
    result = (char**)malloc(count *sizeof(char*));
    if (result == NULL)
    {

    }
    else
    {
        size_t i;
        for (i = 0; i < count; i++)
        {
            if (mallocandstrcpy_s(result + i, source[i]) != 0)
            {
                break;
            }
        }

        if (i == count)
        {
            /*it is all good, proceed to return result*/
        }
        else
        {
            size_t j;
            for (j = 0; j < i; j++)
            {
                free(result[j]);
            }
            free(result);
            result = NULL;
        }
    }
    return result;
}

SKY_API SKY_HANDLE sky_map_clone(SKY_HANDLE handle)
{
    SKY_HANDLE_MAP* result;
    if (handle == NULL)
    {
        result = NULL;
       perror("invalid arg to Map_Clone (NULL)");
    }
    else
    {
        SKY_HANDLE_MAP * handleData = (SKY_HANDLE_MAP *)handle;
        result = (SKY_HANDLE_MAP*)malloc(sizeof(SKY_HANDLE_MAP));
        if (result == NULL)
        {
           perror("unable to malloc");
        }
        else
        {
            if (handleData->count == 0)  
            {
                result->count = 0;
                result->keys = NULL;
                result->values = NULL;
                result->mapFilterCallback = NULL;
            }
            else
            {
                result->mapFilterCallback = handleData->mapFilterCallback;
                result->count = handleData->count;
                if( (result->keys = map_clonevector((const char* const*)handleData->keys, handleData->count))==NULL)
                {
                   perror("unable to clone keys");
                    free(result);
                    result = NULL;
                }
                else if ((result->values = map_clonevector((const char* const*)handleData->values, handleData->count)) == NULL)
                {
                   perror("unable to clone values");
                    size_t i;
                    for (i = 0; i < result->count; i++)
                    {
                        free(result->keys[i]); 
                    }
                    free(result->keys);
                    free(result);
                    result = NULL;
                }
                else
                {
                    /*all fine, return it*/
                }
            }
        }
    }
    return (SKY_HANDLE)result;
}

static int Map_IncreaseStorageKeysValues(SKY_HANDLE_MAP* handleData)
{
    int result;
    char** newKeys = (char**)realloc(handleData->keys, (handleData->count + 1) * sizeof(char*));
    if (newKeys == NULL)
    {
       perror("realloc error");
        result = __LINE__;
    }
    else
    {
        char** newValues;
        handleData->keys = newKeys;
        handleData->keys[handleData->count] = NULL;
        newValues = (char**)realloc(handleData->values, (handleData->count + 1) * sizeof(char*));
        if (newValues == NULL)
        {
           perror("realloc error");
            if (handleData->count == 0) /*avoiding an implementation defined behavior */
            {
                free(handleData->keys);
                handleData->keys = NULL;
            }
            else
            {
                char** undoneKeys = (char**)realloc(handleData->keys, (handleData->count) * sizeof(char*));
                if (undoneKeys == NULL)
                {
                   perror("CATASTROPHIC error, unable to undo through realloc to a smaller size");
                }
                else
                {
                    handleData->keys = undoneKeys;
                }
            }
            result = __LINE__;
        }
        else
        {
            handleData->values = newValues;
            handleData->values[handleData->count] = NULL;
            handleData->count++;
            result = 0;
        }
    }
    return result;
}

static void Map_DecreaseStorageKeysValues(SKY_HANDLE_MAP* handleData)
{
    if (handleData->count == 1)
    {
        free(handleData->keys);
        handleData->keys = NULL;
        free(handleData->values);
        handleData->values = NULL;
        handleData->count = 0;
        handleData->mapFilterCallback = NULL;
    }
    else
    {
        /*certainly > 1...*/
        char** undoneValues;
        char** undoneKeys = (char**)realloc(handleData->keys, sizeof(char*)* (handleData->count - 1)); 
        if (undoneKeys == NULL)
        {
           perror("CATASTROPHIC error, unable to undo through realloc to a smaller size");
        }
        else
        {
            handleData->keys = undoneKeys;
        }

        undoneValues = (char**)realloc(handleData->values, sizeof(char*)* (handleData->count - 1));
        if (undoneValues == NULL)
        {
           perror("CATASTROPHIC error, unable to undo through realloc to a smaller size");
        }
        else
        {
            handleData->values = undoneValues;
        }

        handleData->count--;
    }
}

static char** findKey(SKY_HANDLE_MAP* handleData, const char* key)
{
    char** result;
    if (handleData->keys == NULL)
    {
        result = NULL;
    }
    else
    {
        size_t i;
        result = NULL;
        for (i = 0; i < handleData->count; i++)
        {
            if (strcmp(handleData->keys[i], key) == 0)
            {
                result = handleData->keys + i;
                break;
            }
        }
    }
    return result;
}

static char** findValue(SKY_HANDLE_MAP* handleData, const char* value)
{
    char** result;
    if (handleData->values == NULL)
    {
        result = NULL;
    }
    else
    {
        size_t i;
        result = NULL;
        for (i = 0; i < handleData->count; i++)
        {
            if (strcmp(handleData->values[i], value) == 0)
            {
                result = handleData->values + i;
                break;
            }
        }
    }
    return result;
}

static int insertNewKeyValue(SKY_HANDLE_MAP* handleData, const char* key, const char* value)
{
    int result;
    if (Map_IncreaseStorageKeysValues(handleData) != 0) /*this increases handleData->count*/
    {
        result = __LINE__;
    }
    else
    {
        if (mallocandstrcpy_s(&(handleData->keys[handleData->count - 1]), key) != 0)
        {
            Map_DecreaseStorageKeysValues(handleData);
           perror("unable to mallocandstrcpy_s");
            result = __LINE__;
        }
        else
        {
            if (mallocandstrcpy_s(&(handleData->values[handleData->count - 1]), value) != 0)
            {
                free(handleData->keys[handleData->count - 1]);
                Map_DecreaseStorageKeysValues(handleData);
               perror("unable to mallocandstrcpy_s");
                result = __LINE__;
            }
            else
            {
                result = 0;
            }
        }
    }
    return result; 
}

SKY_API int sky_map_add(SKY_HANDLE handle, const char* key, const char* value)
{
    SKY_MAP_RESULT result;
    if (
        (handle == NULL) ||
        (key == NULL) ||
        (value == NULL)
        )
    {
        result = MAP_INVALIDARG;
    }
    else
    {
        SKY_HANDLE_MAP* handleData = (SKY_HANDLE_MAP*)handle;
        if (findKey(handleData, key) != NULL)
        {
            result = MAP_KEYEXISTS;
        }
        else
        {
            if ( (handleData->mapFilterCallback != NULL) && (handleData->mapFilterCallback(key, value) != 0) )
            {
                result = MAP_FILTER_REJECT;
            }
            else
            {
                if (insertNewKeyValue(handleData, key, value) != 0)
                {
                    result = MAP_ERROR;

                }
                else
                {
                    result = MAP_OK;
                }
            }
        }
    }
    return result;
}

SKY_API int sky_map_addorupdate(SKY_HANDLE handle, const char* key, const char* value)
{
    SKY_MAP_RESULT result;
    if (
        (handle == NULL) ||
        (key == NULL) ||
        (value == NULL)
        )
    {
        result = MAP_INVALIDARG;

    }
    else
    {
        SKY_HANDLE_MAP* handleData = (SKY_HANDLE_MAP*)handle;

        if (handleData->mapFilterCallback != NULL && handleData->mapFilterCallback(key, value) != 0)
        {
            result = MAP_FILTER_REJECT;
        }
        else
        {
            char** whereIsIt = findKey(handleData, key);
            if (whereIsIt == NULL)
            {
                if (insertNewKeyValue(handleData, key, value) != 0)
                {
                    result = MAP_ERROR;

                }
                else
                {
                    result = MAP_OK;
                }
            }
            else
            {
                size_t index = whereIsIt - handleData->keys;
                size_t valueLength = strlen(value);
                /*try to realloc value of this key*/
                char* newValue = (char*)realloc(handleData->values[index],valueLength  + 1);
                if (newValue == NULL)
                {
                    result = MAP_ERROR;

                }
                else
                {
                    memcpy(newValue, value, valueLength + 1);
                    handleData->values[index] = newValue;
                    result = MAP_OK;
                }
            }
        }
    }
    return result;
}

SKY_API int sky_map_delete(SKY_HANDLE handle, const char* key)
{
    SKY_MAP_RESULT result;
    if (
        (handle == NULL) ||
        (key == NULL)
        )
    {
        result = MAP_INVALIDARG;

    }
    else
    {
        SKY_HANDLE_MAP* handleData = (SKY_HANDLE_MAP*)handle;
        char** whereIsIt = findKey(handleData,key);
        if (whereIsIt == NULL)
        {
            result = MAP_KEYNOTFOUND;
        }
        else
        {
            size_t index = whereIsIt - handleData->keys;
            free(handleData->keys[index]);
            free(handleData->values[index]);
            memmove(handleData->keys + index, handleData->keys + index + 1, (handleData->count - index - 1)*sizeof(char*)); /*if order doesn't matter... then this can be optimized*/
            memmove(handleData->values + index, handleData->values + index + 1, (handleData->count - index - 1)*sizeof(char*));
            Map_DecreaseStorageKeysValues(handleData);
            result = MAP_OK;
        }

    }
    return result;
}

SKY_API int sky_map_containskey(SKY_HANDLE handle, const char* key, bool* keyExists)
{
    SKY_MAP_RESULT result;
    if (
        (handle ==NULL) ||
        (key == NULL) ||
        (keyExists == NULL)
        )
    {
        result = MAP_INVALIDARG;

    }
    else
    {
        SKY_HANDLE_MAP* handleData = (SKY_HANDLE_MAP*)handle;
        *keyExists = (findKey(handleData, key) != NULL) ? true: false;
        result = MAP_OK;
    }
    return result;
}

SKY_API int sky_map_containsvalue(SKY_HANDLE handle, const char* value, bool* valueExists)
{
    SKY_MAP_RESULT result;
    if (
        (handle == NULL) ||
        (value == NULL) ||
        (valueExists == NULL)
        )
    {
        result = MAP_INVALIDARG;

    }
    else
    {
        SKY_HANDLE_MAP* handleData = (SKY_HANDLE_MAP*)handle;
        *valueExists = (findValue(handleData, value) != NULL) ? true : false;
        result = MAP_OK;
    }
    return result;
}

SKY_API const char* sky_map_getvaluefromkey(SKY_HANDLE handle, const char* key)
{
    const char* result;
    if (
        (handle == NULL) ||
        (key == NULL)
        )
    {
        result = NULL;
       perror("invalid parameter to Map_GetValueFromKey");
    }
    else
    {
        SKY_HANDLE_MAP * handleData = (SKY_HANDLE_MAP *)handle;
        char** whereIsIt = findKey(handleData, key);
        if(whereIsIt == NULL)
        {
            result = NULL;
        }
        else
        {
            size_t index = whereIsIt - handleData->keys;
            result = handleData->values[index];
        }
    }
    return result;
}

