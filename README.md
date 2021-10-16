# Motion


## Installation

Most dependencies are installed by conan but some need to exist on your system:

libva-dev


## Compiling steps

```bash
./build-debug.sh
```

## FFMPEG

```bash
ffmpeg -framerate 30 \
    -pattern_type glob -i 'images/increase/*.png' \
    -framerate 30 \
    -i screens/Background-1000.png \
    -i audio/50-White-Noise-10min.mp3 \
    -filter_complex "[1:v][0:v] overlay" \
    -preset slow -c:a copy -shortest -c:v libx264 -pix_fmt yuv420p test.mp4 

ffmpeg -framerate 30 \
    -pattern_type glob -i 'images/decrease/*.png' \
    -i test.mp4 \
    -filter_complex "[1:v][0:v] overlay" \
    -preset slow -c:a copy -shortest -c:v libx264 -pix_fmt yuv420p test2.mp4 

ffmpeg -i screens/Background-1000.png \
    -framerate 30 \
    -pattern_type glob -i 'images/increase/*.png' \
    -framerate 30 \
    -pattern_type glob -i 'images/decrease/*.png' \
    -i audio/50-White-Noise-10min.mp3 \
    -filter_complex "[0:v][1:v][2:v] overlay" \
    -c:a copy -shortest -c:v libx264 -pix_fmt yuvj420p test.mp4 
```

non-looping video
```bash
ffmpeg -framerate 60 \
    -pattern_type glob -i 'images/*.png' \
    -i audio/50-White-Noise-10min.mp3 \
    -preset slow -c:a copy -shortest -c:v libx264 -pix_fmt yuv420p black-cream.mp4 

```

