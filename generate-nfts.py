import os
import subprocess
import argparse
import sys

shader_path = "./shaders"
nfts_path = "./nfts"
output_path = "./nft-videos"

def runVideoProcess(name):

    tmpVideo = output_path + '/' + name + '-temp.mp4'
    video = output_path + '/' + name + '.mp4'
    videoCommand = ' '.join(
            [ './build/bin/VectorFieldParticleSystem'
            , '--config MOTION-record'
            , '--config'
            , nfts_path + '/' + name
            , '--record' 
            , tmpVideo
            , '--screenshot'
            , output_path + '/' + name + '.png'
            ])  
    audioCommand = ' '.join(
        [ 'ffmpeg'
        ,'-i'
        , tmpVideo
        ,'-i audio/noise.wav'
        ,'-map 0:v -map 1:a -c:v copy -shortest'
        , video
        ]
    )
    removeTmpCommand = 'rm ' + tmpVideo

    subprocess.run( videoCommand
                    , stderr=subprocess.STDOUT
                    , shell=True
                    , check=True
                    )
    subprocess.run( audioCommand
                    , stderr=subprocess.STDOUT
                    , shell=True
                    , check=True
                    )
    subprocess.run( removeTmpCommand
                    , stderr=subprocess.STDOUT
                    , shell=True
                    , check=True
                    )



def generate_nfts(start, end):
    print("Generate NFTs in range: %d - %d" % (start, end))
    for x in range(start, end + 1):
        print("Generate NFT: MOTION-%d" % x)
        runVideoProcess('MOTION-' + str(x))


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Generate NFTs in the MOTION NFT Series on Cardano.')
    parser.add_argument('--start', metavar='INT', type=int, default=1,
                        help='start index when generating nfts (inclusive)')
    parser.add_argument('--end', metavar='INT', type=int, default=128,
                        help='end index when generating nfts (inclusive)')
    
    args = parser.parse_args()

    if not(args.start <= args.end):
        sys.exit("Error: --start " + str(args.start) + " was larger then " + "--end " + str(args.end) )
    
    if not(args.start >= 1) :
        sys.exit("Error: --start " + str(args.start) + "must be larger or equal then 1" )

    if not(args.end >= 1) :
        sys.exit("Error: --end " + str(args.end) + "must be larger or equal then 1" )

    if not(args.start <= 128)  :
        sys.exit("Error: --start " + str(args.start) + "must be less then or equal to 128" )

    if not(args.end <= 128)  :
        sys.exit("Error: --end " + str(args.end) + "must be less then or equal to 128" )

    try:
        if not(os.path.isdir(output_path)):
            os.mkdir(output_path)
            print ("Created the directory '%s'" % output_path)
        print ("Start generating the NFTs...")
        generate_nfts(args.start, args.end)
    except OSError:
        print ("Creation of the directory %s failed" % output_path)
