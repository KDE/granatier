project(granatier)

find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)
find_package(LibKDEGames REQUIRED)
macro_optional_find_package(Gluon QUIET)

include_directories(${KDE4_INCLUDES})
include_directories(${KDEGAMES_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(GLUON_FOUND)
    include_directories(${GLUON_INCLUDES})
    ADD_DEFINITIONS(-DGRANATIER_USE_GLUON)
endif(GLUON_FOUND)

include(KDE4Defaults)
include(MacroLibrary)

add_subdirectory(themes) 
add_subdirectory(sounds)
add_subdirectory(pictures)
add_subdirectory(src)
add_subdirectory(arenas)