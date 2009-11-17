FILE(GLOB wavfiles *.wav)
install(FILES ${wavfiles}
        DESTINATION  ${DATA_INSTALL_DIR}/granatier/sounds)
