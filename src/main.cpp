#include <iostream>
#include <cmath>
#include <functional>
#include <tuple>

#include "../include/glad/glad.h" 
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include "Shader.hpp" 
#include "Arrow.hpp"
#include "GLHelpers.hpp"
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "CmdOptions.hpp"
#include "InputParser.hpp"

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

// enum VectorFieldFN { Rotate, Directional, blue };

glm::vec4 fromHexColor(std::string hexColor);
VectorField createVectorField(int width, int height, int resolution, int padding, std::function<std::tuple<float, float>(float, float, float, float)> f);
ParticleSystem initParticleSystem(Shader particleComputeShader, VectorField vectorField);
void updateParticleSystem(ParticleSystem particleSystem, VectorField vectorField);

GLenum glCheckError_(const char *file, int line);
// settings


// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


int main(int argc, char **argv)
{

        // Command line options
        // ------------------------------
    CmdOptions cmdOptions(argc, argv);

    if(cmdOptions.show_help){
        std::cout << cmdOptions.cmdline_options << '\n';
        return EXIT_SUCCESS;
    }

    if(cmdOptions.show_version){
        std::cout << "Motion v0.2.0" << '\n';
        return EXIT_SUCCESS;
    }


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
    GLFWwindow* window = glfwCreateWindow(cmdOptions.width, cmdOptions.height, "MOTION", NULL, NULL);
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

    // Global Settings
    // ---------------
    glPointSize(cmdOptions.point_size);

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

    std::function<std::tuple<float, float>(float, float, float, float)> vectorFieldFn; 

   switch(cmdOptions.vector_field_function) {
    case 0 : vectorFieldFn 
                = [](float x, float y, float width, float height) { 
                        return std::make_tuple(0.01 * sin(x*M_PI/9.0f), 0.01 * cos(y*M_PI/9.0f)); 
                        };
             break;       // and exits the switch
    case 1 : vectorFieldFn 
                = [](float x, float y, float width, float height) { 
                        return std::make_tuple(0.01 * sin(x*M_PI/9.0f) + 0.01, 0.0); 
                        };
             break;
    case 2 : vectorFieldFn 
                = [](float x, float y, float width, float height) { 
                        float center_x = width / 2;
                        float center_y = width / 2;
                        float max_length = sqrt(width*width + height*height) / 2.0;
                        return std::make_tuple(0.01 * (x - center_x) / max_length   , 0.01 * (y - center_y) / max_length ); 
                        };
            break;
    case 3 : vectorFieldFn 
                = [](float x, float y, float width, float height) { 
                        float sign = -1.0f + 2.0f * ((int) y % 2);
                        return std::make_tuple(sign * 0.01, 0);
                        };
            break;
    case 4 : vectorFieldFn 
                = [](float x, float y, float width, float height) { 
                        float x_ = x / width * 0.01;
                        float y_ = y / height * 0.01;
                        return std::make_tuple(x_ + y_, y_ - x_);
                        };
            break;
    }

    VectorField vectorField = createVectorField(cmdOptions.width, cmdOptions.height, cmdOptions.grid_resolution, 0, vectorFieldFn);

    ParticleSystem particleSystem = initParticleSystem(particleComputeShader, vectorField);

    // Set model
    // ------------------------------------
    particleShader.use();
    glm::mat4 model = glm::mat4(1.0f);
    particleShader.setMat4("model", model);

    // Particles
    // ------------------------------------
    std::vector<float> particles;

    for(int i = 0; i < cmdOptions.nbr_particles; i++){
        // TODO: Use proper good randomness instead...

        // Start position
        float pos_x = static_cast <float> ( 2 * rand()) / static_cast <float> (RAND_MAX);
        float pos_y = static_cast <float> ( 2 * rand()) / static_cast <float> (RAND_MAX);
        particles.push_back(pos_x);
        particles.push_back(pos_y);
        
        // Padding
        particles.push_back(0.0);
        particles.push_back(0.0);

        // Color
        particles.push_back(cmdOptions.particle_color.x);
        particles.push_back(cmdOptions.particle_color.y);
        particles.push_back(cmdOptions.particle_color.z);
        particles.push_back(cmdOptions.particle_color.w);
    }

    unsigned int PARTICLE_VAO, PARTICLE_VBO;
    
    glGenVertexArrays(1, &PARTICLE_VAO);
    glGenBuffers(1, &PARTICLE_VBO);
    glBindVertexArray(PARTICLE_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, PARTICLE_VBO);
    glBufferData(GL_ARRAY_BUFFER, particles.size() * 4, particles.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (4 * sizeof(float)));
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
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, cmdOptions.width, cmdOptions.height, 0,GL_RGB, GL_UNSIGNED_BYTE, NULL);

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
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, cmdOptions.width, cmdOptions.height, 0,GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backgroundTextures[0], 0);  

    // Bind second FBO
    glBindFramebuffer(GL_FRAMEBUFFER, backgroundFBOs[1]);
    glBindTexture(GL_TEXTURE_2D, backgroundTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, cmdOptions.width, cmdOptions.height, 0,GL_RGB, GL_UNSIGNED_BYTE, NULL);

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
        particleComputeShader.setFloat("u_time", currentFrame);
        particleComputeShader.setFloat("u_probability_to_die", cmdOptions.probability_to_die);

        particleComputeShader.setBool("u_angle_color", true);

        particleComputeShader.setVec3f("u_cc", 0.3, 0.6, 0.9);
        particleComputeShader.setVec3f("u_dd", 0.3, 1.6, 2.9);

        glBindVertexArray(PARTICLE_VAO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, PARTICLE_VBO);
        glDispatchCompute(cmdOptions.nbr_particles / cmdOptions.nbr_compute_groups, 1, 1);


        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

        // Render Particles
        // ------
        glBindFramebuffer(GL_FRAMEBUFFER, particleFBO);
        glEnable(GL_DEPTH_TEST);

        glClearColor( cmdOptions.background_color.x
                    , cmdOptions.background_color.y
                    , cmdOptions.background_color.z
                    , cmdOptions.background_color.w
                    );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        particleShader.use();
        glBindVertexArray(PARTICLE_VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, particleTexture);
        glDrawArrays(GL_POINTS, 0, cmdOptions.nbr_particles);

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
        postprocessingTrailShader.setVec4f( "u_clearColor"
                                          , cmdOptions.background_color.x
                                          , cmdOptions.background_color.y
                                          , cmdOptions.background_color.z
                                          , cmdOptions.background_color.w
                                          );
        postprocessingTrailShader.setFloat("u_trail_mix", cmdOptions.trail_mix_rate);

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
        glClearColor( cmdOptions.background_color.x
                    , cmdOptions.background_color.y
                    , cmdOptions.background_color.z
                    , cmdOptions.background_color.w
                    );
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
        if(cmdOptions.record){
            std::stringstream filename;
            std::string fileNumber = std::to_string(frameNbr);
            std::string new_string = std::string(6- fileNumber.length(), '0') + fileNumber;
            filename << cmdOptions.record_folder.c_str() << new_string << ".png";
            screenshot_png(filename.str().c_str(), cmdOptions.width, cmdOptions.height, &pixels, &png_bytes, &png_rows);
            std::cout << filename.str().c_str() << std::endl;
            frameNbr += 1;

            if(cmdOptions.nbr_frames_to_record == frameNbr){
                return 0;
            }
        }

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
VectorField createVectorField(int width, int height, int resolution, int padding, std::function<std::tuple<float, float>(float, float, float, float)> f)
{

    int vectorFieldWidth = (width / resolution) + padding * 2;
    int vectorFieldHeight = (height / resolution) + padding * 2;

    std::vector<float> vectorFieldData;
    for (int i = 0; i < vectorFieldWidth; i++) {
            for (int j = 0; j < vectorFieldHeight; j++) {
                auto res = f(i, j, vectorFieldWidth, vectorFieldHeight);
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

glm::vec4 fromHexColor(std::string hexColor){
    unsigned int start = 1;
    if(hexColor.length() == 6){
        start = 0;
    }
    unsigned int r = std::stoul(hexColor.substr(start,2), nullptr, 16);
    unsigned int g = std::stoul(hexColor.substr(start + 2,2), nullptr, 16);
    unsigned int b = std::stoul(hexColor.substr(start + 4,2), nullptr, 16);
    return glm::vec4((float) r / 255.0, (float) g / 255.0, (float) b / 255.0, 1.0);
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

