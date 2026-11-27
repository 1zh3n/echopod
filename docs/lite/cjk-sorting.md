# CJK sorting and section jump

Status: **proposal, not implemented.** MINIV931 still uses the sort described under
[What happens now](#what-happens-now).

## What happens now

- The sort key is Unicode NFKD, combining marks removed, case-folded, and a leading
  `The ` is ignored. Latin names sort A-Z; other scripts keep their code points and sort
  after all Latin text.
- `sort_key_initial` maps ASCII and Latin-1/Extended-A letters to `A`-`Z`. Every other
  script returns the final `#` section. Chinese and Japanese entries therefore all share
  the last section.
- All Songs uses `LIBRARY_TITLE_ORDER`: every track that is not in a top-level
  `Audiobooks` / `Podcasts` folder. Chinese tracks are in the list. They sit after the
  Latin titles, under `#`; the first screen can look English-only.
- Long-press volume up/down for about 3 seconds in a lettered list to jump sections. The
  list shows one letter in a box while jumping. There is no visible A-Z index.

## Why All Songs can look like it has no Chinese songs

The tracks are present. The current order puts every non-Latin title after `A`-`Z`, so a
mixed library shows the English titles first and the Chinese/Japanese titles at the very
bottom under `#`. Jump to `#` with the long volume press to see them.

If they are still absent after jumping to `#`, that is a different problem: the files may
be in a top-level `Audiobooks` / `Podcasts` folder (marked spoken and excluded from the
music count), or the scan may have skipped files whose tags/paths it could not read.

## Goal

- Sort Chinese by pinyin and Japanese kana by romaji.
- Unknown Hanzi/kanji fall back to `#` rather than being guessed.
- Make the A-Z/# jump usable in All Songs, Albums, Artists, Album Artists, Genres and
  Playlists.
- Keep the daily RAM cost at zero for the sorting table and avoid per-character flash
  reads.

## Non-goals

- Complete Japanese kanji readings. Kana is practical; kanji needs a reading dictionary
  and context, so those entries can stay under `#`.
- Tone-aware pinyin ordering. One common reading per character is enough for list order.
- A visible index is a separate UI decision; this document covers the sorting data and the
  existing long-press jump.

## Budget data

Current release-lite build:

```text
code window       134,412 / 157,292   22,880 free
loaded total      403,484 / 437,600   34,116 free
resident tables    18,248 /  18,432      184 free
rescan overlay     84,360 /  86,016    1,656 free
scan scratch                393,216 B (borrowed core1 RAM during a rescan)
20k-track target            about 360 KB of scan memory
```

Daily RAM is already used by `music_ring` (112,000 B), `app` (92,400 B), `strip_pixels`
(21,760 B), `volumes` (11,016 B), `icon_buffers` (8,192 B) and the rest. The 34 KB free is
headroom for the UI, caches, stack and later work; it is not a table budget.

Table estimates:

| Scope | Size |
|---|---:|
| 3,500 common Hanzi + pinyin syllables | about 12 KB |
| GB2312 6,763 Hanzi | about 22 KB |
| full CJK, about 20,000 Hanzi | about 60 KB |
| kana to romaji | under 1 KB |

## Where the table goes

The device already borrows 384 KB (0x60000) of core1 RAM for a rescan. A 12 KB table
leaves about 372 KB for the builder, above the 360 KB target. A 22 KB table leaves about
362 KB, at the edge. A 60 KB table leaves about 324 KB, below the target, so a full table
does not fit the 20k-track budget.

`code window` is not the right place even when a small table fits: it competes with code
and the 22 KB GB2312 table would leave almost no margin. The table belongs in the rescan
scratch and is released when the rescan ends.

For the daily A-Z jump, the full table is not needed. Two choices:

1. **Section byte in `library.db`.** The PC indexer and the device builder write one
   section byte per lettered string/row. Browsing reads that byte; daily RAM cost is zero.
   This changes the library format and needs matching tests.
2. **On-demand flash lookup.** The table stays in the image; when the selected row changes,
   look up only that row's first character. A few reads per selection, zero daily RAM, no
   format change. A small cache can cover repeated rows.

A small initial table in daily RAM is possible (3,500 chars: about 10.5 KB; GB2312: about
20.3 KB) but it consumes most of the 34 KB headroom, so it is not the first choice.

## Performance

Sorting happens only during a device rescan (or a budget rebuild), and only on unique text
runs, not on every track. The normal browsing path reads a database already sorted by the
PC indexer, so it does no sorting at all.

Host microbenchmarks of the current `sort_key_make`: 4M CJK characters take about 86 ms;
2M binary searches over a 6.7k-entry table take about 48 ms. Scaled to the 96 MHz device
(roughly 30-50 times slower), 200k CJK characters cost about 0.3-0.5 s and 2M about 2-5 s.
A table in RAM or scratch avoids per-character eMMC reads; a table in flash would need a
block cache and would be much slower.

## Open decisions

- **Sort order:** interleave CJK under A-Z by pinyin/romaji, or keep English first and put
  the CJK group after it, sorted within the group. Both need the same table; the choice is
  presentation, not cost.
- **Scope:** 3,500 common characters first, or GB2312 6,763; whether a compressed table is
  wanted to keep more of the scan budget.
- **Japanese:** kana romaji only (recommended), or an attempt at kanji readings.
- **Jump UI:** keep the long volume press, or add a visible A-Z/# index.
- **Section source:** a section byte in `library.db`, or an on-demand flash lookup.
