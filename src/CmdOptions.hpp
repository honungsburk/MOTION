#ifndef CMD_OPTIONS_H
#define CMD_OPTIONS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <stdexcept>
#include <boost/program_options.hpp>

struct ColorSchema {
    glm::vec4 particleColor;
    glm::vec4 backgroundColor;
};

using namespace boost::program_options;

// Contains all the options that has been set for the application.
class CmdOptions
{
public:
    bool show_help;
    bool show_version;
    unsigned int width;
    unsigned int height;
    unsigned int grid_resolution;
    float point_size;
    unsigned int nbr_particles;
    unsigned int nbr_compute_groups;
    glm::vec4 particle_color;
    glm::vec4 background_color;   
    bool record;
    int nbr_frames_to_record;
    unsigned int vector_field_function;
    float probability_to_die;
    float trail_mix_rate = 0.9;

    std::string record_folder = "../images/";


    options_description cmdline_options{"Motion Options"};
    
    CmdOptions(int argc, char **argv){

        options_description generic{"Generic"};
        options_description simulation{"Simulate"};
        options_description recording{"Record"};

        generic.add_options()
            ("help", "Help screen")
            ("version,v", "Print version string")
            ("config", value<std::string>(), "File containing command line options");

        simulation.add_options()
            ("width,w", value<unsigned int>()->default_value(1200), "Width in pixels")
            ("height,h", value<unsigned int>()->default_value(1200), "Height in pixels")
            ("grid-resolution,r", value<unsigned int>()->default_value(100), "The number of pixels between each vector in the grid")
            ("point-size", value<float>()->default_value(2.0f), "The pixel size of the particles")
            ("nbr-particles", value<unsigned int>()->default_value(1024), "The number of particles in the simulation")
            ("nbr-compute-groups", value<unsigned int>()->default_value(1024), "The number of compute groups issues to the graphics card")
            ("particle-color", value<std::string>()->default_value("ffffff"), "Particle Color as 'ffffff'")
            ("background-color", value<std::string>()->default_value("000000"), "Background Color as '000000'")
            ("vector-field-function", value<unsigned int>()->default_value(0), "Which function to use when creating the vector field")
            ("probability-to-die", value<float>()->default_value(0.01f), "The probability for a particle to die")
            ("trail-mix-rate", value<float>()->default_value(0.9f), "The rate by which the particle trail is mixed into the background");
        
        recording.add_options()
            ("record", "If the program should record")
            ("nbr-frames-to-record", value<int>()->default_value(-1), "The total number of frames to record");
        
        cmdline_options.add(generic).add(simulation).add(recording);

        options_description config_file_options;
        config_file_options.add(simulation).add(recording);

        variables_map vm;
        store(parse_command_line(argc, argv, cmdline_options), vm);

        if (vm.count("config")){
            std::string filename = vm["config"].as<std::string>();
            store(parse_config_file(filename.c_str(), config_file_options), vm);
        }

        notify(vm);
        
        if (vm.count("help"))
            show_help = true;
        else 
            show_help = false;

        if (vm.count("version"))
            show_version = true;
        else 
            show_version = false;
        
        if (vm.count("width"))
            width = vm["width"].as<unsigned int>();
        
        if (vm.count("height"))
            height = vm["height"].as<unsigned int>();
        
        if (vm.count("grid-resolution"))
            grid_resolution = vm["grid-resolution"].as<unsigned int>();
        
        if (vm.count("point-size"))
            point_size = vm["point-size"].as<float>();

        if (vm.count("nbr-particles"))
            nbr_particles = vm["nbr-particles"].as<unsigned int>();

        if (vm.count("nbr-compute-groups"))
            nbr_compute_groups = vm["nbr-compute-groups"].as<unsigned int>();

        if (vm.count("particle-color"))
            particle_color = fromHexColor(vm["particle-color"].as<std::string>());
        

        if (vm.count("background-color"))
            background_color = fromHexColor(vm["background-color"].as<std::string>());

        if (vm.count("record"))
            record = true;
        else 
            record = false;

        if(vm.count("nbr-frames-to-record"))
            nbr_frames_to_record = vm["nbr-frames-to-record"].as<int>();

        if(vm.count("vector-field-function"))
            vector_field_function = vm["vector-field-function"].as<unsigned int>();

        if(vm.count("probability-to-die"))
            probability_to_die = vm["probability-to-die"].as<float>();     

        if(vm.count("trail-mix-rate"))
            trail_mix_rate = vm["trail-mix-rate"].as<float>();         


        // WARNINGS
        // ----------------------------------------------------------------------------------------

        if(nbr_particles % nbr_compute_groups != 0){
            std::cout 
                << "WARNING: '--nbr-particles "
                << nbr_particles
                << "' was specified to a number not divisible by '--nbr-compute-groups" 
                << nbr_compute_groups 
                << "'" 
                << std::endl;
            }

        if(width % grid_resolution != 0){
            std::cout 
                << "WARNING: '--width "
                << width
                << "' was specified to a number not divisible by '--grid-resolution" 
                << grid_resolution 
                << "'" 
                << std::endl;
            }

        if(height % grid_resolution != 0){
            std::cout 
                << "WARNING: '--height "
                << height
                << "' was specified to a number not divisible by '--grid-resolution" 
                << grid_resolution 
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
        
        if(hexColor.length() < 6 || hexColor.length() > 7 || hexColor.find_first_not_of("0123456789abcdefABCDEF#") != std::string::npos)
            throw std::invalid_argument( "'" + hexColor + "' is not a valid color"  );

        unsigned int r = std::stoul(hexColor.substr(start,2), nullptr, 16);
        unsigned int g = std::stoul(hexColor.substr(start + 2,2), nullptr, 16);
        unsigned int b = std::stoul(hexColor.substr(start + 4,2), nullptr, 16);
        return glm::vec4((float) r / 255.0, (float) g / 255.0, (float) b / 255.0, 1.0);
    }

};

#endif