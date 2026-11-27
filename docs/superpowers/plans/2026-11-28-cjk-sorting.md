# CJK Pinyin Sorting Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Sort GB2312 Chinese by pinyin and Japanese kana by romaji, interleaved under A-Z, with a lossless compressed table that lives in the rescan scratch and in the image, not in daily RAM.

**Architecture:** `assets/pinyin.bin` holds 6,763 GB2312 Hanzi plus kana mapped to plain syllables, compressed with per-block codepoint deltas and frequency-coded syllable indices. The PC indexer and the device builder read the same asset. During a rescan the compressed asset is copied into the borrowed 384 KB core1 scratch; `sort_key` searches it in place. Browsing looks up only the selected row's first character from the image through a tiny block cache. Latin and mapped CJK keys are compared directly, so scripts interleave. Missing characters and Japanese kanji keep their code point and fall under `#`.

**Tech Stack:** Python 3 (generator, PC indexer), C11 (device), CMake, existing `library/sort_key` and `library/library_scan` code.

**Spec:** `docs/lite/cjk-sorting.md`

## Global Constraints

- 512 MB Echo Mini only; the 8 GB model is not supported.
- Do not modify the boot chain, IDB, PMU firmware, fw2 or slot layout.
- Daily RAM table cost is zero. The compressed table lives in the rescan scratch and the image.
- Table scope: GB2312 6,763 Hanzi plus kana; Japanese kanji and missing characters fall back to `#`.
- Sort order: interleaved A-Z. Unknown CJK keeps its code point and sorts after mapped text.
- One table source. PC and device sort keys must agree for the same text.
- Compression is lossless. The decoded records are pinned by CRC and a generated manifest.
- No comments in C source; `no_comments` must pass.
- Keep `code window`, `loaded total`, `resident tables`, `rescan overlay` and `np_faces` within their limits.
- Run host tests with `ctest --preset dev -E "^format$"`; Lite tests with the `test_lite_panel` target; device builds with `cmake --build --preset device`.

## Review Focus

- Polyphonic characters use one primary reading; 重庆 (`zhong`), 长沙 (`zhang`), 音乐 (`le`) are expected to differ from context-aware pinyin.
- Japanese kanji must fall under `#`, while kana maps to romaji; mixed kana/kanji names must not crash or reorder unpredictably.
- A malformed, truncated or missing pinyin asset must fall back to the current Unicode order, not crash or produce an empty library.
- The rescan scratch reservation must leave the builder above its 20k-track budget.
- Mixed Latin/CJK/kana ties must produce the same byte order on the PC and the device.
- Section initials at the boundaries (`#` before letters, `#` after letters, digits, unknown CJK) must stay distinct.

---

### Task 1: Generate and validate the compressed pinyin asset

**Files:**
- Create: `tools/data/pinyin-gb2312.txt` (pinned Unihan 16.0.0 extract plus kana)
- Create: `tools/pinyin_table.py`
- Create: `tools/tests/test_pinyin_table.py`
- Create: `assets/pinyin.bin` (generated)
- Create: `assets/pinyin.md` (manifest, source version, license, checksums)
- Modify: `tests/CMakeLists.txt` (add `pinyin_table_up_to_date`)

**Interfaces:**
- Consumes: Unicode 16.0.0 `Unihan_Readings.txt` and the GB2312 codec.
- Produces: `pinyin_table.load(path) -> PinyinTable` in Python; `assets/pinyin.bin` with the format below.

```text
header  magic "EPY1" | version u8 = 1 | block_shift u8 = 5 | count u16 | decoded_crc32 u32
index   block_count * { first_codepoint u16, data_offset u16 }
data    per block, per record:
        delta u8 (0xFF then u16 absolute delta when >= 255)
        syllable_code u8 (0xFF then u8 code-255 when >= 255)
syllables  count u16, then { length u8, ASCII bytes } per entry
```

Records are sorted by code point. Syllables are ordered by descending frequency so the 255 most common get one-byte codes. The decoded CRC is over `{ codepoint u16, syllable_index u16 }` for every record.

- [ ] **Step 1: Write the failing generator tests**

