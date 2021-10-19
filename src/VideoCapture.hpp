
#ifndef VIDEO_CAPTURE_H
#define VIDEO_CAPTURE_H
/**
 * @file
 * video encoding with libavcodec API example
 *
 * @example encode_video.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/glad/glad.h" 
#include "finite_math.hpp"
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
}

class VideoCapture
{
public:

    VideoCapture(const char *filename, unsigned int width, unsigned int height, int framerate, unsigned int bitrate){

    /* find the mpeg1video encoder */
        codec = avcodec_find_encoder(AV_CODEC_ID_H264);
        // codec = avcodec_find_encoder_by_name(codec_name);
        if (!codec) {
            fprintf(stderr, "Codec '%d' not found\n", AV_CODEC_ID_H264);
            exit(1);
        }

        c = avcodec_alloc_context3(codec);
        if (!c) {
            fprintf(stderr, "Could not allocate video codec context\n");
            exit(1);
        }

        pkt = av_packet_alloc();
        if (!pkt)
            exit(1);

        /* put sample parameters */
        c->bit_rate = bitrate;
        /* resolution must be a multiple of two */
        c->width = width;
        c->height = height;
        /* frames per second */
        c->time_base = (AVRational){1, framerate};
        c->framerate = (AVRational){framerate, 1};

        /* emit one intra frame every ten frames
        * check frame pict_type before passing frame
        * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
        * then gop_size is ignored and the output of encoder
        * will always be I frame irrespective to gop_size
        */
        c->gop_size = 10;
        c->max_b_frames = 1;
        c->pix_fmt = AV_PIX_FMT_YUV420P; // AV_PIX_FMT_RGB8

        if (codec->id == AV_CODEC_ID_H264)
            av_opt_set(c->priv_data, "preset", "slow", 0);

        /* open it */
        ret = avcodec_open2(c, codec, NULL);
        if (ret < 0) {
            fprintf(stderr, "Could not open codec: %d\n", ret);
            exit(1);
        }

        f = fopen(filename, "wb");
        if (!f) {
            fprintf(stderr, "Could not open %s\n", filename);
            exit(1);
        }

        frame = av_frame_alloc();
        if (!frame) {
            fprintf(stderr, "Could not allocate video frame\n");
            exit(1);
        }
        frame->format = c->pix_fmt;
        frame->width  = c->width;
        frame->height = c->height;

        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Could not allocate the video frame data\n");
            exit(1);
        }

        frame_order = 1;
        sws = sws_getContext(c->width, c->height, AV_PIX_FMT_RGB32, c->width, c->height, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, 0, 0, 0);
    }

    void addFrame(){
        fflush(stdout);

        /* Make sure the frame data is writable.
           On the first round, the frame is fresh from av_frame_get_buffer()
           and therefore we know it is writable.
           But on the next rounds, encode() will have called
           avcodec_send_frame(), and the codec may have kept a reference to
           the frame in its internal structures, that makes the frame
           unwritable.
           av_frame_make_writable() checks that and allocates a new buffer
           for the frame only if necessary.
         */
        ret = av_frame_make_writable(frame);
        if (ret < 0)
            exit(1); // Wait... you should throw error instead!

        size_t nvals = 4 * c->width * c->height; //GL_BGRA
        pixels = (GLubyte *) realloc(pixels, nvals * sizeof(GLubyte));
        glReadPixels(0, 0, c->width, c->height, GL_BGRA, GL_UNSIGNED_BYTE, pixels);

        // CONVERT TO YUV AND ENCODE
        // int frame_size = avpicture_get_size(AV_PIX_FMT_YUV420P, c->width, c->height);
        // c->frame_buffer = malloc(frame_size);

        //avpicture_fill((AVPicture *) frame, (uint8_t *) c->frame_buffer, AV_PIX_FMT_YUV420P, c->width, c->height);
        
        uint8_t *in_data[1] = {(uint8_t *) pixels};
        int in_linesize[1] = { 4*c->width }; // RGBA stride
        sws_scale(sws, in_data, in_linesize, 0, c->height, frame->data, frame->linesize);

        frame->pts = frame_order;
        frame_order++;

        /* encode the image */
        encode(c, frame, pkt, f);
    }

    void close(){
        /* flush the encoder */
        encode(c, NULL, pkt, f);

        /* Add sequence end code to have a real MPEG file.
        It makes only sense because this tiny examples writes packets
        directly. This is called "elementary stream" and only works for some
        codecs. To create a valid file, you usually need to write packets
        into a proper file format or protocol; see muxing.c.
        */
        if (codec->id == AV_CODEC_ID_MPEG1VIDEO || codec->id == AV_CODEC_ID_MPEG2VIDEO)
            fwrite(endcode, 1, sizeof(endcode), f);
        fclose(f);

        avcodec_free_context(&c);
        av_frame_free(&frame);
        av_packet_free(&pkt);
    }


private:
    GLubyte *pixels = NULL;
    struct SwsContext *sws;

    const AVCodec *codec;
    AVCodecContext *c= NULL;
    int frame_order, ret;
    FILE *f;
    AVFrame *frame;
    AVPacket *pkt;
    uint8_t endcode[4] = { 0, 0, 1, 0xb7 };


void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,
                   FILE *outfile)
    {
        int ret;

        /* send the frame to the encoder */
        if (frame)
            printf("Send frame %3" PRId64 "\n", frame->pts);

        ret = avcodec_send_frame(enc_ctx, frame);
        if (ret < 0) {
            fprintf(stderr, "Error sending a frame for encoding\n");
            exit(1);
        }

        while (ret >= 0) {
            ret = avcodec_receive_packet(enc_ctx, pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                return;
            else if (ret < 0) {
                fprintf(stderr, "Error during encoding\n");
                exit(1);
            }

            printf("Write packet %3" PRId64 " (size=%5d)\n", pkt->pts, pkt->size);
            fwrite(pkt->data, 1, pkt->size, outfile);
            av_packet_unref(pkt);
        }
    }
};
#endif