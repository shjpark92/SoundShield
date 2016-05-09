#########################################################################
## CMakeLists files in 'SoundShield' project can refer to the root source 
## directory of the project as $(SOUNDSHIELD_SOURCE_DIR) and to the root binary
## directory of the project as $(SOUNDSHIELD_BINARY_DIR).
#########################################################################
## Searches the following directories:
##
## src			- Source Code Files
##
#########################################################################
## The following variables will be set upon execution:
##
## CMAKE_MODULE_PATH	- CMake module folder path
##
#########################################################################
## Executes the following files located within the CMake module folder
## 
## FindFFTW3.cmake	- Find FFTW3 includes and library, with components
## FindPortAudio.cmake	- Find PortAudio includes and library
## 
#########################################################################
## Creates the executable:
##
## SoundShield		- Main program
## SSTest           - Program to test SS functionalities
## DeviceConfig		- Program to find the user's device specification 
##
#########################################################################
# Minimum CMake version required
cmake_minimum_required(VERSION 2.6)

# Project name for Visual Studio and makefiles
project(SoundShield)

# SoundShield Version Number
set(SoundShield_VERSION_MAJOR 1)
set(SoundShield_VERSION_MINOR_0)

# Linker flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set(GCC_COVERAGE_COMPILE_FLAGS "-save-temps")
#set_target_properties(SoundShield PROPERTIES COMPILE_FLAGS "-save-temps")

# Application
set(APPLICATION_SOURCE_DIR ${PROJECT_SOURCE_DIR}/application/“)

# Configuring a header file to pass CMake settings to source
configure_file(
  "${PROJECT_SOURCE_DIR}/config/SoundShieldConfig.h.in"
  "${PROJECT_BINARY_DIR}/config/SoundShieldConfig.h"
  )

# Add the binary tree to the search path for include files
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/config")
include_directories(“${APPLICATION_SOURCE_DIR}”)

# Add subdirectories to search CMakeLists.txt for
add_subdirectory(src)

# Defines the path to CMake modules required for the project
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

# Finds PortAudio package and include in project directory
find_package(PortAudio REQUIRED)
include_directories(${PORTAUDIO_INCLUDE_DIRS})

# Finds FFTW3 package and include in project directory
find_package(FFTW3 REQUIRED COMPONENTS fftw3f)
include_directories(${FFTW3_INCLUDE_DIR})

# Find LibWebSockets library and include in project directory
find_package(LIBWEBSOCKETS REQUIRED)
include_directories(${LIBWEBSOCKETS_INCLUDE_DIRS})

#Find Libsndfile Library and include in project directory
find_package(SNDFILE REQUIRED)
include_directories(${SNDFILE_INCLUDE_DIRS})

#find Python
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

# Set SoundShield Source files
set(SOURCE	src/soundshield.c)
	 	
# Create the main executable 'SoundShield'
add_executable(SoundShield ${SOURCE})
#set_target_properties(SoundShield PROPERTIES COMPILE_FLAGS "-save-temps")

# Link dependencies to 'SoundShield'
ADD_DEFINITIONS( -g )
target_link_libraries(SoundShield ${PORTAUDIO_LIBRARIES})
target_link_libraries(SoundShield ${LIBWEBSOCKETS_LIBRARIES})
target_link_libraries(SoundShield ${SNDFILE_LIBRARIES})
target_link_libraries(SoundShield ${PYTHON_LIBRARIES})
target_link_libraries(SoundShield ss_config)
target_link_libraries(SoundShield ss_server)
target_link_libraries(SoundShield ss_callback)
target_link_libraries(SoundShield ss_mask_generation)

# Installation rules defined here
install(TARGETS SoundShield DESTINATION bin)
install(FILES "$(PROJECT_BINARY_DIR)/SoundShieldConfig.h"
	DESTINATION include)

install(TARGETS SoundShield DESTINATION bin)
install(FILES “${PROJECT_BINARY_DIR}/Application/” 
	DESTINATION application)