```python
def test_gb2312_is_complete(self):
    table = pinyin_table.load(ASSET)
    for code in gb2312_hanzi():
        self.assertIsNotNone(table.syllable(ord(code)))

def test_known_readings(self):
    table = pinyin_table.load(ASSET)
    self.assertEqual("ni", table.syllable(ord("你")))
    self.assertEqual("zhou", table.syllable(ord("周")))
    self.assertEqual("ai", table.syllable(ord("爱")))
    self.assertEqual("a", table.syllable(ord("あ")))
    self.assertIsNone(table.syllable(ord("漢")))  # Japanese kanji is not in the table

def test_asset_is_within_budget(self):
    self.assertLessEqual(ASSET.stat().st_size, 18000)

def test_truncated_asset_is_rejected(self):
    with self.assertRaises(ValueError):
        pinyin_table.decode(b"EPY1")
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `python3 -m unittest tools.tests.test_pinyin_table -v`
Expected: FAIL because `tools/pinyin_table.py` and `assets/pinyin.bin` do not exist.

- [ ] **Step 3: Implement the extractor, compressor and reader**

`tools/pinyin_table.py` provides `extract`, `build`, `check` and `load`. `extract` reads Unihan `kMandarin`, strips tone marks with NFKD, lowercases, replaces `ü` with `v`, takes the first reading, and writes one `U+XXXX<TAB>syllable` line per character. Kana lines use a fixed Hepburn map for `U+3041..U+30FF`; kanji is not added. `build` emits the asset and manifest. `load` decodes the asset in Python for the indexer and tests.

- [ ] **Step 4: Generate the asset and run the tests**

Run:
```bash
python3 tools/pinyin_table.py extract --unihan /path/to/Unihan_Readings.txt \
    --output tools/data/pinyin-gb2312.txt
python3 tools/pinyin_table.py build --source tools/data/pinyin-gb2312.txt \
    --output assets/pinyin.bin --manifest assets/pinyin.md
python3 -m unittest tools.tests.test_pinyin_table -v
```
Expected: all tests pass; `assets/pinyin.bin` is about 17.2 KB and the manifest records 6,763 Hanzi, 401 syllables and the source version.

- [ ] **Step 5: Add the up-to-date test and commit**

Add `pinyin_table_up_to_date` next to `sort_fold_up_to_date`, running `tools/pinyin_table.py check --source tools/data/pinyin-gb2312.txt --asset assets/pinyin.bin --manifest assets/pinyin.md`.

```bash
git add tools/data/pinyin-gb2312.txt tools/pinyin_table.py tools/tests/test_pinyin_table.py \
    assets/pinyin.bin assets/pinyin.md tests/CMakeLists.txt
git commit -m "feat: add the compressed pinyin table"
```

---

### Task 2: Add pinyin keys to the PC indexer

**Files:**
- Modify: `tools/library_index.py`
- Modify: `tools/tests/test_library_index.py`

**Interfaces:**
- Consumes: `pinyin_table.load(path)` from Task 1.
- Produces: `library_index.sort_key(text)` maps kana/CJK to syllables before the existing NFKD/casefold; `library_index.sort_initial(text)` returns `A`-`Z` or `#`.

- [ ] **Step 1: Write the failing indexer tests**

```python
def test_cjk_sorts_by_pinyin(self):
    self.assertEqual("zhoujielun", library_index.sort_key("周杰伦"))
    self.assertEqual("aimyon", library_index.sort_key("あいみょん"))

def test_scripts_interleave(self):
    names = ["周杰伦", "Adele", "あいみょん", "Zara"]
    self.assertEqual(["Adele", "あいみょん", "Zara", "周杰伦"],
                     sorted(names, key=library_index.sort_key))

def test_unknown_kanji_falls_after_mapped_text(self):
    self.assertGreater(library_index.sort_key("漢"), library_index.sort_key("zzz"))
    self.assertEqual("#", library_index.sort_initial("漢"))

def test_polyphonic_uses_primary_reading(self):
    self.assertEqual("zhongqing", library_index.sort_key("重庆"))
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `python3 -m unittest tools.tests.test_library_index.SortKeyTest -v`
Expected: FAIL on the CJK assertions.

- [ ] **Step 3: Implement the mapping**

In `sort_key`, replace each character with its table syllable when one exists, then run the existing NFKD/casefold/article pipeline on the result. `sort_initial` maps the first surviving character the same way; if it is not A-Z after mapping, return `#`. Load the asset lazily and cache it.

- [ ] **Step 4: Run the PC tests and the asset check**

Run:
```bash
python3 -m unittest tools.tests.test_library_index -v
ctest --preset dev -E "^format$" -R "pinyin_table_up_to_date|library_index"
```
Expected: pass; existing Latin/accent/natural tests unchanged.

- [ ] **Step 5: Commit**

```bash
git add tools/library_index.py tools/tests/test_library_index.py
git commit -m "feat: sort CJK by pinyin in the PC indexer"
```

---

### Task 3: Add the device table reader and sort-key mapping

