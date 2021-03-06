#pragma once

#include <cseries/cseries.h>

/* ---------- constants */

enum // data array flags
{
    _data_array_can_disconnect_bit,
    _data_array_disconnected_bit,
    _data_array_protection_bit,
    _data_array_verify_data_pattern_bit
};

/* ---------- structures */

struct s_datum_header
{
    short identifier;
};

struct s_data_array
{
    char name[32];
    long maximum_count;
    long size;
    char alignment_bits;
    bool valid;
    word flags;
    dword signature;
    void *allocation;
    long first_possibly_free_absolute_index;
    long count;
    long actual_count;
    short next_identifier;
    short isolated_next_identifier;
    void *data;
    void *in_use_bit_vector;
    dword offset_to_data;
    dword offset_to_bit_vector;
};

struct s_data_iterator
{
    s_data_array *data;
    long index;
    long absolute_index;
};

/* ---------- prototypes/data.cpp */

long data_next_index(s_data_array *data, long index);
void *datum_try_and_get(const s_data_array *data, long index);