# CJK sorting and section jump

Status: **implemented in MINIV932.**

## What is implemented

- Before the existing NFKD/casefold pipeline, each GB2312 Hanzi is replaced by
  its primary pinyin syllable and each kana by its romaji syllable. Latin and mapped CJK
  therefore share one A-Z order.
- The table is `assets/pinyin.bin`: 6,940 records (6,763 GB2312 Hanzi plus 177 kana),
  428 syllables, 17,822 bytes. It is lossless and checksum-pinned.
- A rescan copies the table into the borrowed core1 scratch before sorting. Normal
  browsing reads the already-sorted `library.db` and does no sorting.
- `sort_key_initial` returns the first pinyin/romaji letter for mapped CJK; the list maps
  `0x80` to the final `#` section. Digits and punctuation keep their ASCII initial and
  land in the leading `#` section.
- All Songs uses `LIBRARY_TITLE_ORDER`: every track that is not in a top-level
  `Audiobooks` / `Podcasts` folder. Chinese and kana tracks now interleave with Latin.
- Double Menu in a lettered list opens the A-Z/# overlay; volume selects a letter, Play jumps,
  and Back closes it. Volume hold only scrolls; there is no visible permanent A-Z index.

## Why All Songs can look like it has no Chinese songs

Before MINIV932 the order put every non-Latin title after `A`-`Z`, so a mixed library
showed the English titles first and the Chinese/Japanese titles at the bottom under `#`.
MINIV932 interleaves mapped CJK, so they appear under their pinyin/romaji letters.

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
code window       135,284 / 157,292   22,008 free
loaded total      404,372 / 437,600   33,228 free
resident tables    18,248 /  18,432      184 free
rescan overlay     85,400 /  86,016      616 free
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

The device already borrows 384 KB (0x60000) of core1 RAM for a rescan. The 17,822-byte
asset plus the 1,052-byte table leaves 364,760 bytes (356.2 KiB) for the builder; a
20,000-track builder test also passes with 340 KiB. A 60 KB table would leave about
324 KB, below that tested margin, so a full table does not fit the 20k-track budget.

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

## Decisions

- **Scope:** GB2312 6,763 Hanzi plus kana, compressed to about 17 KB. The compressed asset
  lives in the rescan scratch during sorting; there is no decoded working table.
- **Sort order:** interleave CJK under A-Z by pinyin/romaji.
- **Japanese:** kana uses per-character romaji (no digraph/sokuon/long-vowel contraction);
  kanji that is not in GB2312 falls back to `#`. Shared Hanzi uses its Chinese pinyin
  reading (日本 is `riben` under R, 大阪 is `daban` under D).
- **Jump UI:** keep the existing long volume press for this pass. A visible A-Z index is
  follow-up work.
- **Section source:** an on-demand flash lookup from the compressed table; no daily table.
  The lookup reads the block index and syllable table only when the selected row changes.
  If that cost is noticeable, a section byte in `library.db` is the follow-up.

## Data quality

Compression is lossless: the decompressed table is byte-identical to the generated source
table, and a checksum test pins that. The quality of the order depends on the reading data,
not on compression.

- The source is a pinned Unicode Unihan `kMandarin` extract (Unicode 16.0.0, Unicode License
  v3), with the license and source version kept beside the generated asset. A generated
  manifest records the source version and checksum.
- The generated table has 6,940 records: all 6,763 GB2312 Hanzi have a `kMandarin`
  reading, plus 177 kana. There are 426 distinct plain-pinyin/romaji syllables; 100
  characters have more than one Unihan reading and the generator takes the first. The
  asset is 17,822 bytes.
- One primary reading per character. Polyphonic characters can sort under the wrong reading
  (for example 重庆 as `zhong` rather than `chong`); this is a known limitation.
- Characters missing from the table fall back to `#` and are never guessed. Kana is
  romanized character by character (`あいみょん` -> `aimiyon`, `がっこう` -> `gatsukou`);
  Japanese kanji outside GB2312 stays under `#`, while shared Hanzi uses its Chinese
  reading.
- Tests cover the decompressed checksum, known characters, the PC/device sort-key
  agreement, mixed Latin/CJK order, kana, unknown fallback, and the byte-for-byte library
  comparison on a CJK fixture.
