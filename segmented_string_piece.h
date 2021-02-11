#pragma once

#include "common.h"
#include "string_data.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

enum StringPieceType {
    STRING_PIECE_TYPE_STATIC,
    STRING_PIECE_TYPE_PLACEHOLDER_UINT8
};

/**
 * A template string piece is one "piece" of a template string. This can be ANY
 * piece, so this is the most dynamic part of the entire puzzle. It's basically
 * just a tagged union.
 */
struct segmented_string_piece {
    enum StringPieceType type;

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
SS_RESULT ss_append_ssp(struct segmented_string *ss, struct segmented_string_piece *ssp);
/** END FORWARD DECLARATIONS **/

SS_RESULT ssp_free(struct segmented_string_piece *ssp) {
    switch (ssp->type) {
        case STRING_PIECE_TYPE_STATIC:
            {
                sd_release(ssp->data.static_string);
            }
            break;
        case STRING_PIECE_TYPE_PLACEHOLDER_UINT8:
            {
                sd_release(ssp->data.uint8_data.placeholder);
            }
            break;
        default:
            return SS_ERR;
    }
    return SS_OK;
}

SS_RESULT ssp_from_sd(struct string_data *sd, struct segmented_string_piece **ssp) {
    *ssp = (struct segmented_string_piece *)malloc(sizeof(struct segmented_string_piece));
    (*ssp)->type = STRING_PIECE_TYPE_STATIC;
    (*ssp)->data.static_string = sd;
    return SS_OK;
}

SS_RESULT ssp_explode_by_char(struct segmented_string_piece *ssp, struct segmented_string *ss, char c) {
    switch (ssp->type) {
        case STRING_PIECE_TYPE_PLACEHOLDER_UINT8:
            {
                return SS_ERR;
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
                        struct string_data *sd;
                        SS_RESULT res = sd_create_from(
                            ssp->data.static_string,
                            start,
                            end,
                            &sd
                        );
                        if (res != SS_OK) return res;

                        start = end + 1;

                        struct segmented_string_piece *ssp;
                        res = ssp_from_sd(sd, &ssp);
                        if (res != SS_OK) return res;

                        ss_append_ssp(
                            ss,
                            ssp
                        );
                    } else {

                    }
                    if (end >= ssp->data.static_string->length) {
                        break;
                    }
                    end++;
                }

                // TODO: Why are we even doing this??
                struct string_data *sd;
                SS_RESULT res = sd_create_from(ssp->data.static_string, start, end, &sd);
                if (res != SS_OK) return res;
            }
            break;
    }

    return SS_OK;
}

SS_RESULT ssp_print(struct segmented_string_piece *ssp) {
    switch (ssp->type) {
        case STRING_PIECE_TYPE_STATIC:
            {
                return sd_print(ssp->data.static_string);
            }
            break;
        case STRING_PIECE_TYPE_PLACEHOLDER_UINT8:
            {
                printf("%d", ssp->data.uint8_data.value);
            }
            break;
        default:
            return SS_ERR;
    }

    return SS_OK;
}

SS_RESULT ssp_clone(struct segmented_string_piece *out, struct segmented_string_piece *in) {
    switch (in->type) {
        case STRING_PIECE_TYPE_STATIC:
            {
                out->type = STRING_PIECE_TYPE_STATIC;
                out->data.static_string = in->data.static_string;
                out->data.static_string->ref_count++;
            }
            break;
        case STRING_PIECE_TYPE_PLACEHOLDER_UINT8:
            {
                out->type = STRING_PIECE_TYPE_PLACEHOLDER_UINT8;
                out->data.uint8_data = in->data.uint8_data;
            }
            break;
        default:
            return SS_ERR;
    }

    return SS_OK;
}

SS_RESULT ssp_fill_uint8(struct segmented_string_piece *ssp, uint8_t value) {
    // TODO: Error checking?
    ssp->data.uint8_data.value = value;

    return SS_OK;
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

SS_RESULT ssp_init_static_copy(struct segmented_string_piece *ssp, const char *value, uint8_t length) {
    ssp->type = STRING_PIECE_TYPE_STATIC;

    return sd_create_copy(value, length, &(ssp->data.static_string));
}

SS_RESULT ssp_init_placeholder_uint8(struct segmented_string_piece *ssp, const char *placeholder) {
    ssp->type = STRING_PIECE_TYPE_PLACEHOLDER_UINT8;
    return sd_create_copy(placeholder, strlen(placeholder), &(ssp->data.uint8_data.placeholder));
}
