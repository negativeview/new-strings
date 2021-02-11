#pragma once

#include "segmented_string_piece.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define UNFILLED_TEMPLATE_STRING         (1 << 0)
#define PARTIALLY_FILLED_TEMPLATE_STRING (1 << 1)
#define FULLY_FILLED_TEMPLATE_STRING     (1 << 2)
#define STATIC_STRING                    (1 << 3)
#define STRING_LIST                      (1 << 4)
#define EMPTY_STRING                     (1 << 5)
typedef uint8_t string_type;

struct segmented_string {
    string_type type;

    // We currently only support 255 total "pieces."
    uint8_t length;
    uint8_t capacity;
    struct segmented_string_piece *pieces;
};

struct segmented_string *ss_create_initialized(string_type type, int prealloc_amount) {
    struct segmented_string *ss = (struct segmented_string *)malloc(
        sizeof(struct segmented_string)
    );

    ss->type = type;
    ss->capacity = prealloc_amount;
    ss->length = 0;
    ss->pieces = (struct segmented_string_piece *)malloc(
        sizeof(struct segmented_string_piece) * prealloc_amount
    );

    return ss;
}

/**
 * Create an entirely uninitialized segmented string. This is likely not what
 * you want to do, as it is inefficient. In practice you should know the size
 * and type that you are creating.
 */
struct segmented_string *ss_create() {
    struct segmented_string *ss = (struct segmented_string *)malloc(
        sizeof(struct segmented_string)
    );

    /**
     * The generated assembly for this does the EMPTY_STRING and 0 in two
     * instructions. We should be able to do in one. Is there any clean way to
     * suggest that to the compiler?
     *
     * If EMPTY_STRING were equivalent to zero, I could also just zero the
     * entire structure!
     */
    ss->type = EMPTY_STRING;
    ss->length = 0;
    ss->capacity = 0;
    ss->pieces = NULL;

    return ss;
}

void ss_append_static_copy(struct segmented_string *ss, const char *value, int length) {
    switch(ss->type) {
        case UNFILLED_TEMPLATE_STRING:
        case PARTIALLY_FILLED_TEMPLATE_STRING:
        case FULLY_FILLED_TEMPLATE_STRING:
        case STATIC_STRING:
        case STRING_LIST:
            {
                // Same
            }
            break;
        case EMPTY_STRING:
            {
                ss->type = STATIC_STRING;
            }
            break;
    }

    ss->length++;
    if (ss->length > ss->capacity) {
        if (ss->capacity < 8) {
            ss->capacity = 8;
        } else {
            ss->capacity *= 2;
        }

        ss->pieces = (struct segmented_string_piece *)realloc(
            ss->pieces,
            sizeof(struct segmented_string_piece) * ss->capacity
        );
    }
    
    ssp_init_static_copy(&ss->pieces[ss->length - 1], value, length);
}

void ss_append_static_copy_static(struct segmented_string *ss, const char *value) {
    ss_append_static_copy(ss, value, strlen(value));
}

void ss_append_ssp(struct segmented_string *ss, struct segmented_string_piece *ssp) {
    ss_append_static_copy(ss, ssp->data.static_string->data, ssp->data.static_string->length);
}

struct segmented_string *ss_explode_by_char(struct segmented_string *ss, char c) {
    switch (ss->type) {
        case UNFILLED_TEMPLATE_STRING:
        case PARTIALLY_FILLED_TEMPLATE_STRING:
        case STRING_LIST:
            // TODO: Error.
            break;
        
        case EMPTY_STRING:
            // A split empty string is still an empty string.
            break;
        
        case FULLY_FILLED_TEMPLATE_STRING:
        case STATIC_STRING:
            {
                struct segmented_string *ret = ss_create();
                ret->type = STRING_LIST;

                for (uint8_t i = 0; i < ss->length; i++) {
                    ssp_explode_by_char(&ss->pieces[i], ret, c);
                }

                return ret;
            }
    }

    return NULL;
}

/**
 * Print this segmented string. In the real world, we likely want to throw a
 * runtime/compile time error in some cases. What do we do if we print a
 * template string that is not fully applied?
 *
 * A few of the specific implementations here are DEFINITELY temporary.
 */
