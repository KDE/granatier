FILE(GLOB oggfiles *.ogg)
install(FILES ${oggfiles} 
        DESTINATION  ${DATA_INSTALL_DIR}/granatier/sounds)
