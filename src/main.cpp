// Your First C++ Program

#include <iostream>
#include <cmath>

#include "../include/glad/glad.h" 
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include "Shader.hpp" 
#include "camera.h"
#include "Arrow.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>


#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

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

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("../shaders/line.vert", "../shaders/line.frag");

    // Vector Field
    int vectorFieldDim = 2;
    Vector vectorField[vectorFieldDim][vectorFieldDim];
    for (int i = 0; i < vectorFieldDim; i++) {
            for (int j = 0; j < vectorFieldDim; j++) {
                vectorField[i][j].x = sin(i*M_PI/9);
                vectorField[i][j].y = cos(i*M_PI/9);
            }
    }

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[vectorFieldDim * vectorFieldDim * 7 * 3];
    unsigned int indices[vectorFieldDim * vectorFieldDim  * 9];

    for (int i = 0; i < vectorFieldDim; i++) {
        for (int j = 0; j < vectorFieldDim; j++) {
            int verticesToCopy =  7 * 3;
            int  verticesBase = (i * vectorFieldDim + j) * verticesToCopy;
            float stepSize = 2.0f / float(vectorFieldDim - 1);
            glm::vec3 arrowStart, arrowEnd;
            arrowStart.x = float(i) * stepSize - 1.0f;
            arrowStart.y = float(j) * stepSize - 1.0f;
            arrowStart.z = 0.0;
            arrowEnd.x = arrowStart.x + vectorField[i][j].x * 0.1;
            arrowEnd.y = arrowStart.y + vectorField[i][j].y * 0.1;
            arrowEnd.z = 0.0;

            Arrow arrow(arrowStart, arrowEnd);

            int indicesToCopy = 9;
            int indicesBase = (i * vectorFieldDim + j) * indicesToCopy;

            arrow.copyTo(vertices, verticesBase, indices, indicesBase);
        }
    }

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    ourShader.use();
    // Defined the camera position
    // calculate the model matrix for each object and pass it to shader before drawing
    glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    model = glm::scale(model, glm::vec3(0.9, 0.9, 1.0));
    // model = glm::translate(model, cubePositions[i]);
    // float angle = 20.0f * i;
    // model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
    ourShader.setMat4("model", model);

    // camera/view transformation
    glm::mat4 view = camera.GetViewMatrix();
    ourShader.setMat4("view", view);

    // pass projection matrix to shader (note that in this case it could change every frame)
    //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    
    // We want a 2D top down view so we use orthographic projection
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);
    ourShader.setMat4("projection", projection);


    // glLineWidth(2);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_LINE_SMOOTH);



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

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        // activate shader
        ourShader.use();
        ourShader.setVec4f("u_color", 1.0f, 1.0f, 1.0f, 1.0f);

        // render boxes
        glBindVertexArray(VAO);
        //glDrawArrays(GL_LINES, 0, vectorFieldDim * vectorFieldDim * 2);
        glDrawElements(GL_TRIANGLES, vectorFieldDim * vectorFieldDim * 9, GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
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
