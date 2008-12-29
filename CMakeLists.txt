project(granatier)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

add_subdirectory(themes) 
add_subdirectory(sounds)
add_subdirectory(pictures)
add_subdirectory(src)
