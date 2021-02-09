#pragma once

#include "string_data.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define STRING_PIECE_TYPE_STATIC             (1 << 0)
#define STRING_PIECE_TYPE_PLACEHOLDER_UINT8  (1 << 1)
typedef uint8_t piece_type;

/**
 * A template string piece is one "piece" of a template string. This can be ANY piece, so this is the most dynamic part of the entire puzzle.
 */
struct template_string_piece {
    piece_type type;

    union {
        struct string_data *static_string;
        struct {
            struct string_data *placeholder;
            uint8_t value;
        } uint8_data;
    } data;
};

void tsp_print(struct template_string_piece *tsp) {
    switch (tsp->type) {
        case STRING_PIECE_TYPE_STATIC:
            {
                sd_print(tsp->data.static_string);
            }
            break;
        case STRING_PIECE_TYPE_PLACEHOLDER_UINT8:
            {
                printf("%d", tsp->data.uint8_data.value);
            }
            break;
    }
}

void tsp_clone(struct template_string_piece *out, struct template_string_piece *in) {
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

void tsp_fill_uint8(struct template_string_piece *tsp, uint8_t value) {
    // TODO: Error checking?
    tsp->data.uint8_data.value = value;
}

bool tsp_is_template_uint8(struct template_string_piece *tsp, const char *placeholder) {
    if (tsp->type != STRING_PIECE_TYPE_PLACEHOLDER_UINT8) return false;
    return sd_is_equal_cstring(tsp->data.uint8_data.placeholder, placeholder);
}

void tsp_init_static_copy(struct template_string_piece *tsp, const char *value) {
    tsp->type = STRING_PIECE_TYPE_STATIC;
    tsp->data.static_string = sd_create_copy(value);
}

void tsp_init_placeholder_uint8(struct template_string_piece *tsp, const char *placeholder) {
    tsp->type = STRING_PIECE_TYPE_PLACEHOLDER_UINT8;
    tsp->data.uint8_data.placeholder = sd_create_copy(placeholder);
}