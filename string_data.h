#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_DATA_ASCII (1 << 0)
#define STRING_DATA_UTF8  (1 << 1)
#define STRING_OWNS_DATA  (1 << 2)
#define STRING_CSTRING    (1 << 3)
typedef uint8_t string_data_flag;

/**
 * String Data is used in several places for several purposes. It should be
 * very generic. This same data structure is used for ASCII and UTF8, as well
 * as for data that we own and data that we are merely borrowing, as well as
 * c-style strings (null terminated) and z-style strings (strings with a
 * specific length).
 */
struct string_data {
    string_data_flag flags;

    uint8_t length;
    char *data;
};

void sd_print(struct string_data *sd) {
    if ((sd->flags & STRING_CSTRING) == STRING_CSTRING) {
        printf("%s", sd->data);
    } else {
        printf("%.*s", sd->length, sd->data);
    }
}

/**
 * Creates a new `string string_data` and returns a pointer. Copies the `value`
 * and considers itself to "own" the copy. You can do whatever you want with
 * the argument afterward.
 */
struct string_data *sd_create_copy(const char *value) {
    struct string_data *sd = (struct string_data *)malloc(sizeof(struct string_data));
    sd->flags = (STRING_DATA_ASCII | STRING_OWNS_DATA);
    sd->length = strlen(value);
    sd->data = (char *)malloc(sizeof(char) * sd->length);
    strncpy(sd->data, value, sd->length);

    return sd;
}

struct string_data *sd_clone(struct string_data *in) {
    struct string_data *sd = (struct string_data *)malloc(sizeof(struct string_data));

    sd->flags = in->flags ^ STRING_OWNS_DATA;
    sd->length = in->length;
    sd->data = in->data;

    return sd;
}

bool sd_is_equal_cstring(struct string_data *sd, const char *compared_to) {
    if ((sd->flags & STRING_CSTRING) == STRING_CSTRING) {
        return strcmp(sd->data, compared_to) == 0;
    } else {
        return strncmp(sd->data, compared_to, sd->length) == 0;
    }
}