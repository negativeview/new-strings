#pragma once

#include "common.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum StringDataFlag {
    STRING_DATA_ASCII,
    STRING_DATA_UTF8,
    STRING_OWNS_DATA,
    STRING_CSTRING
};

/**
 * String Data is used in several places for several purposes. It should be
 * very generic. This same data structure is used for ASCII and UTF8, as well
 * as for data that we own and data that we are merely borrowing, as well as
 * c-style strings (null terminated) and z-style strings (strings with a
 * specific length).
 */
struct string_data {
    enum StringDataFlag flags;

    uint8_t ref_count;
    uint8_t length;
    char *data;
};

/**
 * NOTE: Not currently used. Will be used for a hashing implementation that is
 *       planned for the future.
 */
bool sd_matches(struct string_data *a, struct string_data *b) {
    if (a->length != b->length) return false;

    // TODO: This might not be the best idea if flags are used for things that "don't matter" for equality.
    if (a->flags != b->flags) return false;

    return strncmp(a->data, b->data, a->length) == 0;
}

SS_RESULT sd_create(struct string_data **sd) {
    *sd = (struct string_data *)malloc(sizeof(struct string_data));
    
    (*sd)->flags = 0;
    (*sd)->length = 0;
    (*sd)->ref_count = 1;
    (*sd)->data = NULL;

    return SS_OK;
}

SS_RESULT sd_release(struct string_data *sd) {
    sd->ref_count--;
    if (sd->ref_count <= 0) {
        if ((sd->flags & STRING_OWNS_DATA) == STRING_OWNS_DATA) {
            free(sd->data);
            free(sd);
        }
    }

    return SS_OK;
}

SS_RESULT sd_print(struct string_data *sd) {
    if ((sd->flags & STRING_CSTRING) == STRING_CSTRING) {
        printf("%s", sd->data);
    } else {
        printf("%.*s", sd->length, sd->data);
    }

    return SS_OK;
}

/**
 * Creates a new string data from the given string data.
 *
 * This new string_data does NOT own its data. It will not handle the data
 * being removed out from under it gracefully. This is done as an efficiency
 * thing -- don't misuse it!
 */
SS_RESULT sd_create_from(struct string_data *in, uint8_t start, uint8_t end, struct string_data **sd) {
    SS_RESULT res = sd_create(sd);
    if (res != SS_OK) return res;

    (*sd)->flags = in->flags ^ (STRING_OWNS_DATA | STRING_CSTRING);
    (*sd)->length = end - start;
    (*sd)->data = &in->data[start];

    return SS_OK;
}

/**
 * Creates a new `string string_data` and returns a pointer. Copies the `value`
 * and considers itself to "own" the copy. You can do whatever you want with
 * the argument afterward.
 */
SS_RESULT sd_create_copy(const char *value, uint8_t length, struct string_data **sd) {
    SS_RESULT res = sd_create(sd);
    if (res != SS_OK) return res;

    (*sd)->flags = (STRING_DATA_ASCII | STRING_OWNS_DATA);
    (*sd)->length = length;
    (*sd)->data = (char *)malloc(sizeof(char) * (*sd)->length);
    strncpy((*sd)->data, value, (*sd)->length);

    return SS_OK;
}

bool sd_is_equal_cstring(struct string_data *sd, const char *compared_to) {
    if ((sd->flags & STRING_CSTRING) == STRING_CSTRING) {
        return strcmp(sd->data, compared_to) == 0;
    } else {
        return strncmp(sd->data, compared_to, sd->length) == 0;
    }
}