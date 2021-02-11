#pragma once

#include "common.h"
#include "segmented_string_piece.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    UNFILLED_TEMPLATE_STRING,
    PARTIALLY_FILLED_TEMPLATE_STRING,
    FULLY_FILLED_TEMPLATE_STRING,
    STATIC_STRING,
    STRING_LIST,
    EMPTY_STRING
} StringType;

/**
 * A segmented string is meant to be the main string type for my eventual
 * language. It makes no real effort to be super compatible with c-style
 * strings. Instead of using c-style strings for compatibility with libraries,
 * we are just going to replace those library methods.
 *
 * The primary use of these strings are for templates, where we are going to be
 * often selecting, replacing, rearranging, etc. different substrings. But I am
 * interested in exploring what this style of string does to other, more
 * traditional, string methods.
 */
struct segmented_string {
    /**
     * There are some fundamental types of string. Some operations can only be
     * performed on some types of string. Some operations alter the type of
     * string that a thing is.
     */
    StringType type;

    /**
     * This is basically a dynamic array. We dynamically allocate `pieces`.
     * Length is the amount we're currently using. Capacity is the amount we
     * have allocated space for. We generally start at 8 and go up by powers of
     * two. This should be seen as an implementation detail.
     *
     * As of now, we max out at 255 pieces. That may change in the future.
     */
    uint8_t length;
    uint8_t capacity;
    struct segmented_string_piece *pieces;
};

SS_RESULT ss_free(struct segmented_string *ss) {
    for (uint8_t i = 0; i < ss->length; i++) {
        SS_RESULT res = ssp_free(&ss->pieces[i]);
        if (res != SS_OK) return res;
    }

    free(ss->pieces);
    return SS_OK;
}

/**
 * Helper function to make sure that we have capacity for at least one more
 * segmented_string_piece. If we don't, it will allocate more space.
 *
 * Valid String Types: All
 * Return String Type: Same as was input.
 */
void _ss_increment_pieces(struct segmented_string *ss) {
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
}

/**
 * Creates a segmented string when you know details about what kind of string
 * you want. In practice, I hope that this will be used in most cases. The code
 * using this library will hopefully be able to know in the vast majority of
 * cases. This will mean that we only allocate the space needed and don't waste
 * time zeroing values that will definitely be un-zeroed soon thereafter.
 *
 * Return String Type: Whatever was passed in.
 */
SS_RESULT ss_create_initialized(StringType type, int prealloc_amount, struct segmented_string **ss) {
    *ss = (struct segmented_string *)malloc(
        sizeof(struct segmented_string)
    );
    if (*ss == NULL) return SS_ALLOC_ERROR;

    (*ss)->type = type;
    (*ss)->capacity = prealloc_amount;
    (*ss)->length = 0;
    (*ss)->pieces = (struct segmented_string_piece *)malloc(
        sizeof(struct segmented_string_piece) * prealloc_amount
    );
    if ((*ss)->pieces == NULL) return SS_ALLOC_ERROR;

    return SS_OK;
}

/**
 * Create an entirely uninitialized segmented string. This is likely not what
 * you want to do, as it is inefficient. In practice you should know the size
 * and type that you are creating.
 *
 * Return String Type: EMPTY_STRING
 */
SS_RESULT ss_create(struct segmented_string **ss) {
    *ss = (struct segmented_string *)malloc(
        sizeof(struct segmented_string)
    );
    if (*ss == NULL) return SS_ALLOC_ERROR;

    /**
     * The generated assembly for this does the EMPTY_STRING and 0 in two
     * instructions. We should be able to do in one. Is there any clean way to
     * suggest that to the compiler?
     *
     * If EMPTY_STRING were equivalent to zero, I could also just zero the
     * entire structure!
     */
    (*ss)->type = EMPTY_STRING;
    (*ss)->length = 0;
    (*ss)->capacity = 0;
    (*ss)->pieces = NULL;

    return SS_OK;
}

/**
 * Grows the segmented string by one and adds the value to the end as a static
 * string.
 *
 * Input String Type: ANY
 * Output String Type: Same as input, unless input was EMPTY_STRING. If it was
 *                     EMPTY_STRING, turns into STATIC_STRING.
 */
SS_RESULT ss_append_static_copy(struct segmented_string *ss, const char *value, int length) {
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

    _ss_increment_pieces(ss);
    ssp_init_static_copy(&ss->pieces[ss->length - 1], value, length);

    return SS_OK;
}

/**
 * Same as `ss_append_static_copy`, but take a NULL-terminated string rather
 * than a length.
 */
SS_RESULT ss_append_static_copy_static(struct segmented_string *ss, const char *value) {
    return ss_append_static_copy(ss, value, strlen(value));
}

/**
 * Takes a segmented_string_piece and appends it to this segmented_string.
 */
