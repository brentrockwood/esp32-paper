#!/usr/bin/env python3
"""Convert any image to the raw 1bpp bitmap format for the ESP32 e-paper display.

Usage:
    python3 scripts/img_to_epaper.py <input-image> <output.bin>

Output: 3904 bytes — 122 rows × 32 bytes, 1bpp MSB-first (1=black, 0=white).
Send:   curl -F bitmap=@output.bin http://<ip>/display
"""
import sys
from PIL import Image, ImageOps

W, H = 250, 122
ROW_BYTES = (W + 7) // 8  # 32 (250 pixels + 6 padding bits)


def convert(src: str, dst: str) -> None:
    img = Image.open(src).convert('RGB')

    # Scale to fit 250×122, preserving aspect ratio; letterbox with white.
    img.thumbnail((W, H), Image.LANCZOS)
    canvas = Image.new('RGB', (W, H), (255, 255, 255))
    canvas.paste(img, ((W - img.width) // 2, (H - img.height) // 2))

    # Greyscale → invert → Floyd-Steinberg dither to 1bpp.
    # Invert because PIL encodes white as 1 but the display treats 1 as black.
    bw = ImageOps.invert(canvas.convert('L')).convert('1')

    # Pack pixels MSB-first; 6 trailing bits of the last byte in each row are 0.
    pixels = bw.load()
    data = bytearray()
    for y in range(H):
        for xb in range(ROW_BYTES):
            byte = 0
            for bit in range(8):
                x = xb * 8 + bit
                if x < W and pixels[x, y]:
                    byte |= 0x80 >> bit
            data.append(byte)

    with open(dst, 'wb') as f:
        f.write(data)
    print(f'{src} → {dst}  ({len(data)} bytes, {img.width}×{img.height} → {W}×{H})')


if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.exit(f'usage: {sys.argv[0]} <input-image> <output.bin>')
    convert(sys.argv[1], sys.argv[2])
