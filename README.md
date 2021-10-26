# Motion

## Installation

Most dependencies are installed by conan but some need to exist on your system:

libva-dev
libswscale-dev

### Warning

The program has only been compiled and used on ubuntu 20.04. If you try to compile
on another platform (most notably Windows or Mac) you might not work...

## Compiling steps

You find all the compiling steps in `./build-debug.sh`. Either execute them one by
one or run the script directly. 

## Running the program

When the program is compiled you can use it
to generate videos like so:


```bash
./build/bin/VectorFieldParticleSystem --config ./nfts/MOTION-122 --config MOTION-record --record example.mp4 --screenshot example.png
```

to generate the same video and thumbnail as used in the MOTION-122 nft.

If you want to change any parameter (such as increasing the resolution) check out
the `--help` command.

```bash
./build/bin/VectorFieldParticleSystem --help
```

## Debug

### Help the screen is black?!?!

You probably specified the `--shader-path` option incorrectly.
