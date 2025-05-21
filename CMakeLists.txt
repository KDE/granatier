cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

# Release Service version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "25")
set (RELEASE_SERVICE_VERSION_MINOR "07")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

# Bump GRANATIER_BASE_VERSION once new features are added
set(GRANATIER_BASE_VERSION "1.4")

set(GRANATIER_VERSION "${GRANATIER_BASE_VERSION}.${RELEASE_SERVICE_COMPACT_VERSION}")

project(granatier VERSION ${GRANATIER_VERSION})

set(QT_MIN_VERSION "6.5.0")
set(KF_MIN_VERSION "5.245.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEGitCommitHooks)
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

include(FeatureSummary)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMSetupVersion)

include(InternalMacros)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Widgets
    Svg
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
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
    IconThemes
)

find_package(KDEGames6 6.0.0 REQUIRED)

ecm_set_disabled_deprecation_versions(
    QT 6.9.0
    KF 6.14.0
    KDEGAMES 6.0
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
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
