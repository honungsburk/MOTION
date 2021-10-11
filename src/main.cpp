#include <iostream>
#include <cmath>
#include <functional>
#include <tuple>

#include "../include/glad/glad.h" 
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include "Shader.hpp" 
#include "camera.h"
#include "Arrow.hpp"
#include "GLHelpers.hpp"
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>



#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

struct VectorField {
    std::vector<float> data;
    int width;
    int height;
};

struct ParticleSystem { 
    Shader shader;
    VectorField vectorField;
    unsigned int ssbo;
};

VectorField createVectorField(int width, int height, int resolution, int padding, std::function<std::tuple<float, float>(int, int)> f);
ParticleSystem initParticleSystem(Shader particleComputeShader, VectorField vectorField);
void updateParticleSystem(ParticleSystem particleSystem, VectorField vectorField);

GLenum glCheckError_(const char *file, int line);
// settings
const unsigned int SCR_WIDTH = 1280 ;
const unsigned int SCR_HEIGHT = 720;
const unsigned int VECTOR_FIELD_RESOLUTION = 80;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, -3.0f));

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//Location Vector

struct Vector {
    float x;
    float y;
};


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //std::cout << glGetError() << std::endl; // returns 0 (no error)

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE); // enabled by default on some drivers, but not all so always enable to make sure
    glCheckError(); 



    // Build and compile our shader programs
    // ------------------------------------
    Shader particleComputeShader("../shaders/particle.comp");
    glCheckError(); 
    Shader particleShader("../shaders/particle.vert", "../shaders/particle.frag");
    glCheckError(); 
    Shader postprocessingTrailShader( "../shaders/post-processing.vert", "../shaders/post-processing-trail.frag" );
    glCheckError(); 
    Shader postprocessingShader( "../shaders/post-processing.vert", "../shaders/post-processing.frag" );
    glCheckError(); 


    // Vector Field
    // ------------------------------------

    auto rotationFn = [](float x, float y) { return std::make_tuple(0.01 * sin(x*M_PI/9.0f), 0.01 * cos(y*M_PI/9.0f)); };

    VectorField vectorField = createVectorField(SCR_WIDTH, SCR_HEIGHT, VECTOR_FIELD_RESOLUTION, 0, rotationFn);

    ParticleSystem particleSystem = initParticleSystem(particleComputeShader, vectorField);


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    // float vertices[vectorFieldDim * vectorFieldDim * 7 * 3];
    // unsigned int indices[vectorFieldDim * vectorFieldDim  * 9];

    // for (int i = 0; i < vectorFieldDim; i++) {
    //     for (int j = 0; j < vectorFieldDim; j++) {
    //         int verticesToCopy =  7 * 3;
    //         int  verticesBase = (i * vectorFieldDim + j) * verticesToCopy;
    //         float stepSize = 2.0f / float(vectorFieldDim - 1);
    //         glm::vec3 arrowStart, arrowEnd;
    //         arrowStart.x = float(i) * stepSize - 1.0f;
    //         arrowStart.y = float(j) * stepSize - 1.0f;
    //         arrowStart.z = 0.0;
    //         arrowEnd.x = arrowStart.x + vectorField[i][j].x * 0.1;
    //         arrowEnd.y = arrowStart.y + vectorField[i][j].y * 0.1;
    //         arrowEnd.z = 0.0;

    //         Arrow arrow(arrowStart, arrowEnd);

    //         int indicesToCopy = 9;
    //         int indicesBase = (i * vectorFieldDim + j) * indicesToCopy;

    //         arrow.copyTo(vertices, verticesBase, indices, indicesBase);
    //     }
    // }

    // unsigned int VBO, VAO, EBO;
    // glGenVertexArrays(1, &VAO);
    // glGenBuffers(1, &VBO);
    // glGenBuffers(1, &EBO);

    // glBindVertexArray(VAO);

    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

    // // position attribute
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);

    // vectorFieldShader.use();
    // Defined the camera position
    // calculate the model matrix for each object and pass it to shader before drawing
    // glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    // model = glm::scale(model, glm::vec3(0.9, 0.9, 1.0));
    // // model = glm::translate(model, cubePositions[i]);
    // // float angle = 20.0f * i;
    // // model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
    // vectorFieldShader.setMat4("model", model);


    // Set model
    // ------------------------------------
    particleShader.use();
    glm::mat4 model = glm::mat4(1.0f);
    particleShader.setMat4("model", model);

    // Particles
    // ------------------------------------
    std::vector<float> particles;
    int numberOfParticles = 1024;
    //float particles[numberOfParticles * 2];

    for(int i = 0; i < numberOfParticles * 2; i++){
        // TODO: Use propeor good randomness instead...
        float pos = static_cast <float> ( 2 * rand()) / static_cast <float> (RAND_MAX);
        particles.push_back(pos);
    }

    unsigned int PARTICLE_VAO, PARTICLE_VBO;
    
    glGenVertexArrays(1, &PARTICLE_VAO);
    glGenBuffers(1, &PARTICLE_VBO);
    glBindVertexArray(PARTICLE_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, PARTICLE_VBO);
    glBufferData(GL_ARRAY_BUFFER, particles.size() * 4, particles.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // Allow particle.comp to update the particle positions
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, PARTICLE_VBO);

    // Particles FBO to be used for post-processing
    // -----------------------------------------------
    GLuint particleFBO;
    glGenFramebuffers(1, &particleFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, particleFBO);
    // The texture we're going to render to
    GLuint particleTexture;
    glGenTextures(1, &particleTexture);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, particleTexture);

    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0,GL_RGB, GL_UNSIGNED_BYTE, NULL);

     // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, particleTexture, 0);  
    glBindTexture(GL_TEXTURE_2D, 0); // unbind

    // Background FBO:s
    // 
    // We create two textures that we ping-pong between
    // -------------------------------
    GLuint backgroundFBOs[2];
    glGenFramebuffers(2, backgroundFBOs);
    GLuint backgroundTextures[2];
    glGenTextures(2, backgroundTextures);

    // Bind first FBO
    glBindFramebuffer(GL_FRAMEBUFFER, backgroundFBOs[0]);
    glBindTexture(GL_TEXTURE_2D, backgroundTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0,GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backgroundTextures[0], 0);  

    // Bind second FBO
    glBindFramebuffer(GL_FRAMEBUFFER, backgroundFBOs[1]);
    glBindTexture(GL_TEXTURE_2D, backgroundTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0,GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backgroundTextures[1], 0);  

    glBindTexture(GL_TEXTURE_2D, 0); // unbind



    // Set the list of draw buffers.
    // GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    // glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);//unbind

    // Post Processing
    // ----------------
    static const GLfloat post_processing_quad[] = {
    //   positions     texture coordinates
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int POST_PROCESSING_VAO, POST_PROCESSING_VBO;
    glGenVertexArrays(1, &POST_PROCESSING_VAO);
    glGenBuffers(1, &POST_PROCESSING_VBO);
    glBindVertexArray(POST_PROCESSING_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, POST_PROCESSING_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(post_processing_quad), post_processing_quad, GL_STATIC_DRAW);

    // position attribute
    glEnableVertexAttribArray(0);
    GLsizei quad_stride = 4 * sizeof(float);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, quad_stride, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, quad_stride, (void*)(2 * sizeof(float)));

    // Shader Configuration
    // --------------------
    postprocessingShader.use();

    postprocessingTrailShader.use();

    // Global Settings
    // ---------------
    glPointSize(2.0f);

    //Used to pingpong between FBO:s
    unsigned int pingPongFBOIndex = 0;

    static GLubyte *pixels = NULL;
    static png_byte *png_bytes = NULL;
    static png_byte **png_rows = NULL;
    unsigned int frameNbr = 0;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // Update Particle Positions
        // ------
        particleComputeShader.use();
        particleShader.setFloat("u_time", currentFrame);
        glBindVertexArray(PARTICLE_VAO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, PARTICLE_VBO);
        glDispatchCompute(numberOfParticles / 1024, 1, 1);


        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

        // Render Particles
        // ------
        glBindFramebuffer(GL_FRAMEBUFFER, particleFBO);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        particleShader.use();
        particleShader.setVec4f("u_color", 1.0f, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(PARTICLE_VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, particleTexture);
        glDrawArrays(GL_POINTS, 0, numberOfParticles);

        // Add Trails Post-Processing
        //
        // The idea is to save the background in one of the FBO:s
        // and reduce the alpha as we ping-pong back and forth.
        // Not that this means we shouldn't be clearing the buffers.
        // ------
        unsigned int inTexture = pingPongFBOIndex;
        unsigned int outTexture = (pingPongFBOIndex + 1) % 2;

        glBindFramebuffer(GL_FRAMEBUFFER, backgroundFBOs[outTexture]);
        postprocessingTrailShader.use();

        postprocessingTrailShader.setInt("screenTexture", 0);
        postprocessingTrailShader.setInt("trailTexture", 1);
        postprocessingTrailShader.setVec4f("clearColor", 0.0f, 0.0f, 0.0f, 1.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, particleTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, backgroundTextures[inTexture]);

        glBindVertexArray(POST_PROCESSING_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        pingPongFBOIndex = outTexture;

        // Post-Processing
        // ------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        glDisable(GL_DEPTH_TEST); // Make sure quad is rendered on top of all other
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        postprocessingShader.use();
        postprocessingShader.setInt("screenTexture", 0);
        glBindVertexArray(POST_PROCESSING_VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, backgroundFBOs[outTexture]);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    
        // Save Image
        //---------------------------------------------------------
        // std::stringstream filename;
        // filename << "../images/" << frameNbr << ".png" << std::endl;

        // screenshot_png(filename.str().c_str(), SCR_WIDTH, SCR_HEIGHT, &pixels, &png_bytes, &png_rows);
        // frameNbr += 1;

    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &PARTICLE_VAO);
    glDeleteBuffers(1, &PARTICLE_VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// Creates a vector field width a given resolution covering the the width and height.
// Width - the width that the vector field has to cover in pixels 
// Height - the hight that the vector field has to cover in pixels 
// resolution - pixels between vectors
// padding - number of extra layers of vectors to add on each side
//
// The padding is helpfull if you want the vector field to extend beyond the screen.
// ------------------------------------------------------------------------------------
VectorField createVectorField(int width, int height, int resolution, int padding, std::function<std::tuple<float, float>(int, int)> f)
{

    int vectorFieldWidth = (width / resolution) + padding * 2;
    int vectorFieldHeight = (height / resolution) + padding * 2;

    std::vector<float> vectorFieldData;
    for (int i = 0; i < vectorFieldWidth; i++) {
            for (int j = 0; j < vectorFieldHeight; j++) {
                auto res = f(i, j);
                vectorFieldData.push_back(std::get<0>(res));
                vectorFieldData.push_back(std::get<1>(res));
            }
    }

    return VectorField { vectorFieldData, vectorFieldWidth, vectorFieldHeight };
}

ParticleSystem initParticleSystem(Shader particleComputeShader, VectorField vectorField){
    particleComputeShader.use();
    glCheckError(); 
    particleComputeShader.setInt("u_width", vectorField.width);
    glCheckError(); 
    particleComputeShader.setInt("u_height", vectorField.height);
    glCheckError(); 

    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glCheckError(); 
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glCheckError(); 
    glBufferData(GL_SHADER_STORAGE_BUFFER, vectorField.data.size() * 4, vectorField.data.data(), GL_STATIC_DRAW);
    glCheckError(); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo);
    glCheckError(); 
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
    glCheckError(); 

    return ParticleSystem { particleComputeShader, vectorField, ssbo };

}

void updateParticleSystem(ParticleSystem particleSystem, VectorField vectorField){
    particleSystem.shader.use();
    glCheckError(); 
    particleSystem.shader.setInt("u_width", vectorField.width);
    glCheckError(); 
    particleSystem.shader.setInt("u_height", vectorField.height);
    glCheckError(); 

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleSystem.ssbo);
    glCheckError(); 
    glBufferData(GL_SHADER_STORAGE_BUFFER, vectorField.data.size() * 4, vectorField.data.data(), GL_STATIC_DRAW);
    glCheckError(); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, particleSystem.ssbo);
    glCheckError(); 
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
    glCheckError(); 

    particleSystem.vectorField = vectorField;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

