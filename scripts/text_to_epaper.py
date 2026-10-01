#!/usr/bin/env python3
"""Render text to the ESP32 e-paper display's raw 1bpp bitmap format.

Usage:
    python3 scripts/text_to_epaper.py 'Hello' > message.bin
    printf 'Hello\nworld' | python3 scripts/text_to_epaper.py > message.bin
"""
import sys
from PIL import Image, ImageDraw, ImageFont, ImageOps

W, H = 250, 122
ROW_BYTES = (W + 7) // 8


def bitmap(text: str) -> bytes:
    image = Image.new('L', (W, H), 'white')
    draw = ImageDraw.Draw(image)
    font = ImageFont.load_default(size=20)
    x, y, line_height = 4, 4, 24

    for paragraph in text.splitlines() or ['']:
        words, line = paragraph.split(), ''
        for word in words:
            candidate = f'{line} {word}'.strip()
            if line and draw.textlength(candidate, font=font) > W - 2 * x:
                draw.text((x, y), line, font=font, fill='black')
                y, line = y + line_height, word
            else:
                line = candidate
        if y >= H:
            break
        draw.text((x, y), line, font=font, fill='black')
        y += line_height

    pixels = ImageOps.invert(image).convert('1').load()
    return bytes(
        sum((0x80 >> bit) for bit in range(8)
            if xb * 8 + bit < W and pixels[xb * 8 + bit, y])
        for y in range(H) for xb in range(ROW_BYTES)
    )


if __name__ == '__main__':
    text = ' '.join(sys.argv[1:]) if sys.argv[1:] else sys.stdin.read()
    data = bitmap(text)
    assert len(data) == ROW_BYTES * H
    sys.stdout.buffer.write(data)
