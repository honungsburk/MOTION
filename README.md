# Motion


## Installation

Most dependencies are installed by conan but some need to exist on your system:

libva-dev
libswscale-dev

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