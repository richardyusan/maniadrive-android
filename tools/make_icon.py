#!/usr/bin/env python3
"""Generates the launcher icons (android/res/mipmap-*) from the game's own
menu logo (data/mania_logo2.tga). Run once; the PNGs are committed."""
import os, sys
from PIL import Image, ImageDraw

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
logo = Image.open(os.path.join(root, "data", "mania_logo2.tga")).convert("RGB")
# square crop around the "Mania Drive" lettering
crop = logo.crop((88, 30, 424, 366))
sizes = {"mdpi": 48, "hdpi": 72, "xhdpi": 96, "xxhdpi": 144, "xxxhdpi": 192}
for dens, px in sizes.items():
    im = crop.resize((px, px), Image.LANCZOS)
    mask = Image.new("L", (px, px), 0)
    ImageDraw.Draw(mask).rounded_rectangle((0, 0, px - 1, px - 1), radius=px // 5, fill=255)
    out = Image.new("RGBA", (px, px), (0, 0, 0, 0))
    out.paste(im, (0, 0), mask)
    d = os.path.join(root, "android", "res", "mipmap-" + dens)
    os.makedirs(d, exist_ok=True)
    out.save(os.path.join(d, "ic_launcher.png"))
    print("wrote", d)
