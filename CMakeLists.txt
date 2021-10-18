cmake_minimum_required(VERSION 2.8.12)
project(VectorFieldParticleSystem)

if(CMAKE_VERSION VERSION_LESS 3.0.0)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-std=c++11 COMPILER_SUPPORTS_CXX11)
    check_cxx_compiler_flag(-std=c++0x COMPILER_SUPPORTS_CXX0X)
    if(COMPILER_SUPPORTS_CXX11)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    elseif(COMPILER_SUPPORTS_CXX0X)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    endif()
else()
    SET(CMAKE_CXX_STANDARD 11)
    SET(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable( VectorFieldParticleSystem 
                src/main.cpp 
                src/glad.c 
                src/stbImager.cpp 
                src/NVEncoder/NvEncoder.cpp 
                src/NVEncoder/NvEncoderGL.cpp 
                include/glad/glad.h 
                include/KHR/khrplatform.h )
                
target_link_libraries(VectorFieldParticleSystem ${CONAN_LIBS})
