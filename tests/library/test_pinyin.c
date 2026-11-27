#include <stdio.h>
#include <string.h>

#include "check.h"
#include "library/pinyin.h"

static unsigned char asset[32768];

static size_t load_asset(void)
{
    FILE *file = fopen(ECHO_PINYIN_ASSET, "rb");
    if (file == NULL) {
        return 0;
    }
    size_t length = fread(asset, 1, sizeof asset, file);
    fclose(file);
    return length;
}

static void asset_has_the_expected_counts(void)
{
    size_t length = load_asset();
    PinyinTable table;
    CHECK(length > 0);
    CHECK(pinyin_table_open(&table, asset, (uint32_t)length));
    CHECK_EQUAL(6940, table.count);
    CHECK_EQUAL(217, table.blocks);
    CHECK_EQUAL(428, table.syllable_count);
}

static void known_syllables_are_read(void)
{
    size_t length = load_asset();
    PinyinTable table;
    CHECK(pinyin_table_open(&table, asset, (uint32_t)length));
    char text[16];
    CHECK(pinyin_table_syllable(&table, 0x4F60, text, sizeof text));
    CHECK_EQUAL(0, strcmp("ni", text));
    CHECK(pinyin_table_syllable(&table, 0x5468, text, sizeof text));
    CHECK_EQUAL(0, strcmp("zhou", text));
    CHECK(pinyin_table_syllable(&table, 0x3042, text, sizeof text));
    CHECK_EQUAL(0, strcmp("a", text));
    CHECK(pinyin_table_syllable(&table, 0x4E2D, text, sizeof text));
    CHECK_EQUAL(0, strcmp("zhong", text));
    CHECK(pinyin_table_syllable(&table, 0x5973, text, sizeof text));
    CHECK_EQUAL(0, strcmp("nv", text));
    CHECK(pinyin_table_syllable(&table, 0x7EFF, text, sizeof text));
    CHECK_EQUAL(0, strcmp("lv", text));
    CHECK(!pinyin_table_syllable(&table, 0xFA19, text, sizeof text));
}

static void initials_are_lowercase(void)
{
    size_t length = load_asset();
    PinyinTable table;
    CHECK(pinyin_table_open(&table, asset, (uint32_t)length));
    CHECK_EQUAL('z', pinyin_table_initial(&table, 0x5468));
    CHECK_EQUAL('a', pinyin_table_initial(&table, 0x3042));
    CHECK_EQUAL(0, pinyin_table_initial(&table, 0xFA19));
}

static void malformed_tables_are_refused(void)
{
    PinyinTable table;
    CHECK(!pinyin_table_open(&table, (const uint8_t *)"EPY1", 4));
    CHECK(!pinyin_table_open(&table, NULL, 0));
    size_t length = load_asset();
    CHECK(length > 16);
    CHECK(!pinyin_table_open(&table, asset, (uint32_t)length - 8));
    static unsigned char corrupt[32768];
    memcpy(corrupt, asset, length);
    corrupt[length / 2] ^= 0x01;
    CHECK(!pinyin_table_open(&table, corrupt, (uint32_t)length));
    memcpy(corrupt, asset, length);
    corrupt[length - 1] ^= 0x01;
    CHECK(!pinyin_table_open(&table, corrupt, (uint32_t)length));
}

int main(void)
{
    static const TestCase cases[] = {
        TEST_CASE(asset_has_the_expected_counts),
        TEST_CASE(known_syllables_are_read),
        TEST_CASE(initials_are_lowercase),
        TEST_CASE(malformed_tables_are_refused),
    };
    return test_run_all(cases, TEST_CASE_COUNT(cases));
}
