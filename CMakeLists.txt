project(granatier)

option(GRANATIER_USE_GLUON_SOUND_BACKEND "Enable compilation with KAL from Gluon as sound backend")

if(GRANATIER_USE_GLUON_SOUND_BACKEND)
    macro_optional_find_package(GluonCommon QUIET)
    macro_optional_find_package(KAL QUIET)
    macro_optional_find_package(Eigen2 QUIET)
endif(GRANATIER_USE_GLUON_SOUND_BACKEND)

if(GLUONCOMMON_FOUND AND KAL_FOUND AND EIGEN2_FOUND)
    set(GRANATIER_KAL_SOUND TRUE)
endif(GLUONCOMMON_FOUND AND KAL_FOUND AND EIGEN2_FOUND)
macro_log_feature(GRANATIER_KAL_SOUND "Gluon is a high-level game development library for the KDE desktop enviornment. Granatier uses KAL for sound output." "http://gluon.tuxfamily.org" FALSE "" "Granatier will use Phonon for sound output. For better sound experience please use KAL from the gluon project.")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(GRANATIER_KAL_SOUND)
    include_directories(${GLUONCOMMON_INCLUDES})
    include_directories(${KAL_INCLUDES})
    include_directories(${EIGEN2_INCLUDE_DIR})
    ADD_DEFINITIONS(-DGRANATIER_USE_GLUON)
endif(GRANATIER_KAL_SOUND)

include(MacroLibrary)

add_subdirectory(themes) 
add_subdirectory(sounds)
add_subdirectory(pictures)
add_subdirectory(src)
add_subdirectory(arenas)
add_subdirectory(players)
