project(granatier)

set(granatierSources
    arena.cpp
    arenaitem.cpp
    arenaselector.cpp
    arenasettings.cpp
    block.cpp
    bonus.cpp
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
    main.cpp
    mainwindow.cpp
    mapparser.cpp
    player.cpp
    playeritem.cpp
)
set(granatierData
    granatierui.rc
)

kde4_add_ui_files(granatierSources arenaselector.ui generalsettings.ui)

kde4_add_kcfg_files(granatierSources settings.kcfgc)

kde4_add_executable(granatier ${granatierSources})

target_link_libraries(granatier ${KDE4_KDEUI_LIBS} ${KDE4_PHONON_LIBS} ${QT_QTXML_LIBRARY} ${KDEGAMES_LIBRARY} kal)

install(TARGETS granatier ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES granatier.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES granatier.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES ${granatierData} DESTINATION ${DATA_INSTALL_DIR}/granatier)

