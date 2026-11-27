import sys
import unittest
from functools import lru_cache
from pathlib import Path

TOOLS_DIRECTORY = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS_DIRECTORY))

import pinyin_table
from pinyin_table import PinyinTable

ASSET = TOOLS_DIRECTORY.parent / "assets" / "pinyin.bin"


@lru_cache(maxsize=1)
def gb2312_hanzi():
    characters = []
    for first in range(0xA1, 0xF8):
        for second in range(0xA1, 0xFF):
            try:
                character = bytes((first, second)).decode("gb2312")
            except UnicodeDecodeError:
                continue
            if "\u4e00" <= character <= "\u9fff":
                characters.append(character)
    return characters


class PinyinAssetTest(unittest.TestCase):
    def test_gb2312_is_complete(self):
        table = pinyin_table.load(ASSET)
        for character in gb2312_hanzi():
            self.assertIsNotNone(table.syllable(ord(character)), character)
        self.assertEqual(6763, len(gb2312_hanzi()))

    def test_known_readings(self):
        table = pinyin_table.load(ASSET)
        self.assertEqual("ni", table.syllable(ord("你")))
        self.assertEqual("zhou", table.syllable(ord("周")))
        self.assertEqual("ai", table.syllable(ord("爱")))
        self.assertEqual("a", table.syllable(ord("あ")))
        self.assertIsNone(table.syllable(ord("\ufa19")))

    def test_kana_maps_to_romaji(self):
        table = pinyin_table.load(ASSET)
        self.assertEqual("a", table.syllable(ord("あ")))
        self.assertEqual("ka", table.syllable(ord("か")))
        self.assertEqual("ga", table.syllable(ord("が")))
        self.assertEqual("a", table.syllable(ord("ア")))
        self.assertEqual("n", table.syllable(ord("ん")))

    def test_asset_is_within_budget(self):
        self.assertLessEqual(ASSET.stat().st_size, 18000)

    def test_truncated_asset_is_rejected(self):
        with self.assertRaises(ValueError):
            pinyin_table.decode(b"EPY1")

    def test_decoded_table_exposes_type(self):
        table = pinyin_table.load(ASSET)
        self.assertIsInstance(table, PinyinTable)
        self.assertEqual(6763, table.hanzi_count())


if __name__ == "__main__":
    unittest.main()
