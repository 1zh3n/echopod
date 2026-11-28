# Private audio / third list — design note

**Status: implemented in the working branch; image/device pass pending.** This is the private
third-root design for the personal 512 MB Lite build. Encryption is explicitly out of scope.

## Goal

- Normal All Songs, Favorites, A-Z, shuffle, queues and resume keep working exactly as they do.
- The owner can copy recordings or other personal audio into one dedicated folder.
- That folder appears as a separate third root (`All Songs → Favorites → Private → All Songs`) and
  is not mixed into All Songs or Favorites.
- The private list has its own playback queue and resume state.
- Leaving the Private root stops/restores the private player and reopens the normal player. There
  is no lock or confirmation; this is isolation from the normal library, not a privacy lock.
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

**Device-built is the target and is implemented in the current branch.** The existing scanner is
reused for the private folder without duplicating the whole library scanner:

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

The rescan overlay was the size limit. With the private scan root and the second output, the
current Lite release build still reports **688 bytes free** in the rescan overlay. The app-side
second-scan orchestration fits in the normal code window.

A PC-built private index remains a fallback if later changes need more overlay room: index
`/.private` to `/.echo/private.db` while connected, and let the device only read it. That is lower
risk but requires a computer step.

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

The private list is a **third root**, shown only when `/.private/` has an indexed track:

```text
All Songs → Favorites → Private → All Songs
```

- Short Back at a root cycles the three roots; with no private index it stays the old
  All Songs ↔ Favorites cycle.
- Entering Private switches the active player/library to the private index and restores its own
  queue and resume. Leaving Private saves the private state, stops/releases that player and
  reopens the normal library/player.
- Inside Private: Volume selects, hold scrolls; Play opens the recording; Long Menu is A-Z;
  Short Menu is Now Playing; Back leaves the root. Long Play does nothing (no Favorites action).
- While Private is the active root, Short Menu and Up Next show the private player/queue. That is
  expected; switching back to All Songs/Favorites stops that private playback and restores the
  normal player, so the normal roots cannot display a private title afterward.
- Private tracks are never added to Favorites, normal Up Next, normal history or normal M3U.
- The left root meter gains a third cell on Private and stays two cells on the normal roots.
- If the folder is removed or the index is empty, the Private root disappears on the next rescan
  and the two-root model returns.
- There is no lock, PIN or confirmation. The Private root is visible only after the private index
  has content. A secret key sequence can hide the entry later, but it is not encryption; see
  [README.md](README.md#secret-entry-and-encryption).

This is **UI isolation, not security**. Anyone who opens Private, plugs the card into a computer,
or reads the private index can see the content.

## Capacity

- **Hard library limit:** 65,535 private tracks (16-bit indices; `LIBRARY_NONE` is `0xFFFF`).
- **Current implementation limit:** **512 tracks**, because the private list uses the existing
  Lite `TrackListScreen`/explicit-queue capacity. The index may contain more, but only the first
  512 are listed and playable until this is moved to a BrowseScreen source query.
- **Index reservation:** 2 MB, enough for the current 512 plus the empty/library overhead.
- **Queue:** the private list uses the existing 512-entry explicit queue. A future BrowseScreen
  version could play the whole folder source without the explicit-queue cap.
- The cap is independent of the 512-song Favorites capacity.

## RAM and code

The working implementation measures about 138.8 KB of code (18.5 KB free), 349.4 KB loaded
(88.2 KB free), and 210.6 KB of `.bss`; the rescan overlay has 688 bytes free. A second
`LibraryStore` plus its block cache is about 2.5–3 KB of static RAM, and the private player
switch adds the private resume/up-next paths and state. The normal library and resident tables
are unchanged.

## Test plan

- Main scan, All Songs, Favorites, A-Z, shuffle and resume are unchanged when `/.private/` exists.
- The Private root is present only when the folder/index has tracks; removing it restores the
  normal two-root UI after a rescan.
- Normal → Private → Normal restores the normal queue, track and position; private resume is
  independent and is not loaded at boot.
- Private tracks never appear in Favorites, All Songs, artist/album/genre lists, shuffle or M3U.
- After leaving Private, normal Short Menu/Now Playing/Up Next cannot show a private title; a
  normal boot never opens Private. Inside Private they show the private queue, as expected.
- A-Z in Private, empty folder, unsupported file, missing folder, corrupt private index.
- Capacity: 512 tracks load; the 513th is not listed until the list moves to a BrowseScreen.
- Device-built scan: normal rescan + private scan; USB copy + eject; private folder removed;
  private index missing; overlay size within its margin.
- Device images still pass updater and RAM/code checks.

## Open decisions

1. Raise the private list from 512 to a larger cap by using a BrowseScreen source query.
2. Whether Long Play/Play Next from a private row is allowed (private Up Next only) or the private
   list is play-from-row only.
3. Whether the private root should be hidden behind an optional key sequence later; no lock or
   encryption is planned.
