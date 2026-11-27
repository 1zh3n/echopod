# List section jump: duplicate letters and key feel

Status: **interaction approved: double Menu, not implemented.** 2026-11-28.

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

The double-Menu choice is A. B and C are kept as rejected alternatives in case the double
window proves unusable on the device. Do not keep section jump on the same hold as scrolling.

### Long Play is not a free gesture

Long Play was proposed for the A-Z overlay. It does not slow short Play: with
`TAP_OR_HOLD`, a short press is emitted on release and the long action only at the 600 ms
threshold. The problem is that Long Play already has a meaning in track lists:

- All Songs / playlist tracks / folder tracks: opens the song-actions menu
  (Play next, Add to Up Next);
- Up Next: currently unused, but it is a track list;
- Albums / Artists: usually unused in Lite.

Using Long Play for the jump would either remove song actions from All Songs or make the same
gesture mean different things in different lists. That is the interaction inconsistency this
design is trying to avoid. Long Play is therefore rejected unless the song-actions menu moves
to another gesture, which is a separate change.

Double Menu or Long Menu in lettered lists are the realistic alternatives:
- Double Menu keeps Long Play for song actions, at the cost of a double-press window;
- Long Menu keeps no delay and no Play conflict, at the cost of Settings being unreachable by
  Long Menu from those lists.

## Deferred

- A visible permanent A-Z strip.
- A section byte in `library.db` (the other way to make the section and sort order agree).

## Menu as the general key

Menu is the better general-purpose key: it already carries the two global playback shortcuts,
and it has no per-row action in the lists.

Proposed map for lettered lists:

| Gesture | Meaning |
|---|---|
| short Menu | Now Playing |
| long Menu | Settings |
| double Menu | open the A-Z/# jump overlay |
| Back | close the overlay |
| volume | move between letters while the overlay is open |
| Play | jump to the selected letter |

This is three gestures, but each gesture has one meaning. It does not change what a continuous
hold does, and it does not collide with the Play song-actions menu.

The cost is double-press detection: a single Menu in a lettered list has to wait for the double
window (about 220-280 ms) before opening Now Playing. If that delay is unacceptable, the fallback
is:

- long Menu = A-Z jump in lettered lists;
- Settings remains available by long Menu from Home/Now Playing, but not from those lists.

**Decision: double Menu**, limited to lettered lists with more than one section. Keep the
double window as short as the key tracker can reliably detect. The overlay opens only on the
double gesture, never as an automatic phase of a hold.

- Short Menu keeps going to Now Playing.
- Long Menu keeps going to Settings.
- Long Play keeps the song-actions menu where it exists.
- Volume hold keeps scrolling rows only; section jump is removed from the hold.
- The hold thresholds and the 4-row fast-scroll phase are no longer part of section jump.

## Settings and the Quick panel

The same "double Menu changes group" gesture can cover the screens with groups:

| Screen | Double Menu |
|---|---|
| Lettered list | open the A-Z/# jump overlay |
| Settings | go to the next category (or open a category chooser) |
| Quick panel | switch between the playback and sleep cards (optional) |

Settings currently switches category with **long Play**. The proposal is to remove that
long-Play function and use double Menu instead:

- short Menu: Now Playing;
- long Menu: Settings (no action when already in Settings);
- double Menu: next Settings category;
- long Play in Settings: no action;
- short Play still changes the selected value/opens the subpage;
- volume still moves rows.

Why double Menu is preferred over long Menu for the category switch: it keeps long Menu as the
single global "go to Settings" gesture. Long Menu would otherwise mean "open Settings" outside
Settings and "next category" inside it. Double Menu is already the group-navigation gesture in
lettered lists, so the same concept applies in Settings. The cost is the double-press window
before short Menu opens Now Playing; accepted.

The Quick panel has only two cards and volume already selects between them. Double Menu there
would duplicate volume. Recommendation: **do not add double Menu to the Quick panel** unless
symmetry turns out to matter on the device. Long Menu there still opens Settings; long Play
stays no-op.

A category chooser overlay (double Menu opens the five categories, volume selects, Play
switches) is a larger alternative. It is more explicit but not needed for five categories;
cycling one category per double press is the smaller change.

## Double Menu in Now Playing: Quick Panel

Proposal: while in Now Playing, double Menu opens the Quick Panel (playback mode / sleep
timer). The only short-Menu action that waits for the double window is the Now Playing one:
Up Next by default, or **next track** when Settings → Buttons → Menu key is Next.

Two costs:

1. If Menu is configured as Next, every short Menu skip is delayed by the double window.
2. Long Play already opens the Quick Panel. Keeping both gestures is redundant, and the user
   pays the Up Next delay for no new capability. The realistic choices are:
   - **A. Double Menu replaces Long Play** for the Quick Panel; Long Play in Now Playing becomes
     a no-op or gets another role later.
   - **B. Keep Long Play only**; do not add double Menu in Now Playing.
   - **C. Keep both** as aliases; not recommended.

If double Menu is meant to be the general "go to the group" gesture, A is coherent:
Now Playing -> Quick Panel, lettered list -> A-Z overlay, Settings -> next category. B is the
smaller change and preserves the direct Long Play gesture. C adds delay without adding access.
