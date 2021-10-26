# MOTION

MOTION is a generative art project by Frank Hampus Weslien on the Cardano blockchain.
It explores the use of particle simulation in a 2D vector fields to create a 
program for rapid creation artworks. 

## Installation

### Install c++, conan, and cmake

Make sure that you have c++ installed. 
Conan is a package manager for c++ that you can install via their [website](https://conan.io/)


### Change compiler version
Conan uses by default a backwards compatible version of c++ that doesn't work with the `boost` library
so we have to change that by using changing to line

```txt
compiler.libcxx=libstdc++
```

to

```txt
compiler.libcxx=libstdc++11
```
in `~/.conan/profiles/default`

### install program dependencies

Run  `./build-debug.sh` after installing conan.

#### Linux

Conan will most likely complain about these packages missing:

* libgl-dev
* libva-dev
* libvdpau-dev

You will need to install them system wide. On ubuntu use `sudo apt-get install libgl-dev libva-dev libvdpau-dev`




### Warning

The program has only been compiled and used on ubuntu 20.04. If you try to compile
on another platform (most notably Windows or Mac) it might not work...

## Compiling steps

You find all the compiling steps in `./build-debug.sh`. Either execute them one by
one or run the script directly. 

## Running the program

When the program is compiled you view one of the nfts like so:

```bash
./build/bin/VectorFieldParticleSystem --config ./nfts/MOTION-3
```

to generate videos like so:


```bash
./build/bin/VectorFieldParticleSystem --config ./nfts/MOTION-3 --record MOTION-3.mp4 --length 30 --fps 30 --preset medium --crf 30 --pixels-per-ratio 120
```
If you want to change any parameter (such as increasing the resolution) check out
the `--help` command.

```bash
./build/bin/VectorFieldParticleSystem --help
```

Note that any argument given on the terminal will have priority over what is specified in the config.

## Generating a the NFT collection

If you want to generate the entire NFT collection from scratch I'd advice you to use
the command `python3 generate-nfts.py`. (Warning: rendering all videos will take a few hours)

The file depends on three things.

1. That you have a python3 installation
2. ffmpeg
3. The 'rm' command available on linux


## Creating new simulations

Colors, particle size, and etc can easily be mixed and matched checkout the 
`--help` command for all available options. If you want to create your own vector fields
you unfortunately have to modify the source code and recompile.
Look for the function called `createVectorFieldFn(..)`
there you will find a ridiculously large switch statement to which you can add new functions. 


## FAQ

## What the hell is width-ratio, height-ratio, pixels-per-ratio, and vector-per-ratio?

It just a way to easily change the resolution and underlying vector field. 

if you use `--width-ratio 16` and `--height-ratio 9` then `--pixels-per-ratio 80` will be 1280px by 720px and
`--pixels-per-ratio 120` will be 1920px by 1080px.  

## Why is the simulation on using the bottom left part of my screen?

You can not change the resolution of the simulation when it is created but the 
window that is displaying it can still be resized. If you use a higher resolution for the simulation
you will be able to fit the entire screen. 

## Why can I not create images/videos larger then my screen resolution?

I didn't see the point in someone create videos with higher resolution then their
own screen can handle so I didn't bother implementing off-screen rendering. 
So yes, if you want to render 4k video you need a 4k monitor.


### Help the screen is black?!?!

You probably specified the `--shader-path` option incorrectly.
