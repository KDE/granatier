FILE(GLOB arenaFiles *.desktop *.xml)
install(FILES ${arenaFiles}
        DESTINATION  ${DATA_INSTALL_DIR}/granatier/arenas)
