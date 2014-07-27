#!/bin/sh

python MagicTest.py up ../../Web/firstTest/defaultConfig/ExtractedFont10x16_1bit_128.magicFont 128 $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/ExtractedFont7x12_1bit_129.magicFont 129 $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/ExtractedFont6x8_1bit_130.magicFont 130 $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/ExtractedFont4x6_1bit_131.magicFont 131 $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/bitmaps_cree/01_heart2_RGB.magicBitmap `expr 2 + $1` $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/bitmaps_cree/03_smilie_RGB.magicBitmap `expr 1 + $1` $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/bitmaps_cree/13_star_RGB.magicBitmap `expr 3 + $1` $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/bitmaps_cree/04_oneup.magicBitmap `expr 4 + $1` $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/bitmaps_cree/05_mushroom_RGB.magicBitmap `expr 5 + $1` $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/bitmaps_cree/06_mario_RGB.magicBitmap `expr 6 + $1` $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/bitmaps_cree/07_blueGhost_RGB.magicBitmap `expr 7 + $1` $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/bitmaps_cree/08_redGhost_RGB.magicBitmap `expr 8 + $1` $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/bitmaps_cree/09_BubbleBobble_RGB.magicBitmap `expr 9 + $1` $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/bitmaps_cree/10_invader_RGB.magicBitmap `expr 10 + $1` $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/bitmaps_cree/11_giraffe_RGB.magicBitmap `expr 11 + $1` $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/bitmaps_cree/12_cursor_RGB.magicBitmap `expr 12 + $1` $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/bitmaps_cree/14-16_spinsmile.magicBitmap `expr 13 + $1` $2
python MagicTest.py up ../../Web/firstTest/defaultConfig/bitmaps_cree/17_nyancat.magicBitmap `expr 17 + $1` $2

