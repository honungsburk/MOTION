
#ifndef VIDEO_CAPTURE2_H
#define VIDEO_CAPTURE2_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/glad/glad.h" 
#include "FiniteMathPatch.hpp"
#include <stdexcept>
#include "GPUPixelReader.hpp"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
    #include <libavformat/avformat.h>
    #include <libavutil/opt.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/timestamp.h>
}


// These exist to patch three functions for which gcc gets compiler errors
#ifdef av_err2str
#undef av_err2str
#include <string>
av_always_inline std::string av_err2string(int errnum) {
    char str[AV_ERROR_MAX_STRING_SIZE];
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}
#define av_err2str(err) av_err2string(err).c_str()
#endif


#ifdef av_ts2str
#undef av_ts2str
#include <string>
av_always_inline std::string av_ts2string(int ts) {
    char str[AV_TS_MAX_STRING_SIZE];
    return av_ts_make_string(str, ts);
}
#define av_ts2str(ts) av_ts2string(ts).c_str()
#endif

#ifdef av_ts2timestr
#undef av_ts2timestr
#include <string>
av_always_inline std::string av_ts2timestring(int ts, AVRational *tb) {
    char str[AV_TS_MAX_STRING_SIZE];
    return av_ts_make_time_string(str, ts, tb);
}
#define av_ts2timestr(ts, tb) av_ts2timestring(ts, tb).c_str()
#endif

/**
    CS-11 Asn 2: Encode video from OpenGL
    @file VideoCapture.hpp
    @author Frank Hampus Weslien
*/
class VideoCapture
{
public:

    VideoCapture( const char *filename
                , unsigned int width
                , unsigned int height
                , int framerate
                , unsigned int bitrate
                , unsigned int crf
                , std::string preset
                , std::string tune
                ): gpuPixelReader(3, width, height, GL_BGRA, width * height * 4){

        avformat_alloc_output_context2(&avFormatContext, NULL, NULL, filename);
        if (!avFormatContext) {
            printf("Could not deduce output format from file extension: using MPEG.\n");
            avformat_alloc_output_context2(&avFormatContext, NULL, "mpeg", filename);
        }
        if (!avFormatContext)
            throw std::runtime_error("Could not allocate a formatting context");

        avOutputFormat = avFormatContext->oformat;

        // Video Stream

        /* find the encoder */
        AVCodecID codec_id = AV_CODEC_ID_H264;
        codec = avcodec_find_encoder(codec_id);
        if (!codec) {
            char buffer[50];
            sprintf (buffer, "Could not find encoder for '%s'\n", avcodec_get_name(codec_id));
            throw std::invalid_argument(buffer);
        }

        pkt = av_packet_alloc();
        if (!pkt) 
            throw std::runtime_error("Could not allocate AVPacket");

        avStream = avformat_new_stream(avFormatContext, NULL);
        if (!avStream) 
            throw std::runtime_error("Could not allocate stream");

        avStream->id = avFormatContext->nb_streams-1;
        codec_ctx = avcodec_alloc_context3(codec);
        if (!codec_ctx)
            throw std::runtime_error("Could not alloc an encoding context");


        codec_ctx->codec_id = codec_id;
        codec_ctx->bit_rate = bitrate;

        /* resolution must be a multiple of two */
        if(width % 2 != 0){
            char buffer[50];
            sprintf(buffer, "The width must be devisible by two but was '%d'", width);
            throw std::invalid_argument(buffer);
        }
        codec_ctx->width = width;

        if(height % 2 != 0){
            char buffer[50];
            sprintf(buffer, "The height must be devisible by two but was '%d'", height);
            throw std::invalid_argument(buffer);
        }

        codec_ctx->height = height;


        /* frames per second */
        codec_ctx->framerate = (AVRational){framerate, 1};

        /* timebase: This is the fundamental unit of time (in seconds) in terms
        * of which frame timestamps are represented. For fixed-fps content,
        * timebase should be 1/framerate and timestamp increments should be
        * identical to 1. */
        avStream->time_base = (AVRational){ 1, framerate };
        codec_ctx->time_base       = avStream->time_base;

        codec_ctx->gop_size      = 10; /* emit one intra frame every twelve frames at most */
        codec_ctx->pix_fmt       = AV_PIX_FMT_YUV420P;

        av_dict_set_int(&avDict, "crf", crf, 0);
        av_dict_set(&avDict, "preset", preset.c_str(), 0);
        av_dict_set(&avDict, "tune", tune.c_str(), 0);

        /* Some formats want stream headers to be separate. */
        if (avOutputFormat->flags & AVFMT_GLOBALHEADER)
            codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;



        /* open the codec */
        AVDictionary *opt = NULL;
        av_dict_copy(&opt, avDict, 0);
        ret = avcodec_open2(codec_ctx, codec, &opt);
        av_dict_free(&opt);
        if (ret < 0) {
            char buffer[50];
            sprintf(buffer, "Could not open video codec: %s\n", av_err2str(ret));
            throw std::invalid_argument(buffer);
        }
        av_opt_set(codec_ctx->priv_data, "tune", "zerolatency", 0);

        frame = alloc_frame(codec_ctx->pix_fmt, codec_ctx->width, codec_ctx->height);
        if (!frame)
            throw std::runtime_error("Could not allocate video frame");
        

        /* copy the stream parameters to the muxer */
        ret = avcodec_parameters_from_context(avStream->codecpar, codec_ctx);
        if (ret < 0)
            throw std::runtime_error("Could not copy the stream parameters");

        // Color fromat COnversion

        sws = sws_getContext( codec_ctx->width
                            , codec_ctx->height
                            , AV_PIX_FMT_RGB32
                            , codec_ctx->width
                            , codec_ctx->height
                            , AV_PIX_FMT_YUV420P
                            , SWS_FAST_BILINEAR // Change this???
                            , 0, 0, 0);

        
        // Check output file
        av_dump_format(avFormatContext, 0, filename, 1);

        /* open the output file, if needed */
        if (!(avOutputFormat->flags & AVFMT_NOFILE)) {
            ret = avio_open(&avFormatContext->pb, filename, AVIO_FLAG_WRITE);
            if (ret < 0) {
                char buffer[50];
                sprintf(buffer, "Could not open '%s': %s\n", filename, av_err2str(ret));
                throw std::invalid_argument(buffer);
            }
        }
        /* Write the stream header, if any. */
        ret = avformat_write_header(avFormatContext, &avDict);
        if (ret < 0){
                char buffer[50];
                sprintf(buffer,  "Error occurred when opening output file: %s\n", av_err2str(ret));
                throw std::invalid_argument(buffer);
        } 

    }


