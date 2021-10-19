# Motion


## Installation

Most dependencies are installed by conan but some need to exist on your system:

libva-dev
libswscale-dev

## Compiling steps

```bash
./build-debug.sh
```

## FFMPEG

### Perfect-Loop
```bash
ffmpeg -framerate 30 \
    -i screens/Background-1000.png \
    -framerate 30 \
    -pattern_type glob -i 'images/increase/*.png' \
    -framerate 30 \
    -pattern_type glob -i 'images/decrease/*.png' \
    -i audio/50-White-Noise-10min.mp3 \
    -filter_complex "[0][1] overlay[out],[out][2] overlay" \
    -c:a copy -shortest -c:v libx264 -pix_fmt yuv420p loop.mp4 
```

### Non-Looping
```bash
ffmpeg -framerate 30 \
    -pattern_type glob -i 'images/*.png' \
    -i audio/50-White-Noise-10min.mp3 \
    -preset slow -c:a copy -shortest -c:v libx264 -pix_fmt yuv420p non-loop.mp4 

```

# Nvidea Hardware encoder
https://docs.nvidia.com/video-technologies/video-codec-sdk/nvenc-video-encoder-api-prog-guide/
https://stackoverflow.com/questions/13857400/encoding-fbo-textures-to-h-264-video-directly-on-gpu
https://stackoverflow.com/questions/49862610/opengl-to-ffmpeg-encode