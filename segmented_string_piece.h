#pragma once

#include "string_data.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define STRING_PIECE_TYPE_STATIC             (1 << 0)
#define STRING_PIECE_TYPE_PLACEHOLDER_UINT8  (1 << 1)
typedef uint8_t piece_type;

/**
 * A template string piece is one "piece" of a template string. This can be ANY
 * piece, so this is the most dynamic part of the entire puzzle. It's basically
 * just a tagged union.
 */
struct segmented_string_piece {
    piece_type type;

    union {
        struct string_data *static_string;
        struct {
            struct string_data *placeholder;
            uint8_t value;
        } uint8_data;
    } data;
};

/** FORWARD DECLARATIONS **/
struct segmented_string;
void ss_append_ssp(struct segmented_string *ss, struct segmented_string_piece *ssp);
/** END FORWARD DECLARATIONS **/

struct segmented_string_piece *ssp_from_sd(struct string_data *sd) {
    struct segmented_string_piece *ssp = (struct segmented_string_piece *)malloc(sizeof(struct segmented_string_piece));
    ssp->type = STRING_PIECE_TYPE_STATIC;
    ssp->data.static_string = sd;
    return ssp;
}

void ssp_explode_by_char(struct segmented_string_piece *ssp, struct segmented_string *ss, char c) {
    switch (ssp->type) {
        case STRING_PIECE_TYPE_PLACEHOLDER_UINT8:
            {
                // TODO: ERROR
            }
            break;
        case STRING_PIECE_TYPE_STATIC:
            {
                uint8_t start = 0;
                uint8_t end = 0;

                while (true) {
                    if (
                        end >= ssp->data.static_string->length
                            || ssp->data.static_string->data[end] == c
                    ) {
                        struct string_data *sd = sd_create_from(
                            ssp->data.static_string,
                            start,
                            end
                        );
                        start = end + 1;

                        ss_append_ssp(
                            ss,
                            ssp_from_sd(
                                sd
                            )
                        );
                    } else {

                    }
                    if (end >= ssp->data.static_string->length) {
                        break;
                    }
                    end++;
                }
                struct string_data *sd = sd_create_from(ssp->data.static_string, start, end);
            }
            break;
    }
}

void ssp_print(struct segmented_string_piece *ssp) {
    switch (ssp->type) {
        case STRING_PIECE_TYPE_STATIC:
            {
                sd_print(ssp->data.static_string);
            }
            break;
        case STRING_PIECE_TYPE_PLACEHOLDER_UINT8:
            {
                printf("%d", ssp->data.uint8_data.value);
            }
            break;
    }
}

void ssp_clone(struct segmented_string_piece *out, struct segmented_string_piece *in) {
    switch (in->type) {
        case STRING_PIECE_TYPE_STATIC:
            {
                out->type = STRING_PIECE_TYPE_STATIC;
                out->data.static_string = sd_clone(
                    in->data.static_string
                );
            }
            break;
        case STRING_PIECE_TYPE_PLACEHOLDER_UINT8:
            {
                out->type = STRING_PIECE_TYPE_PLACEHOLDER_UINT8;
                out->data.uint8_data = in->data.uint8_data;
            }
            break;
    }
}

void ssp_fill_uint8(struct segmented_string_piece *ssp, uint8_t value) {
    // TODO: Error checking?
    ssp->data.uint8_data.value = value;
}

bool ssp_is_template(struct segmented_string_piece *ssp) {
    switch (ssp->type) {
        case STRING_PIECE_TYPE_PLACEHOLDER_UINT8:
            return true;
        default:
            return false;
    }
}

bool ssp_is_template_uint8(struct segmented_string_piece *ssp, const char *placeholder) {
    if (ssp->type != STRING_PIECE_TYPE_PLACEHOLDER_UINT8) return false;
    return sd_is_equal_cstring(ssp->data.uint8_data.placeholder, placeholder);
}

void ssp_init_static_copy(struct segmented_string_piece *ssp, const char *value, uint8_t length) {
    ssp->type = STRING_PIECE_TYPE_STATIC;
    ssp->data.static_string = sd_create_copy(value, length);
}

void ssp_init_placeholder_uint8(struct segmented_string_piece *ssp, const char *placeholder) {
    ssp->type = STRING_PIECE_TYPE_PLACEHOLDER_UINT8;
    ssp->data.uint8_data.placeholder = sd_create_copy(placeholder, strlen(placeholder));
}
