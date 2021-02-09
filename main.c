#include "string_data.h"
#include "template_string.h"

int main(int argc, char *argv[]) {
    struct template_string *ts = ts_create();
    ts_append_static_copy(ts, "hello ");
    ts_append_placeholder_uint8(ts, "test");
    ts_append_static_copy(ts, "\n");

    for (uint8_t i = 0; i < 10; i++) {
        struct template_string *ts_instance = ts_clone(ts);
        ts_fill_uint8(ts_instance, "test", i);
        ts_print(ts_instance);
    }

    return 0;
}