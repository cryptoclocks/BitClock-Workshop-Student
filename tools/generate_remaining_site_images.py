#!/usr/bin/env python3
"""Build deterministic workshop illustrations from the CryptoClock code and assets."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "teaching-site" / "assets" / "generated"
SCREENS = ROOT / "teaching-site" / "assets" / "screens"
FONT = "/System/Library/Fonts/Supplemental/Arial Unicode.ttf"
if not Path(FONT).exists(): FONT = "/System/Library/Fonts/Supplemental/Thonburi.ttc"

def font(size): return ImageFont.truetype(FONT, size)
def center(d, box, text, f, fill):
    b=d.textbbox((0,0),text,font=f); x=box[0]+(box[2]-box[0]-(b[2]-b[0]))//2
    d.text((x,box[1]),text,font=f,fill=fill)

def four_screens():
    canvas=Image.new("RGB",(1280,960),"#eef3f6"); d=ImageDraw.Draw(canvas)
    items=[("01-profile-original.png","PROFILE"),("02-coin-btc-original.png","COIN PRICE"),("06-cdc-buy-original.png","CDC ACTION ZONE"),("08-sd-slide-original.png","SD SLIDE")]
    for i,(name,label) in enumerate(items):
        im=Image.open(SCREENS/name).convert("RGB").resize((576,432),Image.Resampling.NEAREST)
        x=40+(i%2)*640; y=54+(i//2)*470
        canvas.paste(im,(x,y)); center(d,(x,y+440,x+576,y+465),label,font(20),"#0b2941")
    center(d,(0,12,1280,42),"CRYPTOCLOCK · 4 MAIN SCREENS",font(22),"#0b2941")
    canvas.save(OUT/"four-main-screens.png",optimize=True)

def json_serial():
    im=Image.new("RGB",(1280,720),"#0a1724"); d=ImageDraw.Draw(im)
    d.rounded_rectangle((36,52,622,676),18,fill="#102537",outline="#2f627f",width=3)
    d.rounded_rectangle((658,52,1244,676),18,fill="#06100d",outline="#2f627f",width=3)
    center(d,(36,72,622,105),"Bitkub Ticker JSON",font(24),"#49d7e7")
    center(d,(658,72,1244,105),"Serial Monitor · 115200 baud",font(24),"#49d7e7")
    code=['[','  {','    "symbol": "BTC_THB",','    "last": 3450000.00,','    "high_24_hr": 3490000.00,','    "low_24_hr": 3390000.00,','    "percent_change": 1.24','  }',']']
    for i,line in enumerate(code): d.text((72,135+i*54),line,font=font(22),fill="#d8ecf8")
    lines=['SD: ready, slides: 3','Wi-Fi connected','Settings: http://192.168.1.42/','BTC = 3450000.00 THB','ETH = 122450.00 THB','KUB = 58.40 THB','USDT = 33.21 THB','CDC history loaded']
    for i,line in enumerate(lines): d.text((694,135+i*54),line,font=font(21),fill="#42e69b" if i>=3 else "#d8ecf8")
    im.save(OUT/"bitkub-json-serial.png",optimize=True)

def sd_tree():
    im=Image.new("RGB",(1080,700),"#eef3f6"); d=ImageDraw.Draw(im)
    d.rounded_rectangle((48,42,1032,652),20,fill="white",outline="#b5c9d7",width=3)
    center(d,(0,72,1080,108),"โครงสร้าง MicroSD Card · FAT32",font(28),"#0b2941")
    rows=[("MICROSD  /",0,"#37cce1"),("profile.jpg   · 96 × 96 JPEG",1,"#f6ae3f"),("slides/",1,"#37cce1"),("slide1.jpg   · 320 × 240 JPEG",2,"#f6ae3f"),("slide2.jpg   · 320 × 240 JPEG",2,"#f6ae3f"),("slide3.jpg   · 320 × 240 JPEG",2,"#f6ae3f")]
    for i,(label,indent,color) in enumerate(rows):
        y=148+i*72; x=115+indent*86
        if indent: d.line((x-42,y-12,x-42,y+32),fill="#a6bbc9",width=3); d.line((x-42,y+32,x-18,y+32),fill="#a6bbc9",width=3)
        d.rounded_rectangle((x,y,x+34,y+30),5,fill=color)
        d.text((x+52,y-4),label,font=font(24),fill="#17354a")
    d.text((110,590),"ชื่อไฟล์ต้องเรียงลำดับต่อเนื่อง เริ่มจาก slide1.jpg",font=font(20),fill="#557083")
    im.save(OUT/"sd-card-structure.png",optimize=True)

if __name__ == "__main__":
    OUT.mkdir(parents=True,exist_ok=True); four_screens(); json_serial(); sd_tree(); print("generated deterministic site illustrations")
