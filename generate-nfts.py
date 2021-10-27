import os
import subprocess
import argparse
import sys

shader_path = "./shaders"
nfts_path = "./nfts"
output_path = "./nft-videos"

def runVideoProcess(name, shouldScreenshot, shouldVideoCapture):

    tmpVideo = output_path + '/' + name + '-raw.mp4'
    video = output_path + '/' + name + '.mp4'

    length = '--length 30'
    if name == 'MOTION-3':
        length = '--length 59'

    screenshotCommand = ' '.join(
            [ './build/bin/VectorFieldParticleSystem'
            , '--config'
            , nfts_path + '/' + name
            , '--screenshot'
            , output_path + '/' + name + '.png'
            , '--screenshot-delay 10'
            , '--length 11'
            , '--fps 30'
            , '--pixels-per-ratio 120'
            ])
    videoRawCommand = ' '.join(
            [ './build/bin/VectorFieldParticleSystem'
            , '--config'
            , nfts_path + '/' + name
            , '--record' 
            , tmpVideo
            , length
            , '--fps 30'
            , '--preset ultrafast'
            , '--crf 18'
            , '--pixels-per-ratio 120'
            ])
    firstPassCommand = ' '.join(
        [ 'ffmpeg -y -i'
        , tmpVideo
        , '-c:v libx264 -b:v 22000k -pass 1 -an -f null /dev/null'
        ]
    )
    secondPassCommand = ' '.join(
        [ 'ffmpeg' 
        , '-i'
        , tmpVideo
        , '-i audio/noise.wav -shortest'
        , '-c:v libx264 -b:v 22000k -pass 2'
        , video
        ]
    )
    removeTmpCommand = 'rm ' + tmpVideo

    # This was made as a separate call because otherwise it fails 
    # to save the entire image
    if shouldScreenshot:
        subprocess.run( screenshotCommand
                        , stderr=subprocess.STDOUT
                        , shell=True
                        , check=True
                        )
    if shouldVideoCapture:
        subprocess.run( videoRawCommand
                        , stderr=subprocess.STDOUT
                        , shell=True
                        , check=True
                        )
        subprocess.run( firstPassCommand
                        , stderr=subprocess.STDOUT
                        , shell=True
                        , check=True
                        )
        subprocess.run( secondPassCommand
                        , stderr=subprocess.STDOUT
                        , shell=True
                        , check=True
                        )
        subprocess.run( removeTmpCommand
                        , stderr=subprocess.STDOUT
                        , shell=True
                        , check=True
                        )



def generate_nfts(start, end, shouldScreenshot, shouldVideoCapture):
    print("Generate NFTs in range: %d - %d" % (start, end))
    for x in range(start, end + 1):
        print("Generate NFT: MOTION-%d" % x)
        runVideoProcess('MOTION-' + str(x), shouldScreenshot, shouldVideoCapture)


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Generate NFTs in the MOTION NFT Series on Cardano.')
    parser.add_argument('--start', metavar='INT', type=int, default=1,
                        help='start index when generating nfts (inclusive)')
    parser.add_argument('--end', metavar='INT', type=int, default=128,
                        help='end index when generating nfts (inclusive)')
    parser.add_argument('--video', action='store_true',
                        help='If present you will render video')
    parser.add_argument('--screenshot', action='store_true',
                        help='If present you will take screenshots')
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
        generate_nfts(args.start, args.end, args.screenshot, args.video)
    except OSError:
        print ("Creation of the directory %s failed" % output_path)
