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

enum ColorMode { basic, anglebasic, anglepos };

using namespace boost::program_options;

// Contains all the options that has been set for the application.
class CmdOptions
{
public:
    //Flag
    bool failed = false;

    // Generic
    bool show_help;
    bool show_version;

    // Simulations

    // Resolution independent
    unsigned int width_ratio;
    unsigned int height_ratio;
    unsigned int pixels_per_ratio;
    unsigned int vectors_per_ratio;

    unsigned int nbr_compute_groups;
    unsigned int vector_field_function;
    float probability_to_die;
    float trail_mix_rate = 0.9;

    unsigned int interpolation_mode;
    float speed;

    // Resolution dependent
    float point_size;
    unsigned int nbr_particles;

    // Coloring
    ColorMode colorMode;
    glm::vec3 cosColorSpeed;
    glm::vec3 cosColorOffset;
    glm::vec2 cosColorAnglePos;
    glm::vec4 particle_color;
    glm::vec4 background_color;   


    // Record Info
    std::string record_folder = "../images/";
    std::string record_to_file = "../motion.mp4";
    std::string codec_name = "265/HVEC";
    unsigned int bitrate = 800000;

    unsigned int fps;
    bool record;
    bool perfectLoop = false;
    unsigned int lengthInSeconds;


    options_description cmdline_options{"Motion Options"};
    options_description config_file_options;

    CmdOptions(){
        init();
    }

    CmdOptions(int argc, char **argv){
        init();
        parse(argc, argv);
    }


    void parse(int argc, char **argv){
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

        
        if (vm.count("width-ratio"))
            width_ratio = vm["width-ratio"].as<unsigned int>();
        
        if (vm.count("height-ratio"))
            height_ratio = vm["height-ratio"].as<unsigned int>();
        
        if (vm.count("pixels-per-ratio"))
            pixels_per_ratio = vm["pixels-per-ratio"].as<unsigned int>();

        if (vm.count("vectors-per-ratio"))
            vectors_per_ratio = vm["vectors-per-ratio"].as<unsigned int>();


        
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

        if (vm.count("background-alpha")){
            background_color.w = vm["background-alpha"].as<float>();
        }

        if(vm.count("vector-field-function"))
            vector_field_function = vm["vector-field-function"].as<unsigned int>();

        if(vm.count("probability-to-die"))
            probability_to_die = vm["probability-to-die"].as<float>();     

        if(vm.count("trail-mix-rate"))
            trail_mix_rate = vm["trail-mix-rate"].as<float>();         
        
        if(vm.count("speed"))
            speed = vm["speed"].as<float>();    

        if (vm.count("interpolation-mode")){
            std::string interpolation_mode_s = vm["interpolation-mode"].as<std::string>();
            if(interpolation_mode_s == "smooth") {
                interpolation_mode = 0;
            } else if (interpolation_mode_s == "min") {
                interpolation_mode = 1;
            } else {
                std::cout 
                    << "WARNING: '--interpolation-mode "
                    << interpolation_mode_s
                    << "' only accepts 'smooth' or 'min'" 
                    << std::endl;
                failed = true;
            }
        }

        if (vm.count("color-mode")){
            std::string color_mode = vm["color-mode"].as<std::string>();
            if(color_mode == "basic") {
                colorMode = basic;
            } else if (color_mode == "angle-basic") {
                colorMode = anglebasic;
            } else if (color_mode == "angle-pos") {
                colorMode = anglepos;
            } else {
                std::cout 
                    << "WARNING: '--color-mode "
                    << color_mode
                    << "' only accepts 'basic' or 'angle'" 
                    << std::endl;
                failed = true;
            }
        }

        if(vm.count("cos-angle-offset")){
            std::vector<float> cosAngleOffset = vm["cos-angle-offset"].as<std::vector<float>>();
            if(cosAngleOffset.size() == 2){
                cosColorAnglePos = glm::vec2(cosAngleOffset[0], cosAngleOffset[1]);
            } else {
                std::cout 
                    << "WARNING: '--cos-angle-offset ...' "
                    << " needs two floating point numbers" 
                    << std::endl;
                failed = true;
            }
        }

        if(vm.count("cos-color-speed")){
            std::vector<float> cosSpeedTemp = vm["cos-color-speed"].as<std::vector<float>>();
            if(cosSpeedTemp.size() == 3){
                cosColorSpeed = glm::vec3(cosSpeedTemp[0],cosSpeedTemp[1], cosSpeedTemp[2]);
            } else {
                std::cout 
                    << "WARNING: '--cos-color-speed ...' "
                    << " needs three floating point numbers" 
                    << std::endl;
                failed = true;
            }
        }

        if(vm.count("cos-color-offset")){
            std::vector<float> cosOffsetTemp = vm["cos-color-offset"].as<std::vector<float>>();
            if(cosOffsetTemp.size() == 3){
                cosColorOffset = glm::vec3(cosOffsetTemp[0],cosOffsetTemp[1], cosOffsetTemp[2]);
            } else {
                std::cout 
                    << "WARNING: '--cos-color-offset ...' "
                    << " needs three floating point numbers" 
                    << std::endl;
                failed = true;
            }
        }


        // Record

        if (vm.count("record"))
            record = true;
        else 
            record = false;

        if(vm.count("fps"))
            fps = vm["fps"].as<unsigned int>();

        if(vm.count("length"))
            lengthInSeconds = vm["length"].as<unsigned int>();

        if (vm.count("perfect-loop"))
            perfectLoop = true;
        else 
            perfectLoop = false;


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
            failed = true;
            }
    }

    unsigned int width(){
        return width_ratio * pixels_per_ratio;
    }

    unsigned int height(){
        return height_ratio * pixels_per_ratio;
    }

    unsigned int vectorGridWidth() {
        return width_ratio * vectors_per_ratio;
    }

    unsigned int vectorGridHeight() {
        return height_ratio * vectors_per_ratio;
    }

