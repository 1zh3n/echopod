# Private audio / third list — design note

**Status: design only, not implemented.** This is the private-list idea discussed for the
personal 512 MB Lite build. Encryption is explicitly out of scope for now.

## Goal

- Normal All Songs, Favorites, A-Z, shuffle, queues and resume keep working exactly as they do.
- The owner can copy recordings or other personal audio into one dedicated folder.
- That folder appears as a separate third list and is not mixed into All Songs or Favorites.
- The private list has its own playback queue and resume state.
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

There are two possible build paths, and this is the main open decision:

1. **PC-built private index (lowest risk).** Extend `tools/library_index.py` with a private mode,
   or add a small tool that indexes `/.private` to `/.echo/private.db` while the player is
   connected. The device only reads the index. This avoids new on-device scanning code and does
   not touch the rescan overlay. The user copies files, runs the small tool/sync once, ejects, and
   the third list is ready.
2. **Device-built private index.** A dedicated small scan on the player. The current main rescan
   overlay has only about 688 bytes free, so the full library scanner cannot simply be added
   there. A private-only scanner would need a new code area or a deliberately small filename-only
   index, plus a progress path. Reusing the existing rescan overlay with a private scan root and
   a second output file is the closest fit, but even that needs a code-size check against the
   688-byte overlay margin before it can be promised.

For a first version, the PC-built index is the safe choice. If "copy and let the player parse it
without a computer" is a hard requirement, the on-device scanner is a separate phase.

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

Proposed third root, shown only when `/.private/` exists and has an index:

```text
All Songs → Favorites → Private → All Songs
```

- Short Back at a root cycles the roots; inside the private list it goes up as usual.
- Long Back: Settings, as on every normal screen; leaving Settings returns to the root that was
  current (including Private).
- Short Menu: Now Playing; on Now Playing it opens the private Up Next queue.
- Long Menu: A-Z jump in the private list, matching the normal list model.
- Play: open the selected recording; hold Play does nothing on a private track (no favorite
  action).
- Volume: move the row, hold to scroll; on Now Playing the normal volume/seek gestures remain.
- The top-left root meter currently has two 12 px cells. A third root needs either three 8 px
  cells inside the same 28 px meter or the meter can be hidden on Private. Three cells fit; the
  third colour should be distinct from All Songs blue and Favorites pink.
- If the folder is removed, the Private root disappears on the next start and the two-root model
  returns. The private resume/index can remain on the card harmlessly.

A secret key sequence to reveal/hide the root can be added later. It is an entry trick, not
encryption; see the note in [README.md](README.md#secret-entry-and-encryption).

## Capacity

- **Hard library limit:** 65,535 private tracks (16-bit indices; `LIBRARY_NONE` is `0xFFFF`).
- **Proposed private-list limit:** **2,048 tracks**.
- **Index reservation:** about 1 MB for 2,048 paths/titles; raise if the cap is raised.
- **Queue:** a private folder source query can play the whole 2,048-track list (the queue uses a
  browse source, not the 512-entry explicit Up Next list). If private tracks are copied into an
  explicit saved queue, the existing 512-entry Lite explicit-queue limit applies.
- The cap is independent of the 512-song Favorites capacity.

## RAM and code

The current Lite image has roughly 20.5 KB of code window and 95.5 KB of loaded RAM free at the
time of this note. A second `LibraryStore` plus its block cache is about 2.5–3 KB of static RAM;
a private list screen and the save/switch logic add more. That is likely to fit, but the private
scan, if built on the device, is the part most likely to exceed the current code/overlay budget.
The normal library and resident tables should not change.

## Test plan

- Main scan, All Songs, Favorites, A-Z, shuffle and resume are unchanged when `/.private/` exists.
- Private root is present only when the folder/index exists; removing it restores two roots.
- Normal → Private → Normal restores the normal queue, track and position; private resume is
  independent.
- Private tracks never appear in Favorites, All Songs, artist/album/genre lists, shuffle or M3U.
- A-Z in Private, empty folder, unsupported file, missing folder, corrupt private index.
- Capacity: 2,048 tracks load; the 2,049th is rejected or the index is capped with a clear
  message.
- USB copy/rescan: main rescan does not touch private tracks; the private index is rebuilt only
  when its build path runs.
- Device images still pass updater and RAM/code checks.

## Open decisions

1. PC-built private index for v1, or invest in an on-device private scanner now?
2. Third root always visible when the folder exists, or hidden behind a key sequence?
3. Third root meter: three cells or hide the meter on Private.
4. Private Up Next: reuse the normal Up Next screen and explicit queue, or omit it in v1.
