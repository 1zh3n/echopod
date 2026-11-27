# List section jump: duplicate letters and key feel

Status: **assessment recorded, not implemented.** 2026-11-28.

## Current behaviour

In a lettered list (All Songs, Artists, Albums, Album Artists, Genres, Playlists):

- holding a volume key repeats row movement every 120 ms;
- after 1.5 s the repeat moves 4 rows at a time (fast scroll);
- after 3.0 s each repeat jumps to the next section, about every 450 ms;
- a section box appears while the list is in the section phase.

So one long hold has two phases: fast scrolling first, section jumping second. That is the
"混乱" feeling: the user cannot tell whether the hold is scrolling or selecting a letter.

## Duplicate letters

The jump itself uses a binary search over the row sections and assumes the section numbers are
non-decreasing. The section label is computed independently from the sort key:

- `sort_key_make` folds the text through the sort table (NFKD, compatibility characters,
  fullwidth forms, pinyin/kana);
- `sort_key_initial` has its own mapping (ASCII, Latin-1/Extended-A, pinyin/kana, otherwise
  the trailing `#`).

They do not always agree. A concrete case:

```text
Ｄ (U+FF24, fullwidth D)  sort key = "d"  section = 0x80 ('#')
D                         sort key = "d"  section = 'd'
Da                        sort key = "da" section = 'd'
```

If the fullwidth title falls between two normal D titles, the row sections are:

```text
D, #, D
```

The section jump then shows D, #, D; after the # it arrives at D again. That looks like "D
twice". It is a section-label/sort-key mismatch, not the pinyin order itself. Mixed
English/Chinese text makes it more visible, but fullwidth Latin can trigger it without any CJK.

## To confirm

The exact titles that show the duplicate D are needed, or the device library. The likely
culprit is a fullwidth/compatibility character whose sort key starts with a Latin letter but
whose section initial is `#`. Other possible edge cases are compatibility ideographs and
legacy Latin forms.

## Proposed fixes

### 1. Make the section agree with the sort key

One source of truth: the section should be the first effective character of the same folded
key used for ordering. For the UI path this can be a small `sort_section` helper that:

- maps fullwidth/halfwidth ASCII and the same compatibility forms as the sort table;
- uses pinyin/kana initials for CJK;
- returns the leading/trailing `#` section only when the key really starts there.

Add a test with `D`, `Ｄ`, and `Da`: the sections must be `D`, `D`, `D`, never `D, #, D`.

### 2. One gesture must have one meaning

"Remove the fast-scroll phase and hold straight into section jump" is **not** a fix: the
hold would still repeat rows and then change meaning mid-gesture. That is the same problem.
A key may have different meanings on different screens; a single continuous gesture may not.

Options for the next pass:

- **A. Keep hold for row scrolling; add a separate A-Z overlay.** Double Menu in a lettered
  list opens the overlay; volume selects the letter, Play jumps, Back closes. The hold stays
  row repeat only. Cost: double-press detection adds a short delay to Menu in lists.
- **B. Make hold section jump only; taps/repeats move rows.** No new key, no mid-gesture
  change, but holding no longer auto-scrolls.
- **C. Use long Menu in lettered lists for the A-Z overlay.** Volume hold stays scrolling.
  Cost: Settings is no longer reachable by long Menu from those lists.

Recommendation: **A** if the double-Menu delay is acceptable; otherwise **B** is the
cleanest no-new-key option. Do not keep section jump on the same hold as scrolling.

## Deferred

- A visible permanent A-Z strip.
- A section byte in `library.db` (the other way to make the section and sort order agree).