void ss_print(struct segmented_string *ss) {
    switch (ss->type) {
        case UNFILLED_TEMPLATE_STRING:
            {
                printf("unfilled_template_string:");
                for (uint8_t i = 0; i < ss->length; i++) {
                    ssp_print(&ss->pieces[i]);
                }
            }
            break;
        case PARTIALLY_FILLED_TEMPLATE_STRING:
            {
                printf("partially_filled_template_string:");
                for (uint8_t i = 0; i < ss->length; i++) {
                    ssp_print(&ss->pieces[i]);
                }
            }
            break;
        case FULLY_FILLED_TEMPLATE_STRING:
            {
                /**
                 * This is functionally the same as STATIC_LIST. We currently
                 * have it as its own case because we're printing the type.
                 * When/if we stop printing the type, we will likely collapse
                 * the cases. We mostly keep them as separate types for useful
                 * metadata.
                 */
                printf("fully_filled_template_string:");
                for (uint8_t i = 0; i < ss->length; i++) {
                    ssp_print(&ss->pieces[i]);
                }
            }
            break;
        case STATIC_STRING:
            {
                printf("static_string:");
                for (uint8_t i = 0; i < ss->length; i++) {
                    ssp_print(&ss->pieces[i]);
                }
            }
            break;
        case STRING_LIST:
            {
                printf("string_list:");
                for (uint8_t i = 0; i < ss->length; i++) {
                    printf("\n\t");
                    ssp_print(&ss->pieces[i]);
                }
                printf("\n");
            }
            break;
        case EMPTY_STRING:
            {
                printf("empty_string:NULL\n");
            }
            break;
        default:
            {
                printf("ss_print:INVALID:%d\n", ss->type);
            }
            break;
    }
}

// TODO: Error if not found.
void ss_fill_uint8(struct segmented_string *ss, const char *placeholder, uint8_t value) {
    switch (ss->type) {
        case PARTIALLY_FILLED_TEMPLATE_STRING:
        case UNFILLED_TEMPLATE_STRING:
            break;
        default:
            // TODO: Error.
            return;
    }

    bool has_unfilled = false;

    for (uint8_t i = 0; i < ss->length; i++) {
        if (ssp_is_template(&ss->pieces[i])) {
            if (ssp_is_template_uint8(&ss->pieces[i], placeholder)) {
                ssp_fill_uint8(&ss->pieces[i], value);
            } else {
                has_unfilled = true;
            }
        }
    }

    if (has_unfilled) {
        ss->type = PARTIALLY_FILLED_TEMPLATE_STRING;
    } else {
        ss->type = FULLY_FILLED_TEMPLATE_STRING;
    }
}

struct segmented_string *ss_clone(struct segmented_string *in) {
    struct segmented_string *ss = (struct segmented_string *)malloc(
        sizeof(struct segmented_string)
    );
    ss->type   = in->type;
    ss->length = in->length;
    ss->pieces = (struct segmented_string_piece *)malloc(
        sizeof(struct segmented_string_piece) * ss->length
    );

    for (uint8_t i = 0; i < ss->length; i++) {
        ssp_clone(&ss->pieces[i], &in->pieces[i]);
    }

    return ss;
}

void ss_append_placeholder_uint8(struct segmented_string *ss, const char *placeholder) {
    switch(ss->type) {
        case UNFILLED_TEMPLATE_STRING:
        case PARTIALLY_FILLED_TEMPLATE_STRING:
            {

            }
            break;

        case FULLY_FILLED_TEMPLATE_STRING:
            {
                ss->type = PARTIALLY_FILLED_TEMPLATE_STRING;
            }
            break;

        case STATIC_STRING:
        case EMPTY_STRING:
            {
                ss->type = UNFILLED_TEMPLATE_STRING;
            }
            break;

        case STRING_LIST:
            {
                // Should be an error.
            }
            break;
    }

    ss->length++;
    if (ss->length > ss->capacity) {
        if (ss->capacity < 8) {
            ss->capacity = 8;
        } else {
            ss->capacity *= 2;
        }

        ss->pieces = (struct segmented_string_piece *)realloc(
            ss->pieces,
            sizeof(struct segmented_string_piece) * ss->capacity
        );
    }

    ssp_init_placeholder_uint8(&ss->pieces[ss->length - 1], placeholder);
}
