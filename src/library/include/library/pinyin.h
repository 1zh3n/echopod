#pragma once

#include <stdbool.h>
#include <stdint.h>

enum { PINYIN_MAX_SYLLABLES = 512 };

typedef struct PinyinTable {
    const uint8_t *data;
    uint32_t size;
    const uint8_t *index;
    const uint8_t *records;
    const uint8_t *syllables;
    uint16_t count;
    uint16_t blocks;
    uint16_t syllable_count;
    uint16_t syllable_offsets[PINYIN_MAX_SYLLABLES];
} PinyinTable;

bool pinyin_table_open(PinyinTable *table, const uint8_t *data, uint32_t size);
bool pinyin_table_syllable(const PinyinTable *table, uint32_t codepoint, char *text,
                           uint32_t capacity);
char pinyin_table_initial(const PinyinTable *table, uint32_t codepoint);
extern const PinyinTable *volatile pinyin_current;
void pinyin_state_keep(void);
char pinyin_initial_from_image(uint32_t codepoint);

static inline void pinyin_table_set(const PinyinTable *table)
{
    pinyin_current = table;
}

static inline const PinyinTable *pinyin_table_get(void)
{
    return pinyin_current;
}
