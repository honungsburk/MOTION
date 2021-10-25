#ifndef SNAPSHOT_H
#define SNAPSHOT_H

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


/**
    CS-11 Asn 2: Take screenshoots
    @file ScreenShooter.hpp
    @author Frank Hampus Weslien
*/
class ScreenShooter{
private:
    GLubyte *pixels = NULL;
    png_byte *png_bytes = NULL;
    png_byte **png_rows = NULL;


public:

    /**
     * Take screenshot of what is currently displayed in the window.
     * 
     * @param filename were to store the file
     * @param width the width both of the screen to be screenshoted and the resulting image
     * @param height the height both of the screen to be screenshoted and the resulting image
     * @return
     */
    void screenshot(const char *filename, unsigned int width, unsigned int height) {
        size_t i, nvals;
        const size_t format_nchannels = 4;
        FILE *f = fopen(filename, "wb");
        nvals = format_nchannels * width * height;
        pixels = (GLubyte *) realloc(pixels, nvals * sizeof(GLubyte));
        png_bytes = (png_byte *) realloc(png_bytes, nvals * sizeof(png_byte));
        png_rows = (png_byte **) realloc(png_rows, height * sizeof(png_byte*));

        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

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