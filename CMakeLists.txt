PROJECT( qshell )

set(CMAKE_CXX_FLAGS "-Wno-reorder")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.3.0")
set(KF5_MIN_VERSION "5.3.0")

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
    Widgets
    X11Extras
)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

# require at least gcc 4.8
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if ("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.8")
        message(SEND_ERROR "Version ${CMAKE_CXX_COMPILER_VERSION} of the ${CMAKE_CXX_COMPILER_ID} C++ compiler is not supported. Please use version 4.8 or later.")
    endif()
endif()

# required frameworks by Core
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    KIO
    WindowSystem
)
find_package(KF5Solid NO_MODULE)

find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE REQUIRED
                    )

add_definitions("-fexceptions")
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_definitions("-Wno-mismatched-tags")
endif()

set(BIN_INSTALL_DIR "/usr/bin")
set(CONFIG_INSTALL_DIR "/usr/share/qshell")
set(XSESSION_INSTALL_DIR "/usr/share/xsessions")

# ----
file(GLOB shell_SRCS RELATIVE ${CMAKE_SOURCE_DIR} "src/*.cpp")

set(pamixer_SRCS
    pamixer/callbacks.cc
    pamixer/device.cc
    pamixer/pulseaudio.cc
)


add_executable(qshell ${shell_SRCS} ${pamixer_SRCS})

target_link_libraries(qshell
    Qt5::Concurrent
    Qt5::DBus
    Qt5::X11Extras
    KF5::ConfigCore
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::Solid
    KF5::WindowSystem
    ${X11_X11_LIB}
    pulse
    )

install(TARGETS qshell DESTINATION ${BIN_INSTALL_DIR})
install(PROGRAMS "qshellctl" DESTINATION ${BIN_INSTALL_DIR})
install(DIRECTORY "themes" DESTINATION ${CONFIG_INSTALL_DIR})
install(FILES "qshell.desktop" DESTINATION ${XSESSION_INSTALL_DIR})
