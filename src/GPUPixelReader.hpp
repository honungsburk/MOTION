#ifndef GPU_PIXEL_READER_H
#define GPU_PIXEL_READER_H

#include "../include/glad/glad.h" 
#include <stdlib.h>

/**
    CS-11 Asn 2: Read pixels from the GPU back to the CPU
    @file GPUPixelReader.hpp
    @author Frank Hampus Weslien
*/
class GPUPixelReader{
    int frameNbr;
    unsigned int nbrPBOs;
    unsigned int index = 0;
    unsigned int nextIndex;
    GLuint * pixelbuffers;
    unsigned int width;
    unsigned int height;
    GLenum format;


public:

    GPUPixelReader(unsigned int nbrPBOs, unsigned int width, unsigned int height, GLenum format, unsigned int nbytes){
        this->nbrPBOs = nbrPBOs;
        this->width = width;
        this->height = height;
        this->format = format;
        this->frameNbr = -1 * (int) nbrPBOs;
        pixelbuffers = new unsigned int[nbrPBOs]();
        glGenBuffers(nbrPBOs, pixelbuffers);
        
        for (int i = 0; i < nbrPBOs; ++i) {
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pixelbuffers[i]);
            glBufferData(GL_PIXEL_PACK_BUFFER, nbytes, NULL, GL_STREAM_READ);
        }
        
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }

    ~GPUPixelReader() {
        glDeleteBuffers(nbrPBOs, pixelbuffers);
        delete pixelbuffers;
    }

    /**
        Reads pixels from the GPU.
        @param pixels the reference to which will be populated with a pointer  
        pointing to the pixels returned from the GPU. It will not be assigned such
        a pointer until the return value is non-negative.
        @return the current frame of pixels that "pixels" will point to, it is
        negative until all internal buffer objects are filled
    */
    int readPixels(GLubyte *& pixels){
    
        index = (index + 1) % nbrPBOs;
        nextIndex = (index + 1) % nbrPBOs;
        frameNbr++;

        loadPBO(format, index, width, height);
        if(frameNbr >= 0){
            GLuint buf = pixelbuffers[nextIndex];
            glBindBuffer(GL_PIXEL_PACK_BUFFER, buf);
            pixels = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        }

        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        return frameNbr;
    }

    /**
        Get the number of internal buffers.
        @return the number of internal buffers set by nbrPBOs at initialization.
    */
    unsigned int getNbrPBOs(){
        return nbrPBOs;
    }

private:

    void loadPBO(GLenum format, unsigned int index, unsigned int width, unsigned int height){
        // set the target framebuffer to read
        glReadBuffer(GL_FRONT);

        // read pixels from framebuffer to PBO
        // glReadPixels() should return immediately.
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pixelbuffers[index]);
        glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, 0);
    }

};

#endif