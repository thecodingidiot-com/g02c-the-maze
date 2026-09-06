#!/bin/bash
# Generate the tileset and sprite sheet for g02c.
#
# tileset.png: 3 tiles, 32x32 each, side by side (96x32 total).
#   index 0 (src.x=0)  -- floor  (TILE_FLOOR = 1)
#   index 1 (src.x=32) -- wall   (TILE_WALL  = 2)
#   index 2 (src.x=64) -- exit   (TILE_EXIT  = 3)
#
# spritesheet.png: 3 frames, 24x24 each, side by side (72x24 total).
#   frame 0 -- idle, frames 1 and 2 -- walk cycle

set -e
mkdir -p assets

python3 - <<'PY'
from PIL import Image, ImageDraw

TILE = 32
ts = Image.new("RGB", (TILE * 3, TILE), (0, 0, 0))
d = ImageDraw.Draw(ts)
d.rectangle([0, 0, TILE - 1, TILE - 1], fill=(0x23, 0x26, 0x2f))              # floor
d.rectangle([TILE, 0, TILE * 2 - 1, TILE - 1], fill=(0x5a, 0x51, 0x47))       # wall
d.rectangle([TILE + 2, 2, TILE * 2 - 3, TILE - 3], fill=(0x6b, 0x60, 0x54))   # wall highlight
d.rectangle([TILE * 2, 0, TILE * 3 - 1, TILE - 1], fill=(0x23, 0x26, 0x2f))   # exit floor
d.ellipse([TILE * 2 + 6, 6, TILE * 3 - 7, TILE - 7], fill=(0xf2, 0xc9, 0x4c)) # exit marker
ts.save("assets/tileset.png")
print("  wrote assets/tileset.png")

FW, FH = 24, 24
sheet = Image.new("RGBA", (FW * 3, FH), (0, 0, 0, 0))
d = ImageDraw.Draw(sheet)
for i in range(3):
    ox = i * FW
    lean = (0, -1, 1)[i]
    d.ellipse([ox + 7, 2, ox + 16, 11], fill=(0xf0, 0xd0, 0xb0))     # head
    d.rectangle([ox + 8, 11, ox + 15, 18], fill=(0xd6, 0x3b, 0x3b))  # body
    d.rectangle([ox + 8, 18, ox + 10 + lean, 23], fill=(0x1c, 0x1c, 0x1c))
    d.rectangle([ox + 13, 18, ox + 15 - lean, 23], fill=(0x1c, 0x1c, 0x1c))
sheet.save("assets/spritesheet.png")
print("  wrote assets/spritesheet.png")
PY
