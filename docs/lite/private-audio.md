# Private audio / third list — design note

**Status: design only, not implemented.** This is the private-list idea discussed for the
personal 512 MB Lite build. Encryption is explicitly out of scope for now.

## Goal

- Normal All Songs, Favorites, A-Z, shuffle, queues and resume keep working exactly as they do.
- The owner can copy recordings or other personal audio into one dedicated folder.
- That folder appears as a separate private list and is not mixed into All Songs or Favorites.
- The private list has its own playback queue and resume state.
- The private list is a session entered on purpose, not part of the normal root cycle. This is
  what keeps a hidden/listening state from leaking through Short Menu or Up Next.
- No boot-chain, IDB, PMU, fw2, partition or slot-layout changes.

"Private" here means **UI isolation from the normal library**, not cryptographic privacy. The
files and any private index are still readable when the card is connected to a computer.

## Why a folder or M3U alone is not enough

The main library indexes every supported audio file under `/`. Favorites is a set of path-hashed
keys into that library, and an M3U playlist resolves entries to library tracks. If private files
are indexed in the main library, they can appear in All Songs, artist/album/genre lists, shuffle
or an M3U. Excluding the folder from the main scan keeps them out, but then the normal M3U reader
cannot resolve them either.

The useful foundation is that both the device scanner and `tools/library_index.py` skip
dot-prefixed directories. A folder such as `/.private/` is therefore invisible to the normal
library without changing the normal data format.

## Recommended shape: separate private library

```text
/.private/                 private audio, managed from a computer
/.echo/private.db          private index (proposed)
/.echo/private_resume.bin  private queue/position (proposed)
```

The normal library stays at `/.echo/library.db`; the private index is a separate small library.
This avoids changing the main `library.db` format and avoids adding private tracks to any normal
index.

### Data and index

- The private index can use the existing library format and the same library reader. Its root
  is the private folder, so it needs either a scan root parameter or a small filesystem wrapper
  that presents `/.private` as `/` to the scanner.
- Titles use the tag title when present, otherwise the file stem. The private list is sorted by
  title and supports A-Z, like a normal list.
- The proposed cap is **2,048 tracks**. The library format's hard ceiling is 65,535 tracks
  (16-bit track indices, with `0xFFFF` reserved as `LIBRARY_NONE`), but 2,048 is enough for
  recordings and keeps the scan, index, A-Z and list UI comfortable. The cap can be raised later.
- Reserve about **2 MB** for `private.db`; 2,048 tracks with paths and titles fit comfortably.
  If the cap is raised toward the hard ceiling, the reservation must grow.
- Empty playlists, unsupported files, missing folders and a corrupt/private index are non-fatal:
  show an empty list or a toast; the normal library continues to work.

### Building the index

**Device-built is the target.** The existing scanner can be reused for the private folder without
duplicating the whole library scanner:

- Add a scan-root field to `ScanJob` (or a small filesystem wrapper). `library_scan` currently
  starts at `/`; a private run starts at `/.private`. The normal run stays `/`.
- Reuse the existing rescan overlay. `library_rescan_run` already has the scanner, builder,
  scratch memory and pinyin data; after the normal output it can run a second scan with the
  private root, a different sink (`/.echo/private.db`) and an empty folder-exclusion list. The
  scanner code is already loaded, so this mostly costs app-side orchestration, not a second
  overlay.
- Build on the normal Rescan and after USB writes, and build on first entry if the private index
  is missing. If the private folder is removed, delete/ignore the index and hide the entry.
- The private scan is small (target 2,048 tracks), so its time is bounded. Show the normal
  rescan/private progress state rather than starting it silently in the background.

The one open size question is the main rescan overlay, which has only about **688 bytes** free.
Adding a root field and the initial path change should be small, but this must be compiled and
measured before it is promised. The app-side second-scan orchestration has the normal 20.5 KB code
window to work with.

If the overlay change does not fit, the fallback is a PC-built private index: index `/.private`
to `/.echo/private.db` while connected, and let the device only read it. That is lower risk but
requires a computer step.

### Player, queue and resume

- Private playback uses the same audio output and the same volume/equalizer settings, but a
  separate private queue and resume file.
- The simplest implementation keeps one active `Player` and switches libraries:
  1. Entering Private: pause and save normal playback/resume, then initialise the player with the
     private library and load `private_resume.bin`.
  2. Leaving Private: save the private resume, then reopen the normal library and restore the
     normal resume.
  The normal queue/position must come back unchanged; the private queue must never leak into
  normal All Songs/Favorites.
- `player_save`/`player_restore` currently use the single `PLAYER_RESUME_PATH`. Private mode
  needs a resume-path parameter or a small private wrapper around those calls; the format itself
  can stay the same.
