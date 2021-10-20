
#ifndef VIDEO_CAPTURE_H
#define VIDEO_CAPTURE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/glad/glad.h" 
#include "finite_math.hpp"
#include <stdexcept>
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
    #include <libavformat/avformat.h>
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

        codex_ctx = avcodec_alloc_context3(codec);
        if (!codex_ctx) {
            fprintf(stderr, "Could not allocate video codec context\n");
            exit(1);
        }

        pkt = av_packet_alloc();
        if (!pkt)
            exit(1);

        /* put sample parameters */
        codex_ctx->bit_rate = bitrate;
        /* resolution must be a multiple of two */
        if(width % 2 != 0)
            throw std::invalid_argument( "The width must be devisible by two" );

        if(height % 2 != 0)
            throw std::invalid_argument( "The height must be devisible by two" );

        codex_ctx->width = width;
        codex_ctx->height = height;
        /* frames per second */
        codex_ctx->time_base = (AVRational){1, framerate};
        codex_ctx->framerate = (AVRational){framerate, 1};

        /* emit one intra frame every ten frames
        * check frame pict_type before passing frame
        * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
        * then gop_size is ignored and the output of encoder
        * will always be I frame irrespective to gop_size
        */
        codex_ctx->gop_size = 10;
        codex_ctx->max_b_frames = 1;
        codex_ctx->pix_fmt = AV_PIX_FMT_YUV420P; // AV_PIX_FMT_RGB8

        if (codec->id == AV_CODEC_ID_H264)
            av_opt_set(codex_ctx->priv_data, "preset", "slow", 0);

        /* open it */
        ret = avcodec_open2(codex_ctx, codec, NULL);
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
        frame->format = codex_ctx->pix_fmt;
        frame->width  = codex_ctx->width;
        frame->height = codex_ctx->height;

        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Could not allocate the video frame data\n");
            exit(1);
        }

        frame_order = 1; 
        sws = sws_getContext( codex_ctx->width
                            , codex_ctx->height
                            , AV_PIX_FMT_RGB32
                            , codex_ctx->width
                            , codex_ctx->height
                            , AV_PIX_FMT_YUV420P
                            , SWS_FAST_BILINEAR // Change this???
                            , 0, 0, 0);


        // Init Muxing
        avFormatContext = avformat_alloc_context();
        avStream = avformat_new_stream(avFormatContext, codec);
        ret = avformat_write_header(avFormatContext, &avDict);
        if(ret < 0){
            fprintf(stderr, "Could not write header to the output media file\n");
            exit(1);
        }
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
        if (ret < 0){
            fprintf(stderr, "Could not make the frame writable\n");
            exit(1); // Wait... you should throw error instead!
        }

        size_t nvals = 4 * codex_ctx->width * codex_ctx->height; //GL_BGRA
        pixels = (GLubyte *) realloc(pixels, nvals * sizeof(GLubyte)); // I don't think I need to do this every time since the size is constant
        glReadPixels(0, 0, codex_ctx->width, codex_ctx->height, GL_BGRA, GL_UNSIGNED_BYTE, pixels);

        // CONVERT TO YUV AND ENCODE
        ret =  av_image_alloc(frame->data, frame->linesize, codex_ctx->width, codex_ctx->height, AV_PIX_FMT_YUV420P, 32);
        if (ret < 0){
            fprintf(stderr, "Could not allocate the image\n");
            exit(1); // Wait... you should throw error instead!
        }
        // Compensate for OpenGL y-axis pointing upwards and ffmpeg y-axis pointing downwards        
        uint8_t *in_data[1] = {(uint8_t *) pixels + (codex_ctx->height-1)*codex_ctx->width*4}; // address of the last line
        int in_linesize[1] = {- codex_ctx->width * 4}; // negative stride

        // uint8_t *in_data[1] = {(uint8_t *) pixels};
        // int in_linesize[1] = { 4*codex_ctx->width }; // RGBA stride
        sws_scale(sws, in_data, in_linesize, 0, codex_ctx->height, frame->data, frame->linesize);

        frame->pts = frame_order;
        frame_order++;

        /* encode the image */
        encode(codex_ctx, frame, pkt, f);
    }

    void close(){
        /* flush the encoder */
        encode(codex_ctx, NULL, pkt, f);

        /* Add sequence end code to have a real MPEG file.
        It makes only sense because this tiny examples writes packets
        directly. This is called "elementary stream" and only works for some
        codecs. To create a valid file, you usually need to write packets
        into a proper file format or protocol; see muxing.c.
        */
        if (codec->id == AV_CODEC_ID_MPEG1VIDEO || codec->id == AV_CODEC_ID_MPEG2VIDEO)
            fwrite(endcode, 1, sizeof(endcode), f);
        fclose(f);

        //av_freep(&frame->data[0]);
        avcodec_free_context(&codex_ctx);
        av_frame_free(&frame);
        av_packet_free(&pkt);
        sws_freeContext(sws);
    }


private:
    // Muxing
    AVFormatContext* avFormatContext = NULL;
    AVStream* avStream;
    //TODO: fill with AVFormatContext values???
    AVDictionary *avDict = NULL; // "create" an empty dictionary

    // Raw Stream
    GLubyte *pixels = NULL;
    struct SwsContext *sws;
    const AVCodec *codec;
    AVCodecContext *codex_ctx= NULL;
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
            av_write_frame(avFormatContext, pkt);
            //fwrite(pkt->data, 1, pkt->size, outfile);
            av_packet_unref(pkt);
        }
    }
};
#endif