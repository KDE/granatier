set(granatierSources
    config/arenaselector.cpp
    config/arenasettings.cpp
    config/playerselector.cpp
    config/playerselectoritem.cpp
    config/playersettings.cpp
    granatier_debug.cpp
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

ecm_qt_declare_logging_category(granatierSources HEADER granatier_debug.h IDENTIFIER GRANATIER_LOG CATEGORY_NAME log_granatier)

ki18n_wrap_ui(granatierSources config/arenaselector.ui config/generalsettings.ui)

kconfig_add_kcfg_files(granatierSources config/settings.kcfgc)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../pictures/*-apps-granatier.png")
ecm_add_app_icon(granatierSources ICONS ${ICONS_SRCS})

add_executable(granatier ${granatierSources})

target_compile_options(granatier PRIVATE -W -Wall -Wpedantic -Wextra -Wconversion -Wuninitialized)

target_link_libraries(granatier
    Qt5::Xml
    Qt5::Widgets
    KF5::NewStuff
    KF5KDEGames
    Qt5::Svg
    KF5::DBusAddons
    KF5::Crash
)

install(TARGETS granatier ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.granatier.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.granatier.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES config/granatier.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES ${granatierData} DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/granatier)
