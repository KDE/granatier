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

kde4_add_ui_files(granatierSources config/arenaselector.ui config/generalsettings.ui)

kde4_add_kcfg_files(granatierSources config/settings.kcfgc)

kde4_add_app_icon(granatierSources "${CMAKE_CURRENT_SOURCE_DIR}/../pictures/hi*-app-granatier.png")

kde4_add_executable(granatier ${granatierSources})

target_link_libraries(granatier ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${KDEGAMES_LIBRARY} ${KDE4_KNEWSTUFF2_LIBRARY} ${KDE4_KNEWSTUFF3_LIBS})


install(TARGETS granatier ${INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS granatier.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES config/granatier.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES ${granatierData} DESTINATION ${DATA_INSTALL_DIR}/granatier)

