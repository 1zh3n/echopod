#include "library/pinyin.h"

#include <stdbool.h>
#include <stdint.h>

#include "hal/system.h"

enum {
    HEADER_BYTES = 12,
    BLOCK_SHIFT = 5,
    BLOCK_SIZE = 32,
    BLOCK_BYTES = 256,
    TEXT_BYTES = 1800
};

static uint16_t blocks;
static uint16_t record_count;
static uint16_t syllable_count;
static uint32_t syllable_offset;
static bool loaded;
static bool failed;

static uint16_t get16(const uint8_t *bytes)
{
    return (uint16_t)(bytes[0] | (uint16_t)bytes[1] << 8);
}

static bool read_at(uint32_t offset, void *target, uint32_t length)
{
    return hal_system_pinyin_read(offset, target, length);
}

static bool load_meta(void)
{
    if (loaded) {
        return true;
    }
    if (failed) {
        return false;
    }
    uint32_t size = hal_system_pinyin_size();
    if (size < HEADER_BYTES) {
        failed = true;
        return false;
    }
    uint8_t header[HEADER_BYTES];
    if (!read_at(0, header, sizeof header) || header[0] != 'E' || header[1] != 'P' ||
        header[2] != 'Y' || header[3] != '1' || header[4] != 1 || header[5] != BLOCK_SHIFT) {
        failed = true;
        return false;
    }
    uint32_t count = get16(header + 6);
    blocks = (uint16_t)((count + BLOCK_SIZE - 1) / BLOCK_SIZE);
    record_count = (uint16_t)count;
    if (count == 0 || blocks == 0) {
        failed = true;
        return false;
    }
    uint32_t index_size = (uint32_t)blocks * 4u;
    uint32_t data_start = HEADER_BYTES + index_size;
    uint8_t entry[4];
    if (!read_at(HEADER_BYTES + (uint32_t)(blocks - 1) * 4u, entry, sizeof entry)) {
        failed = true;
        return false;
    }
    uint32_t first = get16(entry);
    uint32_t offset = get16(entry + 2);
    uint32_t read_length = size - data_start - offset;
    if (read_length > BLOCK_BYTES) {
        read_length = BLOCK_BYTES;
    }
    uint8_t block[BLOCK_BYTES];
    if (read_length == 0 || !read_at(data_start + offset, block, read_length)) {
        failed = true;
        return false;
    }
    uint32_t previous = 0;
    uint32_t position = 0;
    uint32_t in_block = count - (uint32_t)(blocks - 1) * BLOCK_SIZE;
    if (in_block > BLOCK_SIZE) {
        in_block = BLOCK_SIZE;
    }
    for (uint32_t index = 0; index < in_block; index++) {
        if (position >= read_length) {
            failed = true;
            return false;
        }
        uint32_t delta = block[position++];
        if (delta == 0xFF) {
            if (position + 2 > read_length) {
                failed = true;
                return false;
            }
            delta = get16(block + position);
            position += 2;
        }
        uint32_t codepoint = index == 0 ? first : previous + delta;
        previous = codepoint;
        if (position >= read_length) {
            failed = true;
            return false;
        }
        uint32_t syllable = block[position++];
        if (syllable == 0xFF) {
            if (position >= read_length) {
                failed = true;
                return false;
            }
            syllable = 0xFFu + block[position++];
        }
    }
    syllable_offset = data_start + offset + position;
    if (syllable_offset + 2 > size) {
        failed = true;
        return false;
    }
    uint8_t count_bytes[2];
    if (!read_at(syllable_offset, count_bytes, sizeof count_bytes)) {
        failed = true;
        return false;
    }
    syllable_count = get16(count_bytes);
    if (syllable_count == 0 || syllable_count > PINYIN_MAX_SYLLABLES) {
        failed = true;
        return false;
    }
    loaded = true;
    return true;
}

char pinyin_initial_from_image(uint32_t codepoint)
{
    load_meta();
    if (!loaded || codepoint > 0xFFFF) {
        return 0;
    }
    uint32_t size = hal_system_pinyin_size();
    uint32_t index_size = (uint32_t)blocks * 4u;
    uint32_t data_start = HEADER_BYTES + index_size;
    uint32_t low = 0;
    uint32_t high = blocks;
    uint8_t entry[4];
    while (low < high) {
        uint32_t middle = low + (high - low) / 2;
        if (!read_at(HEADER_BYTES + middle * 4u, entry, sizeof entry)) {
            return 0;
        }
        if (get16(entry) <= codepoint) {
            low = middle + 1;
        } else {
            high = middle;
        }
    }
    if (low == 0) {
        return 0;
    }
    uint32_t block = low - 1;
    if (!read_at(HEADER_BYTES + block * 4u, entry, sizeof entry)) {
        return 0;
    }
    uint32_t first = get16(entry);
    uint32_t offset = get16(entry + 2);
    uint32_t read_length = size - data_start - offset;
    if (read_length > BLOCK_BYTES) {
        read_length = BLOCK_BYTES;
    }
    uint8_t data[BLOCK_BYTES];
    if (read_length == 0 || !read_at(data_start + offset, data, read_length)) {
        return 0;
    }
    uint32_t previous = 0;
    uint32_t position = 0;
    uint32_t in_block = (uint32_t)record_count - block * BLOCK_SIZE;
    if (in_block > BLOCK_SIZE) {
        in_block = BLOCK_SIZE;
    }
    uint32_t syllable = 0;
    bool found = false;
    for (uint32_t index = 0; index < in_block; index++) {
        if (position >= read_length) {
            return 0;
        }
        uint32_t delta = data[position++];
        if (delta == 0xFF) {
            if (position + 2 > read_length) {
                return 0;
            }
            delta = get16(data + position);
            position += 2;
        }
        uint32_t current = index == 0 ? first : previous + delta;
        previous = current;
        if (position >= read_length) {
            return 0;
        }
        syllable = data[position++];
        if (syllable == 0xFF) {
            if (position >= read_length) {
                return 0;
            }
            syllable = 0xFFu + data[position++];
        }
        if (current == codepoint) {
            found = true;
            break;
        }
        if (current > codepoint) {
            return 0;
        }
    }
    if (!found || syllable >= syllable_count) {
        return 0;
    }
    uint32_t text_offset = syllable_offset + 2;
    uint32_t text_length = size - text_offset;
    if (text_length > TEXT_BYTES) {
        text_length = TEXT_BYTES;
    }
    uint8_t text[TEXT_BYTES];
    if (text_length == 0 || !read_at(text_offset, text, text_length)) {
        return 0;
    }
    uint32_t text_position = 0;
    for (uint32_t index = 0; index < syllable_count; index++) {
        if (text_position >= text_length) {
            return 0;
        }
        uint32_t length = text[text_position++];
        if (text_position + length > text_length) {
            return 0;
        }
        if (index == syllable) {
            return (char)text[text_position];
        }
        text_position += length;
    }
    return 0;
}