- The alternative, a second `Player` object kept alive, costs roughly another 2–3 KB of static
  RAM and needs an active-player pointer throughout the app. The save/switch approach is smaller
  and keeps the existing player code single-instance.
- Private tracks are never added to Favorites. The song-actions menu on a private row should
  offer Play Next/Add to Up Next only, or no song actions at all, so Favorites cannot receive a
  private path key.
- No listen history or scrobbling for private tracks (Lite has no history path, but the rule
  should be explicit if any history feature is added later).

### UI and keys

The private list is **not a third root** in the normal root cycle. A third root would be
reachable by Short Back, and if private playback continued when the user left it, Short Menu and
Up Next would show private titles. Instead it is a session entered from a deliberate entry:

- Entry: **Settings → System → Private audio**. The row is only drawn when `/.private/` exists.
- Opening it asks a confirmation question, for example `Open private audio?` with Play = yes and
  Back = no. This is the "second confirmation"; it is not a password.
- While the private session is open, the private list replaces the normal list; Short Menu opens
  the private Now Playing, and Short Menu there opens the private Up Next. That is expected while
  the session is deliberately open.
- Leaving the private list, leaving Now Playing for Settings, screen-off, power-off or a
  timeout **locks and stops the private session**. The normal player/queue is then restored, so
  Short Menu, Now Playing and Up Next cannot display a private track from a normal screen.
- There is no private auto-resume at boot. On the next start the normal roots appear first; the
  private session starts only through the Settings entry and confirmation.
- Inside the private list: Volume selects, hold scrolls; Play opens the recording; Long Menu is
  A-Z; Short Menu is Now Playing; Back leaves and locks. Long Play does nothing (no Favorites
  action).
- Private tracks are never added to Favorites, normal Up Next, normal history or normal M3U.
- The root meter stays two cells; the private session is a separate screen, not a root. The
  private list can have a small "PRIVATE" marker in its header instead.
- If the folder is removed or the index is empty, the Settings row disappears and the normal
  two-root model is unchanged.

This design is a **casual privacy curtain**, not security. It prevents the normal player from
accidentally showing a private title; it does not stop someone who knows the Settings entry and
opens it while the owner is not looking. A secret key sequence can replace the Settings entry
later, but it is still not encryption; see
[README.md](README.md#secret-entry-and-encryption).

## Capacity

- **Hard library limit:** 65,535 private tracks (16-bit indices; `LIBRARY_NONE` is `0xFFFF`).
- **Proposed private-list limit:** **2,048 tracks**.
- **Index reservation:** about 2 MB for 2,048 paths/titles; raise if the cap is raised.
- **Queue:** a private folder source query can play the whole 2,048-track list (the queue uses a
  browse source, not the 512-entry explicit Up Next list). If private tracks are copied into an
  explicit saved queue, the existing 512-entry Lite explicit-queue limit applies.
- The cap is independent of the 512-song Favorites capacity.

## RAM and code

The current Lite image has roughly 20.5 KB of code window and 95.5 KB of loaded RAM free at the
time of this note. A second `LibraryStore` plus its block cache is about 2.5–3 KB of static RAM;
a private list screen and the save/switch logic add more. The second scan mainly reuses the
already-loaded scanner, so the app-side code should fit, but the overlay root-field change must
still fit the 688-byte margin. The normal library and resident tables should not change.

## Test plan

- Main scan, All Songs, Favorites, A-Z, shuffle and resume are unchanged when `/.private/` exists.
- The Settings entry is present only when the folder/index exists; removing it restores the
  normal two-root UI.
- Normal → Private → Normal restores the normal queue, track and position; private resume is
  independent and is not loaded at boot.
- Private tracks never appear in Favorites, All Songs, artist/album/genre lists, shuffle or M3U.
- **Leak checks:** after leaving Private, Short Menu, Now Playing, Up Next and Settings cannot
  show a private title while normal playback is active; screen-off locks the session; a normal
  boot never opens Private.
- A-Z in Private, empty folder, unsupported file, missing folder, corrupt private index.
- Capacity: 2,048 tracks load; the 2,049th is rejected or the index is capped with a clear
  message.
- Device-built scan: normal rescan + private scan; USB copy + eject; private folder removed;
  private index missing; overlay size within its margin.
- Device images still pass updater and RAM/code checks.

## Open decisions

1. Confirm the Settings confirmation entry versus a visible third root. The session design is
   recommended because it closes the Short Menu/Up Next leak.
2. Private Up Next: reuse the normal Up Next screen and explicit queue, or omit it in v1.
3. Whether the private session should lock on screen-off only, or also on a short idle timeout.
4. Whether Long Play/Play Next from a private row is allowed (private Up Next only) or the private
   list is play-from-row only.
