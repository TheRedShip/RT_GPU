#!/usr/bin/env bash
set -e

if [ $# -ne 3 ]; then
    echo "Usage: $0 target_size_MB input_file output_file"
    exit 1
fi

target_size_mb="$1"
input_file="$2"
output_file="$3"

duration=$(ffprobe -v error -select_streams v:0 -show_entries format=duration \
    -of default=noprint_wrappers=1:nokey=1 "$input_file")
duration=${duration%.*}  # remove decimals
target_size_kbit=$((target_size_mb * 8192))  # 1 MB = 8192 kbit
bitrate=$((target_size_kbit / duration))
clear
echo "                                 bitrate : $bitrate"
ffmpeg -y -i "$input_file" -c:v libx264 -b:v "${bitrate}k" -pass 1 -an -f mp4 /dev/null && \
ffmpeg -i "$input_file" -c:v libx264 -b:v "${bitrate}k" -pass 2 -c:a aac -b:a 128k "$output_file"
rm -f ffmpeg2pass-0.log* 

