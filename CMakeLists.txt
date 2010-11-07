project(granatier)

macro_optional_find_package(OpenAL QUIET)
macro_log_feature(OPENAL_FOUND "OpenAL" "OpenAL (Open Audio Library) is a free software cross-platform audio API." "http://connect.creativelabs.com/openal" FALSE "" "Granatier uses OpenAL for sound output if available. If OpenAL is not found, Phonon is used instead.")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
macro_optional_find_package(SndFile )
macro_log_feature(SNDFILE_FOUND "SndFile" "libsndfile is a C library written by Erik de Castro Lopo for reading and writing audio files." "http://www.mega-nerd.com/libsndfile/" FALSE "" "SndFile is needed to decode the sound files.")

if(OPENAL_FOUND AND SNDFILE_FOUND)
  include_directories(${OPENAL_INCLUDE_DIR} ${SNDFILE_INCLUDE_DIRS})
  include_directories(src/TagaroAudio)
  ADD_DEFINITIONS(-DGRANATIER_USE_OPENAL)
  set(GRANATIER_OPENAL TRUE)
endif(OPENAL_FOUND AND SNDFILE_FOUND)

add_subdirectory(themes) 
add_subdirectory(sounds)
add_subdirectory(pictures)
add_subdirectory(src)
add_subdirectory(arenas)
add_subdirectory(players)
