#include "check.h"
#include "ui/list_navigation.h"
#include "ui_test_canvas.h"
#include "ui_test_theme.h"

enum {
    ITEM_COUNT = 100,
    ROWS_PER_SECTION = 10,
    REPEAT_MS = 120
};

static int item_count(const void *context)
{
    (void)context;
    return ITEM_COUNT;
}

static const char *item_label(const void *context, int index)
{
    (void)context;
    (void)index;
    return "A";
}

static int item_section(const void *context, int index)
{
    (void)context;
    return index / ROWS_PER_SECTION;
}

static ListView view;
static uint32_t now_ms;
static int section_lookups;

static int counted_section(const void *context, int index)
{
    section_lookups++;
    return item_section(context, index);
}

static const ListModel SECTIONED = {
    .context = NULL, .count = item_count, .label = item_label, .section = item_section};
static const ListModel COUNTED = {
    .context = NULL, .count = item_count, .label = item_label, .section = counted_section};
static const ListModel PLAIN = {.context = NULL, .count = item_count, .label = item_label};

static void start(const ListModel *model, int selected)
{
    list_view_init(&view, model, &TEST_THEME, 0);
    list_view_select(&view, selected, 0);
    now_ms = 1000;
}

static void send(Key key, KeyGesture gesture)
{
    ListSelection selection = {.select = NULL, .hold = NULL, .context = NULL};
    list_navigation_handle(&view, (KeyEvent){key, gesture}, now_ms, selection);
}

static void press_down(void)
{
    send(KEY_VOLUME_UP, KEY_GESTURE_PRESS);
}

static void repeat_down_at(uint32_t held_ms)
{
    now_ms = 1000 + held_ms;
    send(KEY_VOLUME_UP, KEY_GESTURE_REPEAT);
}

static void holding_moves_one_row_at_a_time_at_first(void)
{
    start(&SECTIONED, 0);
    press_down();
    repeat_down_at(400);
    repeat_down_at(400 + REPEAT_MS);
    CHECK_EQUAL(3, view.selected);
}

static void holding_longer_moves_several_rows_a_step(void)
{
    start(&PLAIN, 0);
    press_down();
    repeat_down_at(LIST_FAST_SCROLL_AFTER_MS);
    CHECK_EQUAL(1 + LIST_FAST_SCROLL_ROWS, view.selected);
}

static void a_long_hold_keeps_accelerating_rows(void)
{
    start(&SECTIONED, 3);
    press_down();
    repeat_down_at(LIST_FAST_SCROLL_AFTER_MS);
    CHECK_EQUAL(4 + LIST_FAST_SCROLL_ROWS, view.selected);
    repeat_down_at(LIST_FASTEST_SCROLL_AFTER_MS);
    CHECK_EQUAL(4 + LIST_FAST_SCROLL_ROWS + LIST_FASTEST_SCROLL_ROWS, view.selected);
}

static void a_list_without_sections_keeps_speeding_up(void)
{
    start(&PLAIN, 0);
    press_down();
    repeat_down_at(LIST_FASTEST_SCROLL_AFTER_MS);
    CHECK_EQUAL(1 + LIST_FASTEST_SCROLL_ROWS, view.selected);
}

static void fast_scrolling_stops_at_the_end(void)
{
    start(&SECTIONED, 95);
    press_down();
    repeat_down_at(LIST_FASTEST_SCROLL_AFTER_MS);
    CHECK_EQUAL(ITEM_COUNT - 1, view.selected);
    repeat_down_at(LIST_FASTEST_SCROLL_AFTER_MS + REPEAT_MS);
    CHECK_EQUAL(ITEM_COUNT - 1, view.selected);
}

static void a_new_press_starts_slow_again(void)
{
    start(&PLAIN, 0);
    press_down();
    repeat_down_at(LIST_FAST_SCROLL_AFTER_MS);
    int after_fast = view.selected;
    now_ms += 500;
    press_down();
    repeat_down_at(now_ms - 1000 + 400);
    CHECK_EQUAL(after_fast + 2, view.selected);
}

static void drawing_a_list_never_looks_up_sections(void)
{
    start(&COUNTED, 0);
    press_down();
    repeat_down_at(LIST_FAST_SCROLL_AFTER_MS);
    section_lookups = 0;
    Canvas canvas = ui_test_canvas();
    list_view_draw(&view, &canvas, theme_content_area(&TEST_THEME), now_ms);
    CHECK_EQUAL(0, section_lookups);
}

static void jumping_selects_the_first_row_of_a_section(void)
{
    start(&SECTIONED, 0);
    list_view_jump_to_section(&view, 3, 1000);
    CHECK_EQUAL(30, view.selected);
    list_view_jump_to_section(&view, 0, 1001);
    CHECK_EQUAL(0, view.selected);
    list_view_jump_to_section(&view, 27, 1002);
    CHECK_EQUAL(ITEM_COUNT - 1, view.selected);
}

static void sections_vary_only_when_the_list_has_more_than_one(void)
{
    start(&SECTIONED, 0);
    CHECK(list_view_sections_vary(&view));
    start(&PLAIN, 0);
    CHECK(!list_view_sections_vary(&view));
}

static void section_labels_are_letters_or_a_hash(void)
{
    CHECK_EQUAL('#', list_section_label(LIST_SECTION_BEFORE_LETTERS)[0]);
    CHECK_EQUAL('A', list_section_label(LIST_SECTION_FIRST_LETTER)[0]);
    CHECK_EQUAL('Z', list_section_label(LIST_SECTION_FIRST_LETTER + 25)[0]);
    CHECK_EQUAL('#', list_section_label(LIST_SECTION_AFTER_LETTERS)[0]);
}

static void sections_come_from_the_first_letter_of_a_sort_key(void)
{
    CHECK_EQUAL(LIST_SECTION_BEFORE_LETTERS, list_section_of_key("1999"));
    CHECK_EQUAL(LIST_SECTION_FIRST_LETTER, list_section_of_key("abba"));
    CHECK_EQUAL(LIST_SECTION_FIRST_LETTER + 25, list_section_of_key("zz top"));
    CHECK_EQUAL(LIST_SECTION_AFTER_LETTERS, list_section_of_key("\xE3\x81\x82"));
    CHECK_EQUAL(LIST_SECTION_BEFORE_LETTERS, list_section_of_key(""));
}

int main(void)
{
    static const TestCase cases[] = {
        TEST_CASE(holding_moves_one_row_at_a_time_at_first),
        TEST_CASE(holding_longer_moves_several_rows_a_step),
        TEST_CASE(a_long_hold_keeps_accelerating_rows),
        TEST_CASE(a_list_without_sections_keeps_speeding_up),
        TEST_CASE(fast_scrolling_stops_at_the_end),
        TEST_CASE(a_new_press_starts_slow_again),
        TEST_CASE(drawing_a_list_never_looks_up_sections),
        TEST_CASE(jumping_selects_the_first_row_of_a_section),
        TEST_CASE(sections_vary_only_when_the_list_has_more_than_one),
        TEST_CASE(section_labels_are_letters_or_a_hash),
        TEST_CASE(sections_come_from_the_first_letter_of_a_sort_key),
    };
    return test_run_all(cases, TEST_CASE_COUNT(cases));
}