    /**
     * Write the current frame in OpenGl to the video stream
     */
    void recordFrame(){
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

        int frameNbr = gpuPixelReader.readPixels(pixels);

        if(frameNbr >= 0) {
            // CONVERT TO YUV AND ENCODE
            ret =  av_image_alloc(frame->data, frame->linesize, codec_ctx->width, codec_ctx->height, AV_PIX_FMT_YUV420P, 32);
            if (ret < 0){
                fprintf(stderr, "Could not allocate the image\n");
                exit(1); // Wait... you should throw error instead!
            }

            // Compensate for OpenGL y-axis pointing upwards and ffmpeg y-axis pointing downwards        
            uint8_t *in_data[1] = {(uint8_t *) pixels + (codec_ctx->height-1)*codec_ctx->width*4}; // address of the last line
            int in_linesize[1] = {- codec_ctx->width * 4}; // negative stride

            sws_scale(sws, in_data, in_linesize, 0, codec_ctx->height, frame->data, frame->linesize);

            frame->pts = frameNbr;

            /* encode the image */
            write_frame(avFormatContext, codec_ctx, avStream, frame, pkt);
        }
    }

    void close()
    {
        // Since the gpuPixelReader is always a few frames behind we have to flush
        // make sure that we get all the frames
        for(int i = 0; i < gpuPixelReader.getNbrPBOs(); i++){
            recordFrame();
        }

        write_frame(avFormatContext, codec_ctx, avStream, NULL, pkt);

        av_write_trailer(avFormatContext);

        avcodec_free_context(&codec_ctx);
        av_frame_free(&frame);
        sws_freeContext(sws);
        if (!(avFormatContext->oformat->flags & AVFMT_NOFILE))
            /* Close the output file. */
            avio_closep(&avFormatContext->pb);

        avformat_free_context(avFormatContext);

    }

private:

    GPUPixelReader gpuPixelReader;

    AVOutputFormat *avOutputFormat;
    AVFormatContext* avFormatContext = NULL;
    AVStream* avStream;
    AVDictionary *avDict = NULL; // "create" an empty dictionary

    GLubyte *pixels = NULL;
    struct SwsContext *sws;
    const AVCodec *codec;
    AVCodecContext *codec_ctx= NULL;

    // Should be ref counted??? https://ffmpeg.org/doxygen/3.3/group__lavc__encdec.html
    AVFrame *frame;
    AVPacket *pkt;
   
    // Return code from opengl functions, just to avoid having to declare it all the time
    int ret;


    int write_frame(AVFormatContext *fmt_ctx, AVCodecContext *c,
                        AVStream *st, AVFrame *frame, AVPacket *pkt)
    {
        // send the frame to the encoder
        ret = avcodec_send_frame(c, frame);
        if (ret < 0) {
            fprintf(stderr, "Error sending a frame to the encoder: %s\n",
                    av_err2str(ret));
            exit(1);
        }

        while (ret >= 0) {
            ret = avcodec_receive_packet(c, pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            else if (ret < 0) {
                fprintf(stderr, "Error encoding a frame: %s\n", av_err2str(ret));
                exit(1);
            }

            /* rescale output packet timestamp values from codec to stream timebase */
            av_packet_rescale_ts(pkt, c->time_base, st->time_base);
            pkt->stream_index = st->index;

            /* Write the compressed frame to the media file. */
            log_packet(fmt_ctx, pkt);
            ret = av_interleaved_write_frame(fmt_ctx, pkt);
            /* pkt is now blank (av_interleaved_write_frame() takes ownership of
            * its contents and resets pkt), so that no unreferencing is necessary.
            * This would be different if one used av_write_frame(). */
            if (ret < 0) {
                fprintf(stderr, "Error while writing output packet: %s\n", av_err2str(ret));
                exit(1);
            }
        }

        return ret == AVERROR_EOF ? 1 : 0;
    }

    void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
    {
        AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

        printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
            av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
            av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
            av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
            pkt->stream_index);
    }

    AVFrame *alloc_frame(enum AVPixelFormat pix_fmt, int width, int height)
    {
        AVFrame *frame;

        frame = av_frame_alloc();
        if (!frame)
            return NULL;

        frame->format = pix_fmt;
        frame->width  = width;
        frame->height = height;

        /* allocate the buffers for the frame data */
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Could not allocate frame data.\n");
            exit(1);
        }

        return frame;
    }


};
#endif