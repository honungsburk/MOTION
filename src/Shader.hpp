#ifndef SHADER_H
#define SHADER_H

#include "../include/glad/glad.h" 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "GLHelpers.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <boost/noncopyable.hpp>

/**
    CS-11 Asn 2: Create OpenGL shader program from source code.
    @file Shader.hpp
    @author Frank Hampus Weslien

    NOTE: The object can not be copied since at destruction it will remove the
    destroy the OpenGL program it manages.
*/
class Shader : private boost::noncopyable
{
    unsigned int ID;
public:

    /**
        Constructs a basic OpenGL shader program.
        @param vertexPath the path to the vertex shader source code.
        @param fragmentPath the path to the fragment shader source code.
    */
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode = loadSourceCode(vertexPath);
        unsigned int vertex = compileShaderCode("VERTEX", vertexCode);
        
        std::string fragmentCode = loadSourceCode(fragmentPath);
        unsigned int fragment = compileShaderCode("FRAGMENT", fragmentCode);

        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);

        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);

    }

    /**
        Constructs a OpenGL compute shader program.
        @param computePath the path to the compute shader source code.
    */
    Shader(const char* computePath)
    {
        std::string computeCode = loadSourceCode(computePath);
        glCheckError(); 
        unsigned int compute; compute = compileShaderCode("COMPUTE", computeCode);
        glCheckError(); 

        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, compute);
        glCheckError();

        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        glDeleteShader(compute);
    }

    void deleteProgram(){
            glDeleteProgram(ID);
    }

    /**
        Set this shader to the current active program in OpenGL.
    */
    void use() 
    { 
        glUseProgram(ID); 
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const
    {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const
    { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const
    { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    } 
    // ------------------------------------------------------------------------
    void setVec2f(const std::string &name, float value1, float value2) const
    { 
        glUniform2f(glGetUniformLocation(ID, name.c_str()), value1, value2); 
    }
    void setVec2f(const std::string &name, glm::vec2 value) const
    { 
        glUniform2f(glGetUniformLocation(ID, name.c_str()), value.x, value.y); 
    }
    // ------------------------------------------------------------------------
    void setVec3f(const std::string &name, float value1, float value2, float value3) const
    { 
        glUniform3f(glGetUniformLocation(ID, name.c_str()), value1, value2, value3); 
    }
    void setVec3f(const std::string &name, glm::vec3 value) const
    { 
        glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z); 
    }
    // ------------------------------------------------------------------------
    void setVec4f(const std::string &name, float value1, float value2, float value3, float value4) const
    { 
        glUniform4f(glGetUniformLocation(ID, name.c_str()), value1, value2, value3, value4); 
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, glm::mat4 value) const
    { 
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value)); 
    }

private:

    unsigned int compileShaderCode(std::string type, std::string sourceCode){
        const char* shaderCode = sourceCode.c_str();
        unsigned int shader;
        if (type == "VERTEX"){
            shader = glCreateShader(GL_VERTEX_SHADER);
        } else if (type == "FRAGMENT"){
            shader = glCreateShader(GL_FRAGMENT_SHADER);
        } else if (type == "COMPUTE"){
            shader = glCreateShader(GL_COMPUTE_SHADER);
        } else {
            std::cout << "ERROR::UNKNOWN_SHADER_TYPE '" << type << "'" << std::endl;
        }
        
        glShaderSource(shader, 1, &shaderCode, NULL);
        glCompileShader(shader);
        checkCompileErrors(shader, type);
        return shader;
    }

    std::string loadSourceCode(const char* shaderPath) {
        std::string shaderCode;
        std::ifstream shaderFile;
        // ensure ifstream objects can throw exceptions:
        shaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try 
        {
            // open files
            shaderFile.open(shaderPath);
            std::stringstream shaderStream;
            // read file's buffer contents into streams
            shaderStream << shaderFile.rdbuf();
            // close file handlers
            shaderFile.close();
            // convert stream into string
            return shaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ '" << shaderPath <<"'" << std::endl;
            return "";
        }
    }

    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
#endif