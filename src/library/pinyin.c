#include "library/pinyin.h"

#include <stddef.h>

enum {
    HEADER_BYTES = 12,
    BLOCK_SHIFT = 5,
    BLOCK_SIZE = 1 << BLOCK_SHIFT,
    SYLLABLE_BYTES = 16
};

static uint16_t get16(const uint8_t *bytes)
{
    return (uint16_t)(bytes[0] | (uint16_t)bytes[1] << 8);
}

static uint32_t get32(const uint8_t *bytes)
{
    return (uint32_t)bytes[0] | (uint32_t)bytes[1] << 8 | (uint32_t)bytes[2] << 16 |
           (uint32_t)bytes[3] << 24;
}

static uint32_t crc32_update(uint32_t crc, const uint8_t *bytes, uint32_t length)
{
    for (uint32_t index = 0; index < length; index++) {
        crc ^= bytes[index];
        for (int bit = 0; bit < 8; bit++) {
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
        }
    }
    return crc;
}

static bool table_header_ok(const uint8_t *data, uint32_t size)
{
    return data != NULL && size >= HEADER_BYTES && data[0] == 'E' && data[1] == 'P' &&
           data[2] == 'Y' && data[3] == '1' && data[4] == 1 && data[5] == BLOCK_SHIFT;
}

bool pinyin_table_open(PinyinTable *table, const uint8_t *data, uint32_t size)
{
    if (table == NULL || !table_header_ok(data, size) || size < HEADER_BYTES + 4) {
        return false;
    }
    uint32_t payload_size = size - 4;
    if ((crc32_update(0xFFFFFFFFu, data, payload_size) ^ 0xFFFFFFFFu) !=
        get32(data + payload_size)) {
        return false;
    }
    uint32_t count = get16(data + 6);
    uint32_t blocks = (count + BLOCK_SIZE - 1) / BLOCK_SIZE;
    uint32_t index_size = blocks * 4u;
    if (count == 0 || HEADER_BYTES + index_size > payload_size) {
        return false;
    }
    table->data = data;
    table->size = payload_size;
    table->index = data + HEADER_BYTES;
    table->records = data + HEADER_BYTES + index_size;
    table->count = (uint16_t)count;
    table->blocks = (uint16_t)blocks;
    uint32_t records_start = HEADER_BYTES + index_size;
    uint32_t record_end = records_start;
    uint32_t largest_syllable = 0;
    uint32_t decoded_crc = 0xFFFFFFFFu;
    for (uint32_t block = 0; block < blocks; block++) {
        uint32_t first = get16(table->index + block * 4u);
        uint32_t offset = get16(table->index + block * 4u + 2u);
        uint32_t position = records_start + offset;
        uint32_t previous = 0;
        if (position > payload_size) {
            return false;
        }
        uint32_t in_block = count - block * BLOCK_SIZE;
        if (in_block > BLOCK_SIZE) {
            in_block = BLOCK_SIZE;
        }
        for (uint32_t index = 0; index < in_block; index++) {
            if (position >= payload_size) {
                return false;
            }
            uint32_t delta = data[position++];
            if (delta == 0xFF) {
                if (position + 2 > payload_size) {
                    return false;
                }
                delta = get16(data + position);
                position += 2;
            }
            uint32_t codepoint = index == 0 ? first : previous + delta;
            if (codepoint > 0xFFFF) {
                return false;
            }
            previous = codepoint;
            if (position >= payload_size) {
                return false;
            }
            uint32_t syllable = data[position++];
            if (syllable == 0xFF) {
                if (position >= payload_size) {
                    return false;
                }
                syllable = 0xFFu + data[position++];
            }
            if (syllable > largest_syllable) {
                largest_syllable = syllable;
            }
            uint8_t decoded[4] = {(uint8_t)codepoint, (uint8_t)(codepoint >> 8),
                                  (uint8_t)syllable, (uint8_t)(syllable >> 8)};
            decoded_crc = crc32_update(decoded_crc, decoded, sizeof decoded);
        }
        if (position > record_end) {
            record_end = position;
        }
    }
    if (record_end + 2 > payload_size || (decoded_crc ^ 0xFFFFFFFFu) != get32(data + 8)) {
        return false;
    }
    uint32_t syllable_count = get16(data + record_end);
    if (syllable_count == 0 || syllable_count > PINYIN_MAX_SYLLABLES) {
        return false;
    }
    table->syllable_count = (uint16_t)syllable_count;
    table->syllables = data + record_end + 2;
    uint32_t position = record_end + 2;
    for (uint32_t index = 0; index < syllable_count; index++) {
        if (position >= payload_size) {
            return false;
        }
        uint32_t length = data[position++];
        if (position + length > size) {
            return false;
        }
        table->syllable_offsets[index] = (uint16_t)(position - (record_end + 2));
        position += length;
    }
    return largest_syllable < syllable_count;
}

static bool syllable_at(const PinyinTable *table, uint32_t syllable, char *text,
                        uint32_t capacity)
{
    if (table == NULL || text == NULL || syllable >= table->syllable_count) {
        return false;
    }
    const uint8_t *start = table->syllables + table->syllable_offsets[syllable];
    uint32_t length = start[-1];
    if (length + 1 > capacity) {
        return false;
    }
    for (uint32_t index = 0; index < length; index++) {
        text[index] = (char)start[index];
    }
    text[length] = '\0';
    return true;
}

bool pinyin_table_syllable(const PinyinTable *table, uint32_t codepoint, char *text,
                           uint32_t capacity)
{
    if (table == NULL || text == NULL || codepoint > 0xFFFF || table->count == 0) {
        return false;
    }
    uint32_t low = 0;
    uint32_t high = table->blocks;
    while (low < high) {
        uint32_t middle = low + (high - low) / 2;
        if (get16(table->index + middle * 4u) <= codepoint) {
            low = middle + 1;
        } else {
            high = middle;
        }
    }
    if (low == 0) {
        return false;
    }
    uint32_t block = low - 1;
    uint32_t first = get16(table->index + block * 4u);
    uint32_t offset = get16(table->index + block * 4u + 2u);
    uint32_t position = (uint32_t)(table->records - table->data) + offset;
    uint32_t previous = 0;
    uint32_t in_block = table->count - block * BLOCK_SIZE;
    if (in_block > BLOCK_SIZE) {
        in_block = BLOCK_SIZE;
    }
    for (uint32_t index = 0; index < in_block; index++) {
        if (position >= table->size) {
            return false;
        }
        uint32_t delta = table->data[position++];
        if (delta == 0xFF) {
            if (position + 2 > table->size) {
                return false;
            }
            delta = get16(table->data + position);
            position += 2;
        }
        uint32_t current_codepoint = index == 0 ? first : previous + delta;
        previous = current_codepoint;
        if (position >= table->size) {
            return false;
        }
        uint32_t syllable = table->data[position++];
        if (syllable == 0xFF) {
            if (position >= table->size) {
                return false;
            }
            syllable = 0xFFu + table->data[position++];
        }
        if (current_codepoint == codepoint) {
            return syllable_at(table, syllable, text, capacity);
        }
        if (current_codepoint > codepoint) {
            return false;
        }
    }
    return false;
}

char pinyin_table_initial(const PinyinTable *table, uint32_t codepoint)
{
    char syllable[SYLLABLE_BYTES];
    return pinyin_table_syllable(table, codepoint, syllable, sizeof syllable) ? syllable[0] : 0;
}

