project(granatier)

set(granatierSources
    config/arenaselector.cpp
    config/arenasettings.cpp
    config/playerselector.cpp
    config/playerselectoritem.cpp
    config/playersettings.cpp

    arena.cpp
    arenaitem.cpp
    block.cpp
    blockitem.cpp
    bonus.cpp
    bonusitem.cpp
    bomb.cpp
    bombitem.cpp
    bombexplosionitem.cpp
    cell.cpp
    character.cpp
    characteritem.cpp
    element.cpp
    elementitem.cpp
    game.cpp
    gamescene.cpp
    gameview.cpp
    infooverlay.cpp
    infosidebar.cpp
    main.cpp
    mainwindow.cpp
    mapparser.cpp
    player.cpp
    playeritem.cpp
)

set(granatierData
    config/granatierui.rc
)

ki18n_wrap_ui(granatierSources config/arenaselector.ui config/generalsettings.ui)

kconfig_add_kcfg_files(granatierSources config/settings.kcfgc)

kde4_add_app_icon(granatierSources "${CMAKE_CURRENT_SOURCE_DIR}/../pictures/hi*-apps-granatier.png")

add_executable(granatier ${granatierSources})

target_link_libraries(granatier  Qt5::Xml Qt5::Widgets KF5::NewStuff KF5KDEGames KF5::KDELibs4Support Qt5::Svg)


install(TARGETS granatier ${INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.granatier.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES granatier.appdata.xml DESTINATION share/appdata)
install(FILES config/granatier.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES ${granatierData} DESTINATION ${DATA_INSTALL_DIR}/granatier)