**Files:**
- Create: `src/library/include/library/pinyin.h`
- Create: `src/library/pinyin.c`
- Modify: `src/library/sort_key.c`
- Modify: `src/library/sort_initial.c`
- Modify: `src/library/CMakeLists.txt`
- Create: `tests/library/test_pinyin.c`
- Modify: `tests/library/test_sort_key.c`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Produces:
```c
typedef struct PinyinTable {
    const uint8_t *data;
    uint32_t size;
    const uint8_t *index;
    const uint8_t *records;
    const uint8_t *syllables;
    uint16_t count;
    uint16_t blocks;
    uint16_t syllable_count;
} PinyinTable;

bool pinyin_table_open(PinyinTable *table, const uint8_t *data, uint32_t size);
const char *pinyin_table_syllable(const PinyinTable *table, uint32_t codepoint);
char pinyin_table_initial(const PinyinTable *table, uint32_t codepoint);
void pinyin_table_set(const PinyinTable *table);
const PinyinTable *pinyin_table_get(void);
```

- [ ] **Step 1: Write the failing reader and key tests**

```c
static void cjk_keys_match_the_pc_indexer(void)
{
    PinyinTable table;
    CHECK(pinyin_table_open(&table, asset, asset_size));
    pinyin_table_set(&table);
    char key[SORT_KEY_CAPACITY];
    sort_key_make("\xE5\x91\xA8\xE6\x9D\xB0\xE4\xBC\xA6", key, sizeof key);
    CHECK_EQUAL(0, strcmp("zhoujielun", key));
    sort_key_make("\xE3\x81\x82\xE3\x81\x84\xE3\x81\xBF\xE3\x82\x87\xE3\x82\x93",
                  key, sizeof key);
    CHECK_EQUAL(0, strcmp("aimyon", key));
}

static void malformed_table_is_refused(void)
{
    PinyinTable table;
    CHECK(!pinyin_table_open(&table, (const uint8_t *)"EPY1", 4));
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `ctest --preset dev -R test_sort_key --output-on-failure`
Expected: FAIL because the pinyin reader does not exist.

- [ ] **Step 3: Implement the reader and mapping**

`pinyin_table_open` validates the header, block index and syllable bounds; it keeps a view over the compressed asset in RAM and does not allocate. `pinyin_table_syllable` binary-searches the block index, decodes the block's deltas, and returns the syllable string. `sort_key_make` appends the syllable before `append_folded`; if there is no entry it keeps the existing behavior. `sort_key_initial` returns the syllable's first letter for mapped characters and keeps the existing Latin/`#` behavior otherwise.

- [ ] **Step 4: Run the device library tests**

Run: `ctest --preset dev -R "test_sort_key|test_pinyin" --output-on-failure`
Expected: pass; existing Latin, accent, Hangul and fold cases unchanged.

- [ ] **Step 5: Commit**

```bash
git add src/library/include/library/pinyin.h src/library/pinyin.c \
    src/library/sort_key.c src/library/sort_initial.c src/library/CMakeLists.txt \
    tests/library/test_pinyin.c tests/library/test_sort_key.c tests/CMakeLists.txt
git commit -m "feat: map CJK to pinyin in the device sort key"
```

---

### Task 4: Use the table during a rescan and prove PC/device equality

**Files:**
- Modify: `src/library/include/library/library_scan.h`
- Modify: `src/library/library_scan.c`
- Modify: `src/app/library_rescan.c`
- Modify: `src/hal/include/hal/system.h`
- Modify: `platform/device/board/glyphs.c`
- Modify: `platform/device/CMakeLists.txt`
- Create: `tests/fixtures/cjk_music/` and its expected database
- Modify: `tests/library/` comparison tests and `tests/CMakeLists.txt`

**Interfaces:**
- Produces:
```c
typedef struct ScanJob {
    ...
    const uint8_t *pinyin;
    uint32_t pinyin_size;
} ScanJob;

uint32_t hal_system_pinyin_size(void);
bool hal_system_pinyin_read(uint32_t offset, void *target, uint32_t length);
```

- [ ] **Step 1: Write the failing rescan fixture test**

Create a four-track fixture: `Adele - Hello.flac`, `Zara - ...flac`, `周杰伦 - 七里香.flac`, `あいみょん - マリーゴールド.flac`. Generate the expected library with `tools/library_index.py`, then assert the device scan produces the same bytes and the order `Adele`, `あいみょん`, `Zara`, `周杰伦`. Also assert a scan with `job.pinyin = NULL` keeps the existing code-point order.

- [ ] **Step 2: Run the test to verify it fails**

Run: `ctest --preset dev -R "cjk|library_compare" --output-on-failure`
Expected: FAIL because the device still sorts CJK by code point.

- [ ] **Step 3: Implement the scratch hand-off**

`library_rescan.c` asks `hal_system_pinyin_size`, reads the asset into the start of the borrowed scratch, and sets `job.pinyin`/`job.pinyin_size`. `library_scan` opens the table, calls `pinyin_table_set`, advances `split_memory` past the aligned asset, and clears the table after the scan. If the asset is absent or malformed, log nothing and keep the current sort.

Add `PINYIN_ENTRY` after `LITE_ICONS_ENTRY` in `board/glyphs.c`; append `--glyph-face assets/pinyin.bin` in the Lite block of `platform/device/CMakeLists.txt`; add the HAL read functions.

