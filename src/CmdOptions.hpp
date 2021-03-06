#ifndef CMD_OPTIONS_H
#define CMD_OPTIONS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <stdexcept>
#include <boost/program_options.hpp>


using namespace boost::program_options;

/**
    CS-11 Asn 2: Parse all application options from the command line and config.
    @file CmdOptions.hpp
    @author Frank Hampus Weslien
*/
class CmdOptions
{
public:
    //Flag
    bool failed = false;

    // Generic
    bool show_help;
    bool show_version;
    std::string shaderPath = "";

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
    int colorMode;
    glm::vec3 cosColorBase;
    glm::vec3 cosColorAmplitude;
    glm::vec3 cosColorSpeed;
    glm::vec3 cosColorOffset;
    glm::vec2 cosColorAnglePos;
    glm::vec4 particle_color;
    glm::vec4 background_color;   


    // Record Info
    unsigned int fps;
    bool record;
    unsigned int lengthInSeconds;
    unsigned int delayInSeconds;
    std::string outFileName = "";
    unsigned int crf;
    std::string preset = "";
    std::string tune = "";

    // Screenshot
    bool screenshot;
    std::string screenshotFileName = "";
    unsigned int screenshotDelay;


    options_description cmdline_options{"Motion Options"};
    options_description config_file_options;

    CmdOptions(){
        init();
    }

    CmdOptions(int argc, char **argv){
        init();
        parse(argc, argv);
    }

