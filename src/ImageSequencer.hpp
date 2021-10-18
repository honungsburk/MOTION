#ifndef IMAGE_SEQUENCER_H
#define IMAGE_SEQUENCER_H

#include "../include/glad/glad.h" 
#include <string>
#include <iostream>
#include <png.h>
#include <stb_image.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

class ImageSequencer{
public:
    unsigned int frameNbr = 0;
    unsigned int index = 1;
    unsigned int nextIndex;
    GLuint * pixelbuffers;
    unsigned int nbrPBOs;
    unsigned int width;
    unsigned int height;

    ImageSequencer(unsigned int nbrPBOs, unsigned int width, unsigned int height){
        this->nbrPBOs = nbrPBOs;
        this->width = width;
        this->height = height;
        pixelbuffers = new unsigned int[nbrPBOs]();
        glGenBuffers(nbrPBOs, pixelbuffers);
        // Since we are using RGBA
        unsigned int nbytes = width * height * 4;
        
        for (int i = 0; i < nbrPBOs; ++i) {
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pixelbuffers[i]);
            glBufferData(GL_PIXEL_PACK_BUFFER, nbytes, NULL, GL_STREAM_READ);
        }
        
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }

    ~ImageSequencer() {
        delete pixelbuffers;
    }

    void screenshot(const char *filename){
        index = (index + 1) % nbrPBOs;
        nextIndex = (index + 1) % nbrPBOs;
        frameNbr++;

        loadPBO(width, height);
        if(frameNbr >= nbrPBOs){
            screenshot_png(filename, width, height);
        }

        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }



private:
    GLubyte *pixels = NULL;
    png_byte *png_bytes = NULL;
    png_byte **png_rows = NULL;

    void loadPBO(unsigned int width, unsigned int height){
        // set the target framebuffer to read
        glReadBuffer(GL_FRONT);

        // read pixels from framebuffer to PBO
        // glReadPixels() should return immediately.
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pixelbuffers[index]);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0); // GL_RGBA is slow use GL_BGRA instead????
    }

    void screenshot_png(const char *filename, unsigned int width, unsigned int height) {
        size_t i, nvals;
        const size_t format_nchannels = 4;
        FILE *f = fopen(filename, "wb");
        nvals = format_nchannels * width * height;
        //pixels = (GLubyte *) realloc(pixels, nvals * sizeof(GLubyte));
        png_bytes = (png_byte *) realloc(png_bytes, nvals * sizeof(png_byte));
        png_rows = (png_byte **) realloc(png_rows, height * sizeof(png_byte*));

        // map the PBO to process its data by CPU
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pixelbuffers[nextIndex]);
        pixels = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

        if(!pixels){
            std::cout << "Could not map pixels to client memory!\n";
        }
        for (i = 0; i < nvals; i++)
            png_bytes[i] = pixels[i];
        for (i = 0; i < height; i++)
            png_rows[height - i - 1] = &png_bytes[i * width * format_nchannels];
        png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png) abort();
        png_infop info = png_create_info_struct(png);
        if (!info) abort();
        if (setjmp(png_jmpbuf(png))) abort();
        png_init_io(png, f);
        png_set_IHDR(
            png,
            info,
            width,
            height,
            8,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
        );
        png_write_info(png, info);
        png_write_image(png, png_rows);
        png_write_end(png, NULL);
        png_destroy_write_struct(&png, &info);

        fclose(f);
    }

};

#endif