- [ ] **Step 4: Verify memory and equality**

Run:
```bash
ctest --preset dev -E "^format$" -R "cjk|library_compare|builder"
cmake --build --preset device
python3 tools/size_report.py --build build/device --summary
```
Expected: byte-identical CJK library; 20k-track builder still fits; rescan overlay within 86,016 bytes.

- [ ] **Step 5: Commit**

```bash
git add src/library/include/library/library_scan.h src/library/library_scan.c \
    src/app/library_rescan.c src/hal/include/hal/system.h platform/device/board/glyphs.c \
    platform/device/CMakeLists.txt tests/fixtures/cjk_music tests/library tests/CMakeLists.txt
git commit -m "feat: sort CJK during a rescan from the scratch table"
```

---

### Task 5: Look up section initials from the image

**Files:**
- Modify: `src/library/sort_initial.c`
- Modify: `src/library/pinyin.c`
- Modify: `src/hal/include/hal/system.h`
- Modify: `platform/device/board/glyphs.c`
- Modify: `platform/sim/sim_glyphs.c` or add `platform/sim/sim_pinyin.c`
- Modify: `tests/app/test_lite_panel.c`
- Modify: `tests/library/test_sort_key.c`

**Interfaces:**
- Produces: `char pinyin_initial_from_image(uint32_t codepoint);`
- Uses: `hal_system_pinyin_size` / `hal_system_pinyin_read`.

- [ ] **Step 1: Write the failing section test**

```c
static void cjk_section_initials_follow_pinyin(void)
{
    CHECK_EQUAL('Z', sort_key_initial("\xE5\x91\xA8"));
    CHECK_EQUAL('A', sort_key_initial("\xE3\x81\x82"));
    CHECK_EQUAL('#', sort_key_initial("\xE6\xBC\xA2"));
    CHECK_EQUAL('#', sort_key_initial("123"));
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `ctest --preset dev -R test_sort_key --output-on-failure`
Expected: FAIL because CJK still returns `#`.

- [ ] **Step 3: Implement the image lookup**

Add a small block cache (header, index block, one data block and one syllable) in `pinyin.c`. `pinyin_initial_from_image` uses it; `sort_key_initial` calls it for non-Latin characters. The app draw path calls only this function, never the full table. The simulator reads `assets/pinyin.bin`; the device reads the glyph entry.

- [ ] **Step 4: Run the UI and sort tests**

Run:
```bash
ctest --preset dev -R "test_sort_key|test_lite_panel" --output-on-failure
cmake --build build/lite-test --target test_lite_panel
./build/lite-test/tests/test_lite_panel
```
Expected: pass; the selected-row section for 周 is `Z`, for あ is `A`, for unknown kanji is `#`.

- [ ] **Step 5: Commit**

```bash
git add src/library/sort_initial.c src/library/pinyin.c src/hal/include/hal/system.h \
    platform/device/board/glyphs.c platform/sim tests/app/test_lite_panel.c \
    tests/library/test_sort_key.c
git commit -m "feat: use pinyin initials for section jumps"
```

---

### Task 6: Document, build and verify the image

**Files:**
- Modify: `docs/lite/cjk-sorting.md` (status implemented, measured sizes)
- Modify: `docs/lite/shortcuts.md` (long-press section jump)
- Modify: `docs/lite/README.md`
- Modify: `docs/lite/install-512m.md`
- Modify: `README.md` / `README.zh-CN.md` if they describe sorting

- [ ] **Step 1: Update the docs**

Record the table size, source/license, 6,763/6,763 coverage, 401 syllables, polyphonic and Japanese-kanji limits, and the A-Z/# jump gesture. Mark the spec implemented.

- [ ] **Step 2: Run the full verification**

Run:
```bash
ctest --preset dev -E "^format$"
cmake --build --preset release-lite
cmake --build --preset device
python3 tools/size_report.py --build build/release-lite --summary
python3 tools/make_image.py --help >/dev/null
```
Expected: all host tests pass; Lite tests pass; code window/loaded total within limits; updater check valid.

- [ ] **Step 3: Build and record the image**

Copy `build/release-lite/HIFIEC90.IMG` to the next `MINIVnnn.IMG`, run the updater check, and update the hash in `docs/lite/install-512m.md`.

- [ ] **Step 4: Device check**

Flash on the 512 MB unit and verify:
- All Songs interleaves English, Chinese and kana by A-Z.
- Long-press volume jumps to the `Z` section for 周 and `A` for あ.
- A kanji-only name still lands under `#`.
- Scan time and memory are acceptable.

- [ ] **Step 5: Commit and push**

```bash
git add docs README.md README.zh-CN.md
git commit -m "docs: record CJK pinyin sorting"
git push origin main
```