    /**
        Parse and fill the values from the command line.
        @param argc the number of arguments given to the main function
        @param argv the arguments from the main function
    */
    void parse(int argc, char **argv){
        variables_map vm;
        store(parse_command_line(argc, argv, cmdline_options), vm);

        if (vm.count("config")){
            std::vector<std::string> filenames = vm["config"].as<std::vector<std::string>>();
            for(std::string filename : filenames) {
                store(parse_config_file(filename.c_str(), config_file_options), vm);
            }
        }

        notify(vm);


        // Generic
        // ----------------------------------------------------------------------------------------
        
        if (vm.count("help"))
            show_help = true;
        else 
            show_help = false;

        if (vm.count("version"))
            show_version = true;
        else 
            show_version = false;

        if (vm.count("shader-path"))
            shaderPath = vm["shader-path"].as<std::string>();
    

        // Resolution
        // ----------------------------------------------------------------------------------------

        if (vm.count("width-ratio"))
            width_ratio = vm["width-ratio"].as<unsigned int>();
        
        if (vm.count("height-ratio"))
            height_ratio = vm["height-ratio"].as<unsigned int>();
        
        if (vm.count("pixels-per-ratio"))
            pixels_per_ratio = vm["pixels-per-ratio"].as<unsigned int>();

        // Simulation
        // ----------------------------------------------------------------------------------------

        if (vm.count("vectors-per-ratio"))
            vectors_per_ratio = vm["vectors-per-ratio"].as<unsigned int>();
        
        if (vm.count("point-size"))
            point_size = vm["point-size"].as<float>();

        if (vm.count("nbr-particles"))
            nbr_particles = vm["nbr-particles"].as<unsigned int>();

        if (vm.count("nbr-compute-groups"))
            nbr_compute_groups = vm["nbr-compute-groups"].as<unsigned int>();


        if(vm.count("vector-field-function"))
            vector_field_function = vm["vector-field-function"].as<unsigned int>();

        if(vm.count("probability-to-die"))
            probability_to_die = vm["probability-to-die"].as<float>();     
     
        
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

        // Trail
        // ----------------------------------------------------------------------------------------

        if(vm.count("trail-mix-rate"))
            trail_mix_rate = vm["trail-mix-rate"].as<float>();    

        // Color
        // ----------------------------------------------------------------------------------------

        if (vm.count("color-mode")){
            std::string color_mode = vm["color-mode"].as<std::string>();
            if(color_mode == "basic") {
                colorMode = 0;
            } else if (color_mode == "angle-basic") {
                colorMode = 1;
            } else if (color_mode == "angle-pos") {
                colorMode = 2;
            } else if (color_mode == "velocity") {
                colorMode = 3;
            } else {
                std::cout 
                    << "WARNING: '--color-mode "
                    << color_mode
                    << "' only accepts 'basic', 'angle-basic', 'angle-pos' or 'velocity'" 
                    << std::endl;
                failed = true;
            }
        }

        if (vm.count("particle-color"))
            particle_color = fromHexColor(vm["particle-color"].as<std::string>());

        if (vm.count("background-color"))
            background_color = fromHexColor(vm["background-color"].as<std::string>());

        if (vm.count("background-alpha"))
            background_color.w = vm["background-alpha"].as<float>();


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

        if(vm.count("cos-color-base")){
            std::vector<float> cosBaseTemp = vm["cos-color-base"].as<std::vector<float>>();
            if(cosBaseTemp.size() == 3){
                cosColorBase = glm::vec3(cosBaseTemp[0],cosBaseTemp[1], cosBaseTemp[2]);
            } else {
                std::cout 
                    << "WARNING: '--cos-color-base ...' "
                    << " needs three floating point numbers" 
                    << std::endl;
                failed = true;
            }
        } else {
            cosColorBase = glm::vec3(0.5, 0.5, 0.5);
        }

        if(vm.count("cos-color-amplitude")){
            std::vector<float> cosAmplitudeTemp = vm["cos-color-amplitude"].as<std::vector<float>>();
            if(cosAmplitudeTemp.size() == 3){
                cosColorAmplitude = glm::vec3(cosAmplitudeTemp[0],cosAmplitudeTemp[1], cosAmplitudeTemp[2]);
            } else {
                std::cout 
                    << "WARNING: '--cos-color-amplitude ...' "
                    << " needs three floating point numbers" 
                    << std::endl;
                failed = true;
            }
        } else {
            cosColorAmplitude = glm::vec3(0.5, 0.5, 0.5);
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
        // ----------------------------------------------------------------------------------------

        if (vm.count("record")){
            outFileName = vm["record"].as<std::string>(); 
            if(hasEnding(outFileName, ".mp4")){
                record = true;
            } else {
                std::cout 
                << "WARNING: '--record "
                << outFileName
                << "' the filename must end with '.mp4'" 
                << std::endl;
                failed = true;
            }
        }
        else 
            record = false;

        if(vm.count("fps"))
            fps = vm["fps"].as<unsigned int>();

        if(vm.count("length"))
            lengthInSeconds = vm["length"].as<unsigned int>();

        if(vm.count("delay"))
            delayInSeconds = vm["delay"].as<unsigned int>();

        if(vm.count("crf")){
            unsigned int tmpCRF = vm["crf"].as<unsigned int>();
            if(tmpCRF >= 0 && tmpCRF <= 51){
                crf = tmpCRF;
            } else {
                std::cout 
                    << "WARNING: '--crf "
                    << tmpCRF
                    << "' must be a value in the range 0-51."
                    << std::endl;
                failed = true;
            }
        }

        if(vm.count("preset")){
            std::string tmpPreset = vm["preset"].as<std::string>();
            if( tmpPreset == "ultrafast" 
                || tmpPreset == "superfast" 
                || tmpPreset == "veryfast" 
                || tmpPreset == "faster" 
                || tmpPreset == "fast" 
                || tmpPreset == "medium" 
                || tmpPreset == "slow" 
                || tmpPreset == "slower" 
                || tmpPreset == "veryslow"
                ){
                preset = tmpPreset;
            } else {
                std::cout 
                    << "WARNING: '--preset "
                    << tmpPreset
                    << "' must be one of: 'ultrafast', 'superfast', 'veryfast', 'faster', 'fast', 'medium', 'slow' , 'slower', or 'veryslow'"
                    << std::endl;
                failed = true;
            }
        }

        if(vm.count("tune")){
            std::string tmpTune = vm["tune"].as<std::string>();
            if( tmpTune == "film" 
                || tmpTune == "animation" 
                || tmpTune == "grain" 
                || tmpTune == "stillimage" 
                || tmpTune == "fastdecode" 
                || tmpTune == "zerolatency" 
                ){
                tune = tmpTune;
            } else {
                std::cout 
                    << "WARNING: '--tune "
                    << tmpTune
                    << "' must be one of: 'film', 'animation', 'grain', 'stillimage', 'fastdecode', or 'zerolatency'"
                    << std::endl;
                failed = true;
            }
        }


        // Screenshot
        // ----------------------------------------------------------------------------------------
        if (vm.count("screenshot")){
            screenshotFileName = vm["screenshot"].as<std::string>();
            if(hasEnding(screenshotFileName, ".png")){
                screenshot = true;
            } else {
                std::cout 
                << "WARNING: '--screenshot "
                << screenshotFileName
                << "' the filename must end with '.png'" 
                << std::endl;
                failed = true;
            }

        }
        else 
            screenshot = false;

        if(vm.count("screenshot-delay"))
            screenshotDelay = vm["screenshot-delay"].as<unsigned int>();



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
        options_description color{"Color"};
        options_description recording{"Record"};
        options_description screenshot{"Screenshot"};

        generic.add_options()
            ("help", "Help screen")
            ("version,v", "Print version string")
            ("config", value<std::vector<std::string>>()->multitoken(), "Files containing command line options")
            ("shader-path", value<std::string>()->default_value("./shaders"), "The folder where the shaders are located");

        simulation.add_options()
            ("width-ratio, w", value<unsigned int>()->default_value(16), "Width-Ratio like 16 in 16:9")
            ("height-ratio, h", value<unsigned int>()->default_value(9), "Width-Ratio like 9 in 16:9")
            ("pixels-per-ratio", value<unsigned int>()->default_value(80), "The number of pixels per ratio")
            ("vectors-per-ratio", value<unsigned int>()->default_value(1200), "The number of vectors per ratio")
            ("speed", value<float>()->default_value(1.0), "modify the speed of the particles")
            ("point-size", value<float>()->default_value(2.0f), "The pixel size of the particles")
            ("nbr-particles", value<unsigned int>()->default_value(1024), "The number of particles in the simulation")
            ("nbr-compute-groups", value<unsigned int>()->default_value(1024), "The number of compute groups issues to the graphics card")
            ("vector-field-function", value<unsigned int>()->default_value(0), "Which function to use when creating the vector field")
            ("interpolation-mode", value<std::string>()->default_value("smooth"), "Which interpolation mode to use")
            ("probability-to-die", value<float>()->default_value(0.01f), "The probability for a particle to die")
            ("trail-mix-rate", value<float>()->default_value(0.9f), "The rate by which the particle trail is mixed into the background");
            
        color.add_options()
            ("color-mode", value<std::string>()->default_value("basic"), "Choose which color mode: basic or angle")
            ("particle-color", value<std::string>()->default_value("ffffff"), "Particle color as a RGB hex")
            ("background-color", value<std::string>()->default_value("000000"), "Background color as a RGB hex")
            ("background-alpha", value<float>()->default_value(1.0), "Background alpha value between 0.0-1.0")
            ("cos-color-base", value<std::vector<float>>()->multitoken(), "The base which we oscillate around")
            ("cos-color-amplitude", value<std::vector<float>>()->multitoken(), "The amplitude of the cos color component ")
            ("cos-color-speed", value<std::vector<float>>()->multitoken(), "The speed of change for the red, green, and blue components when using cos coloring")
            ("cos-color-offset", value<std::vector<float>>()->multitoken(), "The offsets for the red, green, and blue components when using cos coloring")
            ("cos-angle-offset", value<std::vector<float>>()->multitoken(), "The angle/position the angle is computed against");

        screenshot.add_options()
            ("screenshot", value<std::string>(), "Name of the output file to store the screenshot (as a png)")
            ("screenshot-delay", value<unsigned int>()->default_value(1), "The number of seconds before the screenshot is taken.");

        recording.add_options()
            ("record", value<std::string>(), "The output video filename (we are using H264 encoding and file must end in '.mp4')")
            ("fps", value<unsigned int>()->default_value(30), "The number of frames per second when recording")
            ("length", value<unsigned int>()->default_value(10), "The length of the recording in seconds")
            ("delay", value<unsigned int>()->default_value(0), "The number of seconds to delay before recording starts")
            ("crf", value<unsigned int>()->default_value(23),"The CRF value: 0???51")
            ("preset", value<std::string>()->default_value("slow"),"The ffmpeg preset: 'ultrafast', 'superfast', 'veryfast', 'faster', 'fast', 'medium', 'slow' , 'slower', 'veryslow'")
            ("tune", value<std::string>()->default_value("animation"),"The ffmpeg tune: 'film', 'animation', 'grain', 'stillimage', 'fastdecode', 'zerolatency'");


        cmdline_options.add(generic).add(simulation).add(color).add(recording).add(screenshot);

        
        config_file_options.add(simulation).add(color).add(recording).add(screenshot);
    }

    bool hasEnding (std::string const &fullString, std::string const &ending) {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
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