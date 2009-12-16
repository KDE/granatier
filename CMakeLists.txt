project(granatier)

macro_optional_find_package(Gluon QUIET)
macro_log_feature(GLUON_FOUND "Gluon" "Gluon is a high-level game development library for the KDE desktop enviornment." "http://gluon.tuxfamily.org" FALSE "" "Granatier will use Phonon for sound output. For better sound experience please use gluon.")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(GLUON_FOUND)
    include_directories(${GLUON_INCLUDES})
    ADD_DEFINITIONS(-DGRANATIER_USE_GLUON)
endif(GLUON_FOUND)

include(MacroLibrary)

add_subdirectory(themes) 
add_subdirectory(sounds)
add_subdirectory(pictures)
add_subdirectory(src)
add_subdirectory(arenas)
add_subdirectory(players)
