#!/bin/bash
# Install the libraries already bundled with this workshop package.
# Double-click this file on macOS, then reopen Arduino IDE.

set -euo pipefail

PACKAGE_DIR="$(cd "$(dirname "$0")" && pwd)"
SOURCE_DIR="$PACKAGE_DIR/bundled-libraries"
DESTINATION_DIR="$HOME/Documents/Arduino/libraries"

if [[ ! -d "$SOURCE_DIR" ]]; then
  echo "ไม่พบโฟลเดอร์ bundled-libraries"
  read -r -p "กด Enter เพื่อปิด..."
  exit 1
fi

mkdir -p "$DESTINATION_DIR"

installed=0
skipped=0
for library_dir in "$SOURCE_DIR"/*; do
  [[ -d "$library_dir" ]] || continue
  library_name="$(basename "$library_dir")"
  target="$DESTINATION_DIR/$library_name"

  if [[ -e "$target" ]]; then
    echo "ข้าม $library_name (มีอยู่แล้ว)"
    skipped=$((skipped + 1))
    continue
  fi

  ditto "$library_dir" "$target"
  echo "ติดตั้ง $library_name"
  installed=$((installed + 1))
done

echo
echo "เสร็จแล้ว: ติดตั้ง $installed ไลบรารี, ข้าม $skipped ไลบรารี"
echo "ปิดและเปิด Arduino IDE ใหม่ แล้วเลือก Board: ESP32 Dev Module"
read -r -p "กด Enter เพื่อปิด..."
