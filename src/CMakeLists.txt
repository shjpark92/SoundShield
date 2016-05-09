#########################################################################
## TODO: Update this later
## Creates CMake libraries for SoundShield's main program.
##
##
#########################################################################
# Forces CMake to properly set C99 flags for
# C files that utilizes C99.
macro(check_c99)
  if(CMAKE_VERSION VERSION_LESS "3.1")
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
      set(CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
    endif()
  else()
    set(CMAKE_C_STANDARD 99)
  endif()
endmacro(check_c99)


# Defines the path to CMake modules required for the project
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../CMake)

# Finds PortAudio package and include in project directory
find_package(PortAudio REQUIRED)
include_directories(${PORTAUDIO_INCLUDE_DIRS})

# Finds FFTW3 packages and include in the project directory
find_package(FFTW3 REQUIRED COMPONENTS fftw3f)
include_directories(${FFTW3_INCLUDE_DIR})

# Call check_c99 function to check all for files with C99 syntax
check_c99()

# Find all necessary files and add respective dependencies
add_library(ss_config ss_config.c)
target_link_libraries(ss_config ${PORTAUDIO_LIBRARIES})

add_library(ss_api ss_api.c)

add_library(ss_mask_generation ss_mask_generation.c)

add_library(ss_server ss_server.c)
target_link_libraries(ss_server ${LIBWEBSOCKETS_LIBRARIES})
target_link_libraries(ss_server pthread)
target_link_libraries(ss_server ss_api)

add_library(ss_fft ss_fft.c)
if(UNIX)
  target_link_libraries(ss_fft m)
endif(UNIX)
target_link_libraries(ss_fft ss_mask_generation)
target_link_libraries(ss_fft ${FFTW3_LIBRARIES})

add_library(ss_psd ss_psd.c)
if(UNIX)
  target_link_libraries(ss_psd m)
endif(UNIX)
target_link_libraries(ss_psd ss_mask_generation)

add_library(ss_callback ss_callback.c)
target_link_libraries(ss_callback ss_config)
target_link_libraries(ss_callback ss_psd)
target_link_libraries(ss_callback ss_fft)
target_link_libraries(ss_callback ss_mask_generation)
target_link_libraries(ss_callback ${PORTAUDIO_LIBRARIES})

# Installation rules defined here
#install(TARGETS input DESTINATION bin)
#install(FILES input DESTINATION include)

#install(TARGETS output DESTINATION bin)
#install(FILES output DESTINATION include)



