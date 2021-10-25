# Motion


## Installation

Most dependencies are installed by conan but some need to exist on your system:

libva-dev
libswscale-dev

## Compiling steps

You find all the compiling steps in `./build-debug.sh`. Either execute them one by
one or run the script directly. When the program is compiled you can use it
to generate videos like so:


```bash
./build/bin/VectorFieldParticleSystem --config ./configs/config-122 \
    --record example.mp4 \
    --length 10 \
    --fps 30 \
    --preset medium \
    --crf 33 \
    --pixels-per-ratio 120 \
    --screenshot example.png \
    --screenshot-delay 3
```

to record a 10 second video and take a screenshot after 3 seconds using the simulation parameters specified in `config-122`.

## 