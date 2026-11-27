#include "library/pinyin.h"

const PinyinTable *volatile pinyin_current __attribute__((used));

void pinyin_state_keep(void)
{
    (void)pinyin_current;
}
