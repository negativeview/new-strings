#pragma once

#include "template_string_piece.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This is the unfilled template string.
 */
struct template_string {
    // We currently only support 255 total "pieces."
    uint8_t length;
    struct template_string_piece *pieces;
};

struct template_string *ts_create() {
    struct template_string *ts = (struct template_string *)malloc(sizeof(struct template_string));
    ts->length = 0;
    ts->pieces = NULL;

    return ts;
}

void ts_print(struct template_string *ts) {
    for (uint8_t i = 0; i < ts->length; i++) {
        tsp_print(&ts->pieces[i]);
    }
}

// TODO: Error if not found.
void ts_fill_uint8(struct template_string *ts, const char *placeholder, uint8_t value) {
    for (uint8_t i = 0; i < ts->length; i++) {
        if (tsp_is_template_uint8(&ts->pieces[i], placeholder)) {
            tsp_fill_uint8(&ts->pieces[i], value);
            break;
        }
    }
}

struct template_string *ts_clone(struct template_string *in) {
    struct template_string *ts = (struct template_string *)malloc(sizeof(struct template_string));
    ts->length = in->length;
    ts->pieces = (struct template_string_piece *)malloc(
        sizeof(struct template_string_piece) * ts->length
    );

    for (uint8_t i = 0; i < ts->length; i++) {
        tsp_clone(&ts->pieces[i], &in->pieces[i]);
    }

    return ts;
}

void ts_append_static_copy(struct template_string *ts, const char *value) {
    ts->length++;
    ts->pieces = (struct template_string_piece *)realloc(ts->pieces, sizeof(struct template_string_piece) * ts->length);
    
    tsp_init_static_copy(&ts->pieces[ts->length - 1], value);
}

void ts_append_placeholder_uint8(struct template_string *ts, const char *placeholder) {
    ts->length++;
    ts->pieces = (struct template_string_piece *)realloc(ts->pieces, sizeof(struct template_string_piece) * ts->length);

    tsp_init_placeholder_uint8(&ts->pieces[ts->length - 1], placeholder);
}

