#ifndef GPU_PIXEL_READER_H
#define GPU_PIXEL_READER_H

#include "../include/glad/glad.h" 
#include <stdlib.h>

/**
 * Reads Pixels from the GPU in a slightly faster way using Pixel Buffer Objects.
 */
class GPUPixelReader{
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
        delete pixelbuffers;
    }

    /**
     * 
     */
    int readPixels(GLubyte *pixels){
    
        index = (index + 1) % nbrPBOs;
        nextIndex = (index + 1) % nbrPBOs;
        frameNbr++;

        loadPBO(format, index, width, height);
        if(frameNbr >= 0){
            // map the PBO to process its data by CPU
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pixelbuffers[nextIndex]);
            pixels = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        }

        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        return frameNbr;
    }

    unsigned int getNbrPBOs(){
        return nbrPBOs;
    }

private:
    int frameNbr;
    unsigned int nbrPBOs;
    unsigned int index = 0;
    unsigned int nextIndex;
    GLuint * pixelbuffers;
    unsigned int width;
    unsigned int height;
    GLenum format;

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