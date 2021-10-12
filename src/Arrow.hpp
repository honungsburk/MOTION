#ifndef ARROW_H
#define ARROW_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Only works in 2D!!!
class Arrow
{
public:
    glm::vec3 Start; 
    glm::vec3 End;
    float Thickness;
    float HeadLength;

    float verticies[7 * 3];
    unsigned int indices[9] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3,   // second triangle
        4, 5, 6    // third triangle
    };  

    Arrow(glm::vec3 start, glm::vec3 end, float thickness = 0.005, float headLength = 0.02){
          Start = start;
          End = end;
          Thickness = thickness;
          HeadLength = headLength;
          generateVertices();
      }

    void copyTo(float *otherVertices, int otherVerticesStart, unsigned int *otherIndices, int otherIndicesStart){
        for(int i = 0; i < 7 * 3; i++){
            otherVertices[otherVerticesStart + i] = verticies[i];
        }
        for(int i = 0; i < 9; i++){
            otherIndices[otherIndicesStart + i] = (otherVerticesStart / 3) + indices[i];
        }
      }

private:
    glm::vec3 Up = glm::vec3(0.0f, 0.0f, 1.0f);

    // Four floats for the box and three floats for the head 


    void generateVertices(){

        glm::vec3 direction = glm::normalize(End - Start);

        glm::vec3 ortogonal = glm::normalize(glm::cross(Up, direction));

        //Tail
        glm::vec3 startUp = Start + ortogonal * Thickness;
        glm::vec3 startDown = Start - ortogonal * Thickness;
        glm::vec3 endUp = End - direction * HeadLength + ortogonal * Thickness;
        glm::vec3 endDown = End - direction * HeadLength - ortogonal * Thickness;

        verticies[0] = startUp.x;
        verticies[1] = startUp.y;
        verticies[2] = startUp.z;
        verticies[3] = startDown.x;
        verticies[4] = startDown.y;
        verticies[5] = startDown.z;
        verticies[6] = endDown.x;
        verticies[7] = endDown.y;
        verticies[8] = endDown.z;
        verticies[9] = endUp.x;
        verticies[10] = endUp.y;
        verticies[11] = endUp.z;

        //Head
        glm::vec3 headUp = endUp + ortogonal * Thickness * 2.0f;
        glm::vec3 headDown = endDown - ortogonal * Thickness * 2.0f;

        verticies[12] = headUp.x;
        verticies[13] = headUp.y;
        verticies[14] = headUp.z;
        verticies[15] = headDown.x;
        verticies[16] = headDown.y;
        verticies[17] = headDown.z;
        verticies[18] = End.x ;
        verticies[19] = End.y ;
        verticies[20] = End.z;

    }

};

#endif