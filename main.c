#include "string_data.h"
#include "segmented_string.h"

int main(int argc, char *argv[]) {
    struct segmented_string *ss = ss_create();
    ss_print(ss);

    ss_append_static_copy(ss, "hello ");
    ss_print(ss);

    ss_append_placeholder_uint8(ss, "test");
    ss_print(ss);

    ss_append_static_copy(ss, "\n");
    ss_print(ss);

    for (uint8_t i = 0; i < 10; i++) {
        struct segmented_string *ss_instance = ss_clone(ss);
        ss_fill_uint8(ss_instance, "test", i);
        ss_print(ss_instance);
    }

    return 0;
}