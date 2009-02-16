project(granatier)

set(granatierSources
	bonus.cpp
  bomb.cpp
  bombitem.cpp
	cell.cpp
	character.cpp
	characteritem.cpp
	element.cpp
	elementitem.cpp
	energizer.cpp
	game.cpp
	gamescene.cpp
	gameview.cpp
	ghost.cpp
	ghostitem.cpp
	kapman.cpp
	kapmanitem.cpp
	mainwindow.cpp
	mapparser.cpp
	main.cpp
	maze.cpp
	mazeitem.cpp
	pill.cpp
)
set(granatierData
	defaultmaze.xml
	granatierui.rc
)

kde4_add_kcfg_files(granatierSources settings.kcfgc)

kde4_add_executable(granatier ${granatierSources})

target_link_libraries(granatier ${KDE4_KDEUI_LIBS} ${KDE4_PHONON_LIBS} ${QT_QTXML_LIBRARY} ${KDEGAMES_LIBRARY})

install(TARGETS granatier ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES granatier.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES granatier.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES ${granatierData} DESTINATION ${DATA_INSTALL_DIR}/granatier)

