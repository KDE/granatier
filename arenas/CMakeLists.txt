FILE(GLOB arenaFiles *.desktop *.xml)
install(FILES ${arenaFiles}
        DESTINATION  ${KDE_INSTALL_DATADIR}/granatier/arenas)
