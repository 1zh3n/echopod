#include "section_jump_screen.h"

#ifdef ECHO_LITE

#include <stddef.h>

#include "ui/label.h"
#include "ui/lite_frame.h"

enum {
    GRID_SECTIONS = LIST_SECTION_AFTER_LETTERS + 1,
    LANDSCAPE_COLUMNS = 7,
    LANDSCAPE_ROWS = 4,
    PORTRAIT_COLUMNS = 4,
    PORTRAIT_ROWS = 7,
    CARD_INSET = 8,
    CELL_GAP = 1
};

static void select_current(SectionJumpScreen *jump)
{
    if (jump->list == NULL) {
        jump->selected = 0;
        return;
    }
    int section = list_view_section_of(jump->list, jump->list->selected);
    if (section < 0 || section >= GRID_SECTIONS) {
        section = 0;
    }
    jump->selected = (uint8_t)section;
}

static void draw_cell(Canvas *canvas, const Theme *theme, Rect cell, int section, bool selected)
{
    if (selected) {
        canvas_fill_rect(canvas, cell, theme->colors->selection_background);
    }
    TextStyle style = {.font = theme->fonts.title,
                       .color = selected ? theme->colors->selection_text : theme->colors->text};
    const char *label = list_section_label(section);
    Rect area = {(cell.x + 1), cell.y, cell.width - 2, cell.height};
    label_draw(canvas, style, (Label){.area = label_centered_area(style.font, area, label),
                                      .text = label});
}

static void draw(void *context, Canvas *canvas)
{
    SectionJumpScreen *jump = context;
    const Theme *theme = jump->theme;
    bool portrait = theme_is_portrait(theme);
    int width = theme->metrics.screen_width;
    int height = theme->metrics.screen_height;
    canvas_fill_rect(canvas, (Rect){0, 0, width, height}, theme->colors->background);

    Rect card = portrait ? (Rect){8, 48, 154, 224} : (Rect){8, 26, 304, 136};
    lite_card_rounded(canvas, theme, card, 5, 6);

    int columns = portrait ? PORTRAIT_COLUMNS : LANDSCAPE_COLUMNS;
    int rows = portrait ? PORTRAIT_ROWS : LANDSCAPE_ROWS;
    int cell_width = (card.width - 2 * CARD_INSET) / columns;
    int cell_height = (card.height - 2 * CARD_INSET) / rows;
    for (int index = 0; index < GRID_SECTIONS; index++) {
        int column = index % columns;
        int row = index / columns;
        Rect cell = {card.x + CARD_INSET + column * cell_width,
                     card.y + CARD_INSET + row * cell_height, cell_width - CELL_GAP,
                     cell_height - CELL_GAP};
        draw_cell(canvas, theme, cell, index, index == jump->selected);
    }
}

static void move_selection(SectionJumpScreen *jump, int delta)
{
    jump->selected = (uint8_t)((jump->selected + delta + GRID_SECTIONS) % GRID_SECTIONS);
}

static ScreenAction handle_key(void *context, KeyEvent event, uint32_t now_ms)
{
    SectionJumpScreen *jump = context;
    jump->now_ms = now_ms;
    switch (event.key) {
    case KEY_VOLUME_UP:
        if (event.gesture == KEY_GESTURE_PRESS || event.gesture == KEY_GESTURE_REPEAT) {
            move_selection(jump, 1);
        }
        return screen_action_none();
    case KEY_VOLUME_DOWN:
        if (event.gesture == KEY_GESTURE_PRESS || event.gesture == KEY_GESTURE_REPEAT) {
            move_selection(jump, -1);
        }
        return screen_action_none();
    case KEY_PLAY:
        if (event.gesture == KEY_GESTURE_PRESS && jump->list != NULL) {
            list_view_jump_to_section(jump->list, jump->selected, now_ms);
            return screen_action_pop();
        }
        return screen_action_none();
    case KEY_BACK:
        return event.gesture == KEY_GESTURE_LONG_PRESS ? screen_action_pop_to_root()
                                                       : screen_action_pop();
    case KEY_MENU:
        return screen_action_pop();
    case KEY_POWER:
        return screen_action_none();
    }
    return screen_action_none();
}

static void enter(void *context, uint32_t now_ms)
{
    SectionJumpScreen *jump = context;
    jump->now_ms = now_ms;
    select_current(jump);
}

static void update(void *context, uint32_t now_ms)
{
    SectionJumpScreen *jump = context;
    jump->now_ms = now_ms;
}

Screen *section_jump_screen_init(SectionJumpScreen *jump, SectionJumpConfig config)
{
    *jump = (SectionJumpScreen){.list = config.list, .theme = config.theme};
    jump->screen = (Screen){
        .title = "A-Z",
        .context = jump,
        .full_screen = true,
        .paints_every_pixel = true,
        .select_on_press = true,
        .overlay = true,
        .enter = enter,
        .handle_key = handle_key,
        .update = update,
        .draw = draw,
    };
    return &jump->screen;
}

#else
typedef int SectionJumpTranslationUnit;

#endif
