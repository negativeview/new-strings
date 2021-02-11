#pragma once

#include <stddef.h>

typedef enum {
    SS_OK,
    SS_ERR,
    SS_INVALID_STRING_TYPE,
    SS_RESULT_COUNT
} SS_RESULT;

const char* const SS_ERROR_STRS[] = {
    "OK",
    "ERROR"
};

const char *ss_error_str(SS_RESULT result) {
    if (result >= SS_RESULT_COUNT) return NULL;
    return SS_ERROR_STRS[result];
}

