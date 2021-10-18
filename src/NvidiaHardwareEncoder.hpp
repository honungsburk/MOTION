#ifndef VIDEO_ENCODER_H
#define VIDEO_ENCODER_H

#include "../include/glad/glad.h" 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <vector>
#include "Shader.hpp"

#include "../include/Video_Codec_SDK_11.1.5/Interface/nvEncodeAPI.h"
#include "NVEncoder/NvEncoderGL.h"

// Uses the NVIDIA hardware encoder on your graphics card.
class NvidiaHardwareEncoder {
public:
    int nWidth;
    int nHeight;
    NvEncoderGL encoder;
    std::ofstream fpOut;

    NvidiaHardwareEncoder( int width
                        , int height
                        , int framerate
                        , NV_ENC_BUFFER_FORMAT eFormat
                        , GUID encodeGUID
                        , GUID presetGUID
                        , NV_ENC_TUNING_INFO tuningInfo
                        , std::string outFilePath
                        )
        : frameCopyShader( "../shaders/post-processing.vert", "../shaders/post-processing.frag" )
        , encoder(width, height, eFormat)
        , fpOut(outFilePath, std::ios::out | std::ios::binary)
        {
        nWidth = width;
        nHeight = height;

        initOpenGL();

        //fpOut = new std::ofstream(outFilePath, std::ios::out | std::ios::binary);
        if (!fpOut)
        {
            std::ostringstream err;
            err << "Unable to open output file: " << outFilePath << std::endl;
            throw std::invalid_argument(err.str());
        }

        //*encoder = new NvEncoderGL(width, height, eFormat);

        NV_ENC_INITIALIZE_PARAMS initializeParams = { NV_ENC_INITIALIZE_PARAMS_VER };
        NV_ENC_CONFIG encodeConfig = { NV_ENC_CONFIG_VER };
        initializeParams.encodeConfig = &encodeConfig;
        encoder.CreateDefaultEncoderParams(&initializeParams, encodeGUID, presetGUID, tuningInfo);

        //NV_ENC_CONFIG &config = initializeParams->encodeConfig;
        initializeParams.frameRateNum = framerate;
        
        encoder.CreateEncoder(&initializeParams);

    }

    void encodeFrame(GLuint inputFrameTexture){
        copyFrameToEncoder(inputFrameTexture);

        std::vector<std::vector<uint8_t>> vPacket;
        encoder.EncodeFrame(vPacket);

        for (std::vector<uint8_t> &packet : vPacket)
        {
            fpOut.write(reinterpret_cast<char*>(packet.data()), packet.size());
        }
    }

    void encodeLastFrame(GLuint inputFrameTexture){
        copyFrameToEncoder(inputFrameTexture);

        std::vector<std::vector<uint8_t>> vPacket;
        encoder.EndEncode(vPacket);

        for (std::vector<uint8_t> &packet : vPacket) {
            fpOut.write(reinterpret_cast<char*>(packet.data()), packet.size());
        }

        encoder.DestroyEncoder();
        fpOut.close();

        glDeleteVertexArrays(1, &INPUT_FRAME_VAO);
        glDeleteBuffers(1, &INPUT_FRAME_FBO);
        glDeleteBuffers(1, &INPUT_FRAME_VBO);
        frameCopyShader.deleteProgram();

    }

private:

    Shader frameCopyShader;
    GLuint INPUT_FRAME_VBO;
    GLuint INPUT_FRAME_FBO;
    GLuint INPUT_FRAME_VAO;

    /* Runs a small shader program to copy a texture into the hardware encoder.
    */
    void copyFrameToEncoder(GLuint inputFrameTexture){
        const NvEncInputFrame* encoderInputFrame = encoder.GetNextInputFrame();
        NV_ENC_INPUT_RESOURCE_OPENGL_TEX *pResource = (NV_ENC_INPUT_RESOURCE_OPENGL_TEX *)encoderInputFrame->inputPtr;

        frameCopyShader.use();
        glBindFramebuffer(GL_FRAMEBUFFER, INPUT_FRAME_FBO);
        glBindTexture(pResource->target, pResource->texture);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pResource->texture, 0);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inputFrameTexture);

        // Copy our frame into the texture that will be encoded
        glBindVertexArray(INPUT_FRAME_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

    }

    void initOpenGL(){
        glGenFramebuffers(1, &INPUT_FRAME_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, INPUT_FRAME_FBO);

        static const GLfloat copy_frame_quad[] = {
        //   positions     texture coordinates
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
        };
        glGenVertexArrays(1, &INPUT_FRAME_VAO);
        glGenBuffers(1, &INPUT_FRAME_VBO);
        glBindVertexArray(INPUT_FRAME_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, INPUT_FRAME_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(copy_frame_quad), copy_frame_quad, GL_STATIC_DRAW);

        // position attribute
        glEnableVertexAttribArray(0);
        GLsizei quad_stride = 4 * sizeof(float);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, quad_stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, quad_stride, (void*)(2 * sizeof(float)));

    }


};



#endif