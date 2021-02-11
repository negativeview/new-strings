#include "common.h"
#include "string_data.h"
#include "segmented_string.h"

#include <assert.h>

int main(int argc, char *argv[]) {
    /**
     * Test that preinitializing puts us into a pretty sane state.
     */
    struct segmented_string *ss;
    assert(SS_OK == ss_create_initialized(STATIC_STRING, 2, &ss));
    assert(ss->type == STATIC_STRING);
    assert(ss->capacity == 2);
    assert(ss->length == 0);

    /**
     * Take that preallocated segmented string and append a single static string to it. Is that sane?
     */
    assert(SS_OK == ss_append_static_copy_static(ss, "hello world mate"));
    assert(ss->type == STATIC_STRING);
    assert(ss->capacity == 2);
    assert(ss->length == 1);

    /**
     * Now append a placeholder. Did things work as we expect?
     */
    assert(SS_OK == ss_append_placeholder_uint8(ss, "test"));
    assert(ss->type == UNFILLED_TEMPLATE_STRING);
    assert(ss->capacity == 2);
    assert(ss->length == 2);

    /**
     * Populate that placeholder. Are we good?
     */
    assert(SS_OK == ss_fill_uint8(ss, "test", 8));
    assert(ss->type == FULLY_FILLED_TEMPLATE_STRING);
    assert(ss->capacity == 2);
    assert(ss->length == 2);

    /**
     * More in depth tests given the above. First for the static string.
     */
    assert(ss->pieces[0].type == STRING_PIECE_TYPE_STATIC);
    assert((ss->pieces[0].data.static_string->flags & STRING_DATA_ASCII) == STRING_DATA_ASCII);
    assert((ss->pieces[0].data.static_string->flags & STRING_OWNS_DATA) == STRING_OWNS_DATA);
    assert(ss->pieces[0].data.static_string->flags != STRING_CSTRING);
    assert(ss->pieces[0].data.static_string->length == strlen("hello world mate"));
    assert(strncmp(ss->pieces[0].data.static_string->data, "hello world mate", ss->pieces[0].data.static_string->length) == 0);
    assert(ss->pieces[0].data.static_string->ref_count == 1);

    /**
     * Now more in-depth tests for the placeholder.
     */
    assert(ss->pieces[1].type == STRING_PIECE_TYPE_PLACEHOLDER_UINT8);
    assert(ss->pieces[1].data.uint8_data.value == 8);
    assert((ss->pieces[1].data.uint8_data.placeholder->flags & STRING_DATA_ASCII) == STRING_DATA_ASCII);
    assert(ss->pieces[1].data.uint8_data.placeholder->flags != STRING_CSTRING);
    assert(strncmp(ss->pieces[1].data.uint8_data.placeholder->data, "test", strlen("test")) == 0);

    /**
     * Create a completely blank segmented string.
     */
    struct segmented_string *ss2;
    assert(SS_OK == ss_create(&ss2));
    assert(ss2->type == EMPTY_STRING);
    assert(ss2->length == 0);
    assert(ss2->capacity == 0);
    assert(ss2->pieces == NULL);

    /**
     * Steal one of the pieces that we made earlier.
     */
    assert(SS_OK == ss_append_ssp(ss2, &ss->pieces[0]));
    assert(ss2->type == STATIC_STRING);
    assert(ss2->length == 1);
    assert(ss2->capacity == 8);
    assert(ss2->pieces[0].type == STRING_PIECE_TYPE_STATIC);
    assert(ss2->pieces[0].data.static_string == ss->pieces[0].data.static_string);

    assert(SS_OK == ss_free(ss));
    assert(SS_OK == ss_free(ss2));

    return 0;
}