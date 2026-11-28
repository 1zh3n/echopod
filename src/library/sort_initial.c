#include <stdbool.h>
#include <stdint.h>

#include "core/resident.h"
#include "core/utf8.h"
#ifdef ECHO_LITE
#include "library/pinyin.h"
#endif
#include "library/sort_key.h"

enum {
    LATIN_FIRST = 0xC0,
    LATIN_LAST = 0x17F,
    FULLWIDTH_FIRST = 0xFF01,
    FULLWIDTH_LAST = 0xFF5E,
    FULLWIDTH_OFFSET = 0xFEE0,
    OTHER_SCRIPT = 0x80,
    UNMAPPED = '~'
};

ECHO_RESIDENT static const char LATIN_INITIALS[] =
    "aaaaaa~ceeeeiiii~nooooo~~uuuuy~saaaaaa~ceeeeiiii~nooooo~~uuuuy~y"
    "aaaaaaccccccccdd~~eeeeeeeeeegggggggghh~~iiiiiiiii~iijjkk~lllllll"
    "l~~nnnnnn~~~oooooo~~rrrrrrsssssssstttt~~uuuuuuuuuuuuwwyyyzzzzzzs";

static bool is_space(uint32_t code_point)
{
    return code_point == ' ' || (code_point >= 0x09 && code_point <= 0x0D) ||
           (code_point >= 0x1C && code_point <= 0x1F) || code_point == 0x85 || code_point == 0xA0 ||
           code_point == 0x1680 || (code_point >= 0x2000 && code_point <= 0x200A) ||
           code_point == 0x2028 || code_point == 0x2029 || code_point == 0x202F ||
           code_point == 0x205F || code_point == 0x3000;
}

static uint32_t folded(uint32_t code_point)
{
    if (code_point >= FULLWIDTH_FIRST && code_point <= FULLWIDTH_LAST) {
        return code_point - FULLWIDTH_OFFSET;
    }
    switch (code_point) {
    case 0xFB00:
    case 0xFB01:
    case 0xFB02:
    case 0xFB03:
    case 0xFB04:
        return 'f';
    case 0xFB05:
    case 0xFB06:
        return 's';
    case 0x212A:
        return 'k';
    case 0x212B:
        return 'a';
    case 0x1E9E:
        return 's';
    default:
        break;
    }
    return code_point;
}

static uint32_t lowered(uint32_t code_point)
{
    return code_point >= 'A' && code_point <= 'Z' ? code_point + ('a' - 'A') : code_point;
}

static const char *skip_spaces(const char *text)
{
    const char *cursor = text;
    while (true) {
        const char *before = cursor;
        uint32_t code_point = utf8_next(&cursor);
        if (code_point == 0 || !is_space(code_point)) {
            return before;
        }
    }
}

char sort_key_initial(const char *text)
{
    const char *cursor = skip_spaces(text);
    uint32_t code_point = lowered(folded(utf8_next(&cursor)));
    if (code_point < OTHER_SCRIPT) {
        return (char)code_point;
    }
#ifdef ECHO_LITE
    char pinyin = pinyin_initial_from_image(code_point);
    if (pinyin != 0) {
        return pinyin;
    }
#endif
    if (code_point >= LATIN_FIRST && code_point <= LATIN_LAST) {
        char initial = LATIN_INITIALS[code_point - LATIN_FIRST];
        return initial == UNMAPPED ? (char)OTHER_SCRIPT : initial;
    }
    return (char)OTHER_SCRIPT;
}