private:

    void init(){
        options_description generic{"Generic"};
        options_description simulation{"Simulate"};
        options_description recording{"Record"};

        generic.add_options()
            ("help", "Help screen")
            ("version,v", "Print version string")
            ("config", value<std::string>(), "File containing command line options");

        simulation.add_options()
            ("width-ratio, w", value<unsigned int>()->default_value(16), "Width-Ratio like 16 in 16:9")
            ("height-ratio, h", value<unsigned int>()->default_value(9), "Width-Ratio like 9 in 16:9")
            ("pixels-per-ratio", value<unsigned int>()->default_value(80), "The number of pixels per ratio")
            ("vectors-per-ratio", value<unsigned int>()->default_value(1200), "The number of vectors per ratio")
            ("speed", value<float>()->default_value(1.0), "modify the speed of the particles")
            ("point-size", value<float>()->default_value(2.0f), "The pixel size of the particles")
            ("nbr-particles", value<unsigned int>()->default_value(1024), "The number of particles in the simulation")
            ("nbr-compute-groups", value<unsigned int>()->default_value(1024), "The number of compute groups issues to the graphics card")
            ("particle-color", value<std::string>()->default_value("ffffff"), "Particle Color as 'ffffff'")
            ("background-color", value<std::string>()->default_value("000000"), "Background Color as '000000'")
            ("background-alpha", value<float>()->default_value(1.0), "Background alpha value between 0.0-1.0")
            ("vector-field-function", value<unsigned int>()->default_value(0), "Which function to use when creating the vector field")
            ("interpolation-mode", value<std::string>()->default_value("smooth"), "Which interpolation mode to use")
            ("probability-to-die", value<float>()->default_value(0.01f), "The probability for a particle to die")
            ("trail-mix-rate", value<float>()->default_value(0.9f), "The rate by which the particle trail is mixed into the background")
            ("cos-color-speed", value<std::vector<float>>()->multitoken(), "The speed of change for the red, green, and blue components when using cos coloring")
            ("cos-color-offset", value<std::vector<float>>()->multitoken(), "The offsets for the red, green, and blue components when using cos coloring")
            ("cos-angle-offset", value<std::vector<float>>()->multitoken(), "The angle/position the angle is computed against")
            ("color-mode", value<std::string>()->default_value("basic"), "Choose which color mode: basic or angle");

        recording.add_options()
            ("record", "If the program should record")
            ("fps", value<unsigned int>()->default_value(30), "The number of frames per second when recording")
            ("length", value<unsigned int>()->default_value(10), "The length of the recording in seconds")
            ("perfect-loop", "if the recording should create perfect loops");
        
        cmdline_options.add(generic).add(simulation).add(recording);

        
        config_file_options.add(simulation).add(recording);
    }


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