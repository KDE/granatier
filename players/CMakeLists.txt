# SPDX-FileCopyrightText: 2023 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

function(install_player name)
    set(svg "${name}.svg")
    set(svgz "${CMAKE_CURRENT_BINARY_DIR}/${svg}z")
    generate_svgz(${svg} ${svgz} "player-")
    install(
        FILES
            ${name}.desktop
            ${svgz}
        DESTINATION ${KDE_INSTALL_DATADIR}/granatier/players
    )
endfunction()

install_player(player1)
install_player(player2)
install_player(player3)
install_player(player4)
install_player(player5)
