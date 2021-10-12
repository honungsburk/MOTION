#ifndef CMD_OPTIONS_H
#define CMD_OPTIONS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include "InputParser.hpp"

struct ColorSchema {
    glm::vec4 particleColor;
    glm::vec4 backgroundColor;
};

// Contains all the options that has been set for the application.
class CmdOptions
{
public:
    unsigned int SCR_WIDTH = 1200;
    unsigned int SCR_HEIGHT = 1200;
    unsigned int VECTOR_FIELD_RESOLUTION = 100;
    float POINT_SIZE = 2.0f;
    unsigned int NUMBER_PARTICLES = 1024;
    unsigned int NUMBER_COMPUTE_GROUPS = 1024;
    ColorSchema COLOR_SCHEMA = {glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)};
    bool RECORD = false;
    std::string RECORD_FOLDER = "../images/";
    int NUMBER_FRAMES_TO_RECORD = -1;
    unsigned int VECTOR_FIELD_FUNCTION= 0;
    float PROBABILITY_TO_DIE = 0.0;

    CmdOptions(int argc, char **argv){
        InputParser input(argc, argv);
    
    const std::string &width = input.getCmdOption("-w");
    if (!width.empty()){
        SCR_WIDTH = std::stoi( width );
    }
    const std::string &height = input.getCmdOption("-h");
    if (!height.empty()){
        SCR_HEIGHT = std::stoi( height );
    }
    const std::string &resolution = input.getCmdOption("-r");
    if (!height.empty()){
        VECTOR_FIELD_RESOLUTION = std::stoi( resolution );
    }
    const std::string &point_size = input.getCmdOption("--p-size");
    if (!height.empty()){
        POINT_SIZE = std::stof( point_size );
    }

    const std::string &number_particles = input.getCmdOption("--nbr-particles");
    if (!number_particles.empty()){
        NUMBER_PARTICLES = std::stoi(number_particles );
    }

    const std::string &number_compute_groups = input.getCmdOption("--nbr-compute-groups");
    if (!number_compute_groups.empty()){
        NUMBER_COMPUTE_GROUPS = std::stoi(number_compute_groups );
    }

    const std::string &particle_color = input.getCmdOption("--particle-color");
    if (!particle_color.empty()){
        COLOR_SCHEMA.particleColor = fromHexColor(particle_color);
    }
    const std::string &background_color = input.getCmdOption("--background-color");
    if (!background_color.empty()){
        COLOR_SCHEMA.backgroundColor = fromHexColor(background_color);
    }
    const std::string &number_frames_record = input.getCmdOption("--number-frames-to-record");
    if (!number_frames_record.empty()){
        NUMBER_FRAMES_TO_RECORD = std::stoi(number_frames_record );
    }
    if (input.cmdOptionExists("--record")){
        RECORD = true;
    }

    const std::string &vector_field_function = input.getCmdOption("--vector-field-function");
    if (!vector_field_function.empty()){
        VECTOR_FIELD_FUNCTION = std::stoi(vector_field_function);
    }

    const std::string &probability_to_die = input.getCmdOption("--probability-to-die");
    if (!probability_to_die.empty()){
        PROBABILITY_TO_DIE = std::stof(probability_to_die);
    }


    // WARNINGS
    // ----------------------------------------------------------------------------------------

    if(NUMBER_PARTICLES % NUMBER_COMPUTE_GROUPS!= 0){
        std::cout 
            << "WARNING: '--nbr-particles' was specified to a number not divisible by the number of compute groups '" 
            << NUMBER_COMPUTE_GROUPS 
            << "'" 
            << std::endl;
        }
    }

private:





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

};

#endif