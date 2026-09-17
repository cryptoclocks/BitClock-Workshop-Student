#!/usr/bin/env python3
"""Render 320x240 reference screens using original V414_CDC coordinates/assets."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

W, H = 320, 240
ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "teaching-site" / "assets" / "screens"
SOURCE = Path("/Users/natthapongsuwanjit/Desktop/CryptoClockV3/Arduino/Version/V414_CDC/sd/4.1.4")
IMAGE_DIR = SOURCE / "System" / "image"
FONT = Path("/System/Library/Fonts/Supplemental/Andale Mono.ttf")

def f(size): return ImageFont.truetype(str(FONT), size)
def asset(name): return Image.open(IMAGE_DIR / name).convert("RGB")
def text(draw, xy, value, size, color): draw.text(xy, value, font=f(size), fill=color)

def profile():
    im = Image.new("RGB", (W, H), "black")
    photo = asset("profile.jpg").resize((100, 100), Image.Resampling.LANCZOS)
    mask = Image.new("L", (100, 100)); ImageDraw.Draw(mask).ellipse((1, 1, 99, 99), fill=255)
    im.paste(photo, (10, 10), mask)
    d = ImageDraw.Draw(im)
    text(d, (105, 15), "Don't Trust,Verify", 14, "#ffae00")
    text(d, (10, 140), "Satoshi Nakamoto", 15, "#ffae00")
    text(d, (10, 175), "(Sat) Founder", 15, "white")
    text(d, (10, 205), "Bitcoin", 15, "white")
    text(d, (152, 60), "10:30", 35, "white")
    text(d, (165, 110), "11/09/2026", 13, "white")
    return im

def coin(symbol, price, low, high, change, coin_image):
    # page2.cpp default background is page2.jpg and uses these exact anchor positions.
    im = asset("page2.jpg").resize((W, H), Image.Resampling.NEAREST); d = ImageDraw.Draw(im)
    for x in (20, 206):
        for i in range(5): d.rounded_rectangle((x-i, 14-i, x+95+i, 62+i), radius=6+i, outline="#ff0000", width=1)
        d.rounded_rectangle((x+5, 19, x+90, 57), radius=6, fill="black")
    im.paste(asset(coin_image).resize((48, 48), Image.Resampling.LANCZOS), (136, 10))
    text(d, (51 if len(symbol)==3 else 47, 30), symbol, 15, "white")
    text(d, (238, 30), "USDT", 15, "white")
    # วัดกล่องข้อความจริงก่อนวาด เพื่อให้ราคากึ่งกลางที่ x = 160 ทุกจำนวนหลัก
    price_font = f(31)
    price_box = d.textbbox((0, 0), price, font=price_font)
    price_x = (W - (price_box[2] - price_box[0])) // 2
    d.text((price_x, 115), price, font=price_font, fill="#ff0000")
    text(d, (10, 85), change, 9, "#00ff00" if not change.startswith("-") else "#ff0000")
    text(d, (268, 85), "+1.24%", 9, "#00ff00")
    text(d, (30, 180), "Lowest", 15, "#00ff00"); text(d, (223, 180), "Highest", 15, "#ff0000")
    text(d, (26, 200), low, 15, "white"); text(d, (221, 200), high, 15, "white")
    im.paste(asset("logo.jpg").resize((32, 32), Image.Resampling.LANCZOS), (144, 175))
    return im

def cdc(action):
    im = Image.new("RGB", (W, H), "black"); d = ImageDraw.Draw(im)
    text(d, (8, 7), "CDC ActionZone 3.0", 15, "white")
    color = "#00ff00" if action == "BUY" else "#ff0000"
    d.rounded_rectangle((250, 5, 319, 55), radius=8, fill=color)
    text(d, (274 if action == "BUY" else 268, 37), action, 15, "black" if action == "BUY" else "white")
    d.rounded_rectangle((12, 62, 52, 82), radius=4, outline="white"); text(d, (25, 67), "4H", 9, "white")
    im.paste(asset("coin1.jpg").resize((48, 48), Image.Resampling.LANCZOS), (10, 34)); text(d, (68, 47), "3,450,000", 23, "#f7931a")
    points = [(10,200),(25,192),(40,205),(55,177),(70,185),(85,165),(100,172),(115,142),(130,156),(145,130),(160,139),(175,119),(190,127),(205,104),(220,117),(235,93),(250,102),(265,82),(280,92),(295,75)]
    for i, (x,y) in enumerate(points):
        if i: d.line((points[i-1], (x,y)), fill="#00ffff", width=2)
        d.line((x, y-9, x, y+8), fill=color, width=1); d.rectangle((x-1,y-4,x+2,y+4), fill=color)
    text(d, (10, 98), "Max: 3500000.00", 8, "#666666"); text(d, (10, 227), "Min: 3300000.00", 8, "#666666")
    return im

def slide(): return Image.open(SOURCE / "bg001.jpg").convert("RGB").resize((W, H), Image.Resampling.LANCZOS)

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    screens = {
      "01-profile-original.png": profile(),
      "02-coin-btc-original.png": coin("BTC", "65,000", "64,250", "65,830", "+750", "coin1.jpg"),
      "03-coin-eth-original.png": coin("ETH", "3,450", "3,320", "3,520", "+130", "coin2.jpg"),
      "04-coin-bnb-original.png": coin("BNB", "580.25", "561.00", "590.20", "+19.25", "coin3.jpg"),
      "05-coin-doge-original.png": coin("DOGE", "0.125", "0.118", "0.132", "-0.007", "coin4.jpg"),
      "06-cdc-buy-original.png": cdc("BUY"), "07-cdc-sell-original.png": cdc("SELL"),
      "08-sd-slide-original.png": slide(),
    }
    for name, image in screens.items(): image.save(OUT / name, optimize=True)
    sheet = Image.new("RGB", (W*2, H*4), "#111111")
    for i, image in enumerate(screens.values()): sheet.paste(image, ((i%2)*W, (i//2)*H))
    sheet.save(OUT / "contact-sheet-original.png", optimize=True)
    print(f"generated {len(screens)} original-layout reference screens")

if __name__ == "__main__": main()
