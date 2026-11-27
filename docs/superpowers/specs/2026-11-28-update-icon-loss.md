# Icons after a firmware update

Status: **observed, likely the pending-restart glyph refusal; needs confirmation.**
Recorded 2026-11-28.

## Observation

After flashing a new system, icons are missing. The user calls it a small problem, and expects
firmware updates to be rare once the UI is stable.

The Lite release build has `ECHO_NO_COVERS=ON`, so this is not album art. The likely icons are
the Home carousel, Settings categories, Quick Panel or status icons.

## Likely mechanism

Lite does not keep the icon masks in the code window. `LITE_ICON_POINTER` addresses a
16,810-byte face in the glyph table, and `hal_system_lite_icons_read()` loads it on demand into
four 2 KB cache slots.

`board/glyphs.c`'s `read_entry()` refuses every glyph-face read while
`hal_system_restart_pending()` is set. After the updater has written a slot, the old firmware
keeps running until the next power cycle; during that time only icons already in the cache can
still draw, and uncached icons disappear. The update notice already says the player must be
restarted.

If the icons return after the power cycle, this is the expected update state, not a permanent
loss. If they are still missing after a full power cycle, it is a different problem.

## What to confirm

- Are the icons missing only between "Update installed" and the power cycle, or after it too?
- Which icons: Home cards, Settings categories, Quick Panel, or the status meter?
- Does it happen after every update, or only when the glyph table changes (for example when the
  pinyin or icon face is added)?
- A photo of the screen, and the version before and after the update.

If it happens after the restart, check:

- whether the image contains the Lite icon face and the `LITE_ICONS_ENTRY` table entry;
- whether the face size matches `LITE_ICON_POINTER_LIMIT` (16,810 bytes);
- whether the update wrote the full image (code and glyph faces) or only the code region;
- whether the player is running the slot that was just written.

## Avoidance

- If this is only the pending-restart state, the smallest fix is to keep the update notice on
  screen until the player is power-cycled, or to make it clear that the icons are unavailable
  until then. The icons return by themselves after the restart.
- Preloading all icons before an update is not attractive: the face is 16,810 bytes and the
  cache is 4 x 2,048 bytes. It would compete with the code/RAM budget for a rare event.
- If the icons remain missing after a restart, fix the image/table rather than adding a second
  icon table to daily RAM.

## Not the cause

Album covers are not involved: the Lite build is compiled with `ECHO_NO_COVERS=ON`. The 512 MB
unit's storage size is also unrelated; this is the glyph-face read path.
