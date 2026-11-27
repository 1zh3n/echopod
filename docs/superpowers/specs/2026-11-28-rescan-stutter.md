# Rescan stutter

Status: **observed, not diagnosed; deferred.** Recorded 2026-11-28.

## Observation

While rebuilding the song library, the player feels stuck for a while. The question is
whether that is the expected synchronous rescan or something else. This note records the
current behavior and what to measure before changing it. It is not part of the motion-system
work.

## Current path

- Settings -> Library -> Rescan library sets `app.rescan_requested`.
- `app_step()` sees the flag, clears it, calls `run_rescan()`, and returns. The main loop does
  not run again until the scan and cover pass finish.
- `run_rescan()` shows the rescan notice and calls `library_rescan_run()`.
- `library_rescan_run()` loads the rescan overlay, borrows the 384 KB core1 scratch, copies the
  pinyin asset into it, and runs the library scan followed by the cover rebuild.
- Progress callbacks (`show_rescan_progress`, `show_cover_progress`) call `app_show_notice()` at
  most every `RESCAN_PROGRESS_MS` (250 ms). `hal_system_keep_alive()` services the display and
  the backlight while the scan runs, but not normal key handling.
- `hal_system_borrow_scratch()` shuts the audio output and core1 down for the duration. At the
  end, `hal_input_drop_presses()` discards keys pressed during the scan.

The cover pass dominates. `docs/known-issues.md` records about 1 minute 45 seconds for
1,563 tracks and 144 albums: 20 seconds for tracks, about 1.4 minutes for covers, at about half
a second per album picture. Later rescans keep unchanged covers and only decode new or changed
art, but the first scan after an update can decode them all.

So a frozen list and frozen input during a rebuild is expected current behavior. It is not the
motion system and should not be fixed by adding transitions.

## Still to identify

The report says "卡顿一会", which could mean one of these:

1. the expected blocking rescan while the notice counts tracks or covers;
2. the short pause after a rescan while `app_start()` reloads the app and reopens the library;
3. a stutter during normal browsing after the rescan, which would be a different bug.

The first is documented. The second needs a measurement. The third is not explained by the
current design and needs a reproduction.

## Evidence to collect

- `/.echo/rescan_times.bin` from the card. It stores two passes (library, covers) and, for each,
  list / open / read / write / total time and count as `uint32` values.
- Track and album counts, and whether this was a first scan or a rescan with unchanged covers.
- Card type and whether the scan ran after a USB session or from Settings.
- Wall-clock duration, whether the screen updated during the scan, whether audio stopped, and
  whether the device responded to keys.
- Whether the stutter happened on the 512 MB unit only, and whether the card was exFAT or FAT32.

## Later options, not decided

- Keep the scan synchronous but improve feedback: a clearer progress screen and a visible
  "do not unplug" state. This is the smallest change and does not alter the scan.
- Split the scan and cover pass into chunks across app steps, servicing input and display
  between chunks. This is the real fix for the freeze, but the scan state must survive in the
  borrowed scratch and re-entry must be made safe; it is a larger change.
- Move the rebuild to an explicit maintenance screen and block input there on purpose, so the
  pause is understandable instead of looking like a hang.
- Keep the existing cover reuse. Do not rebuild covers when the library and art stamps are
  unchanged.

Do not mix this with the motion work. Treat it as a separate performance task after the
navigation and motion changes are in place.
