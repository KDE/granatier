cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

# Release Service version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "23")
set (RELEASE_SERVICE_VERSION_MINOR "08")
set (RELEASE_SERVICE_VERSION_MICRO "3")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

# Bump GRANATIER_BASE_VERSION once new features are added
set(GRANATIER_BASE_VERSION "1.4")

set(GRANATIER_VERSION "${GRANATIER_BASE_VERSION}.${RELEASE_SERVICE_COMPACT_VERSION}")

project(granatier VERSION ${GRANATIER_VERSION})

set (QT_MIN_VERSION "5.15.2")
set (KF_MIN_VERSION "5.91.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(FeatureSummary)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_REQUIRED_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
endif()
include(ECMSetupVersion)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Svg)
find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons 
    Config
    Crash
    DBusAddons
    WidgetsAddons 
    Config 
    I18n 
    ConfigWidgets 
    XmlGui
    DocTools
)

if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(KDEGames6 7.5.0 REQUIRED)
else()
    find_package(KF5KDEGames 7.3.0 REQUIRED)
endif()

ecm_set_disabled_deprecation_versions(
    QT 6.4
    KF 5.102
    KDEGAMES 7.3
)

add_subdirectory(themes) 
add_subdirectory(sounds)
add_subdirectory(icons)
add_subdirectory(src)
add_subdirectory(arenas)
add_subdirectory(players)
add_subdirectory(doc)

ki18n_install(po)
kdoctools_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
