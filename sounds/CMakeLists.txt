FILE(GLOB wavfiles *.wav)
install(FILES ${wavfiles}
        DESTINATION  ${KDE_INSTALL_DATADIR}/granatier/sounds)