SS_RESULT ss_append_ssp(struct segmented_string *ss, struct segmented_string_piece *ssp) {
    _ss_increment_pieces(ss);

    switch (ssp->type) {
        case STRING_PIECE_TYPE_STATIC:
            {
                ss->pieces[ss->length - 1].type = STRING_PIECE_TYPE_STATIC;
                ss->pieces[ss->length - 1].data.static_string = ssp->data.static_string;
                ss->pieces[ss->length - 1].data.static_string->ref_count++;

                switch (ss->type) {
                    case EMPTY_STRING:
                        {
                            ss->type = STATIC_STRING;
                        }
                        break;
                }
            }
            return SS_OK;
        case STRING_PIECE_TYPE_PLACEHOLDER_UINT8:
            {
                // TODO: This should be able to alter our own type.
                ss->pieces[ss->length - 1].type = STRING_PIECE_TYPE_PLACEHOLDER_UINT8;
                ss->pieces[ss->length - 1].data.uint8_data.placeholder = ssp->data.uint8_data.placeholder;
                ss->pieces[ss->length - 1].data.uint8_data.value = ssp->data.uint8_data.value;

                switch (ss->type) {
                    case EMPTY_STRING:
                        {
                            ss->type = UNFILLED_TEMPLATE_STRING;
                        }
                        break;
                    case FULLY_FILLED_TEMPLATE_STRING:
                        {
                            ss->type = PARTIALLY_FILLED_TEMPLATE_STRING;
                        }
                        break;
                    case STATIC_STRING:
                        {
                            ss->type = UNFILLED_TEMPLATE_STRING;
                        }
                        break;
                }
            }
            return SS_OK;
        default:
            return SS_INVALID_STRING_TYPE;
    }
}

/**
 * One of our library methods. Similar to `explode` in PHP.
 *
 * Given a segmented_string and a char, turns things into a STRING_LIST split
 * by that character.
 *
 * TODO: This doesn't work entirely correctly.
 *       It doesn't do what you want if this was already a split string
 *           (possibly as the result of a template substitution)
 */
SS_RESULT ss_explode_by_char(struct segmented_string *ss, char c, struct segmented_string *out) {
    switch (ss->type) {
        case UNFILLED_TEMPLATE_STRING:
        case PARTIALLY_FILLED_TEMPLATE_STRING:
        case STRING_LIST:
            return SS_INVALID_STRING_TYPE;
        
        case EMPTY_STRING:
            // A split empty string is still an empty string.
            break;
        
        case FULLY_FILLED_TEMPLATE_STRING:
        case STATIC_STRING:
            {
                SS_RESULT res = ss_create(&out);
                if (res != SS_OK) return res;

                out->type = STRING_LIST;

                for (uint8_t i = 0; i < ss->length; i++) {
                    ssp_explode_by_char(&ss->pieces[i], out, c);
                }

                return SS_OK;
            }
        default:
            return SS_INVALID_STRING_TYPE;
    }

    return SS_ERR;
}

/**
 * Print this segmented string. In the real world, we likely want to throw a
 * runtime/compile time error in some cases. What do we do if we print a
 * template string that is not fully applied?
 *
 * A few of the specific implementations here are DEFINITELY temporary.
 */
SS_RESULT ss_print(struct segmented_string *ss) {
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
            return SS_INVALID_STRING_TYPE;
    }

    return SS_OK;
}

SS_RESULT ss_fill_uint8(struct segmented_string *ss, const char *placeholder, uint8_t value) {
    switch (ss->type) {
        case PARTIALLY_FILLED_TEMPLATE_STRING:
        case UNFILLED_TEMPLATE_STRING:
            break;
        default:
            return SS_INVALID_STRING_TYPE;
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

    return SS_OK;
}

/**
 * Clones a segmented string into a brand new copy. Currently it is exactly the
 * same except the segmented string pieces are also cloned, so their memory
 * addresses are different, but their contents should be the same.
 *
 * TODO: I could probably save some space/time if I integrated some sort of COW
 *       semantics here. Right now I'm agressively copying data away.
 */
SS_RESULT ss_clone(struct segmented_string *in, struct segmented_string **out) {
    *out = (struct segmented_string *)malloc(
        sizeof(struct segmented_string)
    );
    if (*out == NULL) return SS_ALLOC_ERROR;

    (*out)->type   = in->type;
    (*out)->length = in->length;
    (*out)->capacity = in->capacity;
    (*out)->pieces = (struct segmented_string_piece *)malloc(
        sizeof(struct segmented_string_piece) * (*out)->capacity
    );
    if ((*out)->pieces == NULL) return SS_ALLOC_ERROR;

    for (uint8_t i = 0; i < (*out)->length; i++) {
        ssp_clone(&((*out)->pieces[i]), &in->pieces[i]);
    }

    return SS_OK;
}

/**
 * Append a placeholder for a unit8 onto this list. This may change the "type"
 * of the segmented string.
 *
 * Input String Transformation: FULLY_FILLED_TEMPLATE_STRING -> PARTIALLY_FILLED_TEMPLATE_STRING
 * Input String Transformation: STATIC_STRING | EMPTy_STRING -> UNFILLED_TEMPLATE_STRING
 */
SS_RESULT ss_append_placeholder_uint8(struct segmented_string *ss, const char *placeholder) {
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
                return SS_INVALID_STRING_TYPE;
            }
            break;
    }

    _ss_increment_pieces(ss);
    ssp_init_placeholder_uint8(&ss->pieces[ss->length - 1], placeholder);

    return SS_OK;
}
