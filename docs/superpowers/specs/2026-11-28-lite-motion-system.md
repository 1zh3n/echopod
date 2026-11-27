# Lite motion system

Status: **layered direction and first choices approved, not implemented.** Recorded 2026-11-28.

## Decision

Use layered motion, not one animation for everything:

| Layer | Examples | Motion |
|---|---|---|
| Page | Home -> list, list -> child, Back | horizontal slide |
| Overlay | Now Playing, Settings, Quick Panel | short overlay transition, distinct from a page |
| In-place | list selection, Quick Panel card, Settings category | immediate, or a short micro-slide |
| Ambient | marquee, Now Playing faces, charging, status | independent loop |

First choices, recorded 2026-11-28:

- **Overlay: vertical sheet.** Now Playing, Settings and Quick Panel use the short sheet
  transition, not a fade and not the page slide.
- **No Off value.** The `Animations` setting stays Normal / Slower / Faster; all UI transitions
  obey that scale, while ambient loops stay independent.
- **Home: 240 ms first.** Use the same page duration and easing as the stack slide; 280 ms is
  the device-check fallback if the three-card carousel feels rushed.
- **Quick Panel: no group motion.** Selection stays immediate.

The point is a consistent **grammar**, not the same duration everywhere. List selection should
stay immediate; adding motion there would only make the player feel slower.

## Current audit

This is what the code does before the change:

| Change | Current behavior | Timing |
|---|---|---|
| Screen-stack push/pop | whole-screen horizontal slide; direction from stack depth | 240 ms normal, cubic ease-out |
| Home carousel | card position and size ease; selected shadow switches on the key press | 360 ms smoothstep, fixed |
| List selection/scroll | row jumps immediately; no row motion | none |
| Settings category | rows, icon and dots replace immediately | none |
| Quick Panel card | selected border replaces immediately | none |
| Toast | appears and disappears immediately | 1500 ms visible |
| Section box | appears and disappears immediately | 600 ms visible |
| Boot | Lite boot image holds, then cuts to the UI | 300 ms fade + 900 ms hold, fade unused on Lite |
| Charging / Now Playing faces | independent frame loops | 100 ms per frame / 100 ms tick |
| Cover Flow (non-Lite) | exponential approach | 70 ms time constant |

The `Animations` setting is **Normal / Slower / Faster**, mapped to 100 / 150 / 60 percent. It
is only used by the screen slide and Cover Flow. Home, Settings category, Toast and the section
box ignore it. That is why the setting feels inconsistent.

Other current details:

- Settings category is changed by the menu's hold action, currently long Play. The layered
  navigation decision moves this to double Menu; that is not implemented yet.
- `settings_show_category()` calls `list_view_select(..., 0)`, so the marquee time base is reset
  to zero instead of the real `now_ms`. A long label can start mid-scroll after a category
  change.
- Now Playing, Settings and Quick Panel are still ordinary stack screens, so they currently use
  the same horizontal slide as a browse page.
- The 1.5 s / 3 s volume-hold phases and the 450 ms section jump are still in the current code.
  The section-jump decision removes them from the hold; that is a separate, approved change.

## Proposed motion rules

### Page

- Use one horizontal slide for real stack changes: Home -> list, list -> child, Back, submenu.
- Keep the existing 240 ms normal duration and cubic ease-out for the first pass; scale it with
  the `Animations` setting (144 / 240 / 360 ms).
- Home should use the same duration, curve and setting. Change its fixed 360 ms smoothstep to
  the shared page motion. If 240 ms feels rushed for the three-card carousel on the device,
  280 ms is the fallback; do not keep a separate unexplained 360 ms.

### Overlay

- Give Now Playing, Settings and Quick Panel a transition that reads as a temporary layer, not
  another page.
- Use a short vertical sheet, about 160-180 ms, with the overlay moving up 12-20 px while its
  content is revealed. The Lite card and shadow style can stay; no dimming scrim is needed in
  the first pass.
- Do not use the page slide for these screens. A shorter horizontal slide is no longer the
  chosen fallback; the sheet is the decision.
- This should land with the layered navigation model, because that is when these screens stop
  being ordinary history entries.

### In-place

- List row selection and scrolling: keep immediate. Do not animate between rows.
- Quick Panel card selection: keep immediate; the border is the feedback.
- Settings category: give the content a short 120-140 ms horizontal micro-slide, 16-24 px of
  offset, with the old and new category drawn in the same pass. Do not use a full-screen page
  slide.
- Restart the marquee with the real `now_ms` when the category changes. If the micro-slide is
  deferred, at least fix the time base.
- Toast and the section box can stay immediate. A short fade is optional, not part of the first
  pass.

### Ambient

- Marquee, Now Playing faces, charging animation and the status meter keep their own timing.
- They are not scaled by the `Animations` setting. Scaling a marquee or a face loop would change
  information density, not transition feel.

## Memory and performance

Layered motion does not need another framebuffer:

- The display path renders one strip at a time (`strip_pixels`, 21,760 bytes); the current slide
  already draws the leaving and arriving screens into the same strip. A page slide and a group
  micro-slide reuse that path.
- A vertical overlay sheet needs a vertical canvas offset, not a new buffer. It can be a small
  addition next to `canvas_shifted()`.
- A Settings micro-slide needs the previous category's rows for 120-140 ms. At most
  `SETTINGS_GROUP_ROWS_MAX` (10) `MenuItem` entries, or an equivalent second draw; that is
  hundreds of bytes of RAM, not kilobytes. It must not become a full-screen snapshot.
- Avoid a full-screen crossfade. The canvas has no global alpha for a whole screen; blending two
  full screens would be more expensive and is not needed for the layered grammar.
- Transitions only run for their short duration, and `hal_audio_service()` runs per strip as it
  does now. This is not expected to affect playback. Rescan is a separate blocking path and does
  not run at the same time as a transition.

Current release-lite budget (baseline `MINIV934`):

```text
code window       135,284 / 157,292   22,008 free
loaded total      404,372 / 437,600   33,228 free
resident tables    18,248 /  18,432      184 free
rescan overlay     85,400 /  86,016      616 free
```

- Motion code will normally land in the code window, which has 22 KB free; that is enough for
  this work, but it still has to be measured.
- Do not add resident tables: 184 bytes is not a budget for a new table.
- Do not put motion code in the rescan overlay. Its 616 bytes are reserved for scan fixes; the
  motion work should not compete with them.
- Loaded RAM has 33 KB free; the small Settings row copy fits, but the build must be checked
  after implementation.

## Implementation notes for later

- Reuse `src/ui/slide.c` and `src/app/app.c`'s `start_slide()` for the page layer.
- Move the duration and easing choice behind one motion helper, so Home, page and group motion
  cannot drift apart again.
- Add a vertical offset helper (or extend the shifted canvas) for the overlay sheet.
- `settings_show_category()` needs the real `now_ms` for `list_view_select()` and the marquee
  restart.
- Tests to add with the implementation:
  - page duration follows the `Animations` setting;
  - Home uses the same duration and easing;
  - list selection has no delayed animation;
  - Settings category uses the real time base and shows the micro-slide without a full-screen
    page move;
  - overlay opens/closes in the overlay direction and page transitions do not;
  - landscape and portrait both keep the status meter and card geometry intact.

## Still open

- Home at 240 ms or 280 ms: 240 ms is the first build; move to 280 ms only if the device check
  says the carousel is too rushed.
- The exact sheet travel (12-20 px) and whether the sheet edges need a small shadow; check on
  the device.
- The animation setting name and documentation: it now scales every UI transition, so the
  existing "screen slides and Cover Flow" wording is incomplete.
