# Motion

## Installation

Most dependencies are installed by conan but some need to exist on your system:

libva-dev
libswscale-dev

### Warning

The program has only been compiled and used on ubuntu 20.04. If you try to compile
on another platform (most notably Windows or Mac) it might not work...

## Compiling steps

You find all the compiling steps in `./build-debug.sh`. Either execute them one by
one or run the script directly. 

## Running the program

When the program is compiled you can use it
to generate videos like so:


```bash
./build/bin/VectorFieldParticleSystem --config MOTION-record --config ./nfts/MOTION-1 --record example.mp4 --screenshot example.png
```

to generate the same video and thumbnail as used in the MOTION-1 nft.

If you want to change any parameter (such as increasing the resolution) check out
the `--help` command.

```bash
./build/bin/VectorFieldParticleSystem --help
```

## Generating a the NFT collection

If you want to generate the entire NFT collection from scratch I'd advice you to use
the command `python3 generate-nfts.py`. 

The file depends on three things.

1. That you have a python3 installation
2. ffmpeg
3. The 'rm' command available on linux


## FAQ

## Why can I not create images/videos larger then my screen resolution?

I didn't see the point in someone create videos with higher resolution then their
own screen can handle so I didn't bother implementing off-screen rendering. 
So yes, if you want to render 4k video you need a 4k monitor.


### Help the screen is black?!?!

You probably specified the `--shader-path` option incorrectly.
