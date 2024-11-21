ecm_setup_version(${GRANATIER_VERSION}
    VARIABLE_PREFIX GRANATIER
    VERSION_HEADER granatier_version.h
)

add_executable(granatier)

target_sources(granatier PRIVATE
    arena.cpp
    arena.h
    arenaitem.cpp
    arenaitem.h
    block.cpp
    block.h
    blockitem.cpp
    blockitem.h
    bomb.cpp
    bombexplosionitem.cpp
    bombexplosionitem.h
    bomb.h
    bombitem.cpp
    bombitem.h
    bonus.cpp
    bonus.h
    bonusitem.cpp
    bonusitem.h
    cell.cpp
    cell.h
    character.cpp
    character.h
    characteritem.cpp
    characteritem.h
    config/arenaselector.cpp
    config/arenaselector.h
    config/arenasettings.cpp
    config/arenasettings.h
    config/playerselector.cpp
    config/playerselector.h
    config/playerselectoritem.cpp
    config/playerselectoritem.h
    config/playersettings.cpp
    config/playersettings.h
    element.cpp
    element.h
    elementitem.cpp
    elementitem.h
    game.cpp
    game.h
    gamescene.cpp
    gamescene.h
    gameview.cpp
    gameview.h
    granatierglobals.h
    granatier_random.h
    infooverlay.cpp
    infooverlay.h
    infosidebar.cpp
    infosidebar.h
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mapparser.cpp
    mapparser.h
    player.cpp
    player.h
    playeritem.cpp
    playeritem.h

    granatier.qrc
)

ecm_qt_declare_logging_category(granatier
    HEADER granatier_debug.h
    IDENTIFIER GRANATIER_LOG
    CATEGORY_NAME granatier
    OLD_CATEGORY_NAMES log_granatier
    DESCRIPTION "Granatier game"
    EXPORT GRANATIER
)

ki18n_wrap_ui(granatier config/arenaselector.ui config/generalsettings.ui)

kconfig_add_kcfg_files(granatier config/settings.kcfgc)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*-apps-granatier.png")
ecm_add_app_icon(granatier ICONS ${ICONS_SRCS})

target_link_libraries(granatier
    KDEGames6
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::Crash
    KF6::I18n
    KF6::XmlGui
    Qt6::Xml
    Qt6::Widgets
    Qt6::Svg
    KF6::IconThemes
)

install(TARGETS granatier ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.granatier.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.granatier.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES config/granatier.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ecm_qt_install_logging_categories(
    EXPORT GRANATIER
    FILE granatier.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
