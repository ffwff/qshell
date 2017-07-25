
PROJECT( qshell )

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.3.0")
set(KF5_MIN_VERSION "5.3.0")

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Concurrent
    Core
    DBus
    UiTools
    Widgets
    X11Extras
    Xml
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

SET(CMAKE_CXX_FLAGS "-std=c++0x")

# required frameworks by Core
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    CoreAddons
    Crash
    GlobalAccel
    I18n
    KIO
    Service
    WindowSystem
)

find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE REQUIRED
                    )

find_package(XCB COMPONENTS XFIXES IMAGE UTIL)
set(XCB_COMPONENTS_ERRORS FALSE)
if (XCB_FOUND)
	find_package(Qt5X11Extras ${QT_MIN_VERSION} REQUIRED)
endif()
set(XCB_COMPONENTS_FOUND TRUE)
if(NOT XCB_XFIXES_FOUND)
	set(XCB_COMPONENTS_ERRORS "${XCB_COMPONENTS_ERRORS} XCB-XFIXES ")
	set(XCB_COMPONENTS_FOUND FALSE)
endif()
if(NOT XCB_IMAGE_FOUND)
	set(XCB_COMPONENTS_ERRORS "${XCB_COMPONENTS_ERRORS} XCB-IMAGE ")
	set(XCB_COMPONENTS_FOUND FALSE)
endif()
if(NOT XCB_UTIL_FOUND)
	set(XCB_COMPONENTS_ERRORS "${XCB_COMPONENTS_ERRORS} XCB-UTIL ")
	set(XCB_COMPONENTS_FOUND FALSE)
endif()

INCLUDE_DIRECTORIES(BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

# fail build if none of the platform backends can be found
if (NOT XCB_FOUND OR NOT XCB_COMPONENTS_FOUND)
    message(FATAL_ERROR "No suitable backend platform was found. Currenty supported platforms are: XCB Components Required: ${XCB_COMPONENTS_ERRORS}")
endif()

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_definitions("-Wno-mismatched-tags")
endif()

# ----
set(shell_SRCS
    shell.cpp
    desktop.cpp
    panel.cpp
    model.cpp
    tasks.cpp
    dash.cpp
    volume.cpp
    network.cpp
    date.cpp
    frame.cpp
    notificationsdialog.cpp
    logout.cpp
    winctrl.cpp
    trash.cpp
)

set(pamixer_SRCS
    pamixer/callbacks.cc
    pamixer/device.cc
    pamixer/pulseaudio.cc
)
    

set(shell_MOC_HDRS shell.h)
qt5_wrap_cpp(shell_MOC_SRCS ${shell_MOC_HDRS})

add_executable(qshell ${shell_SRCS} ${pamixer_SRCS} ${shell_MOC_HDRS})

target_link_libraries(qshell
    Qt5::Concurrent
    Qt5::DBus
    Qt5::X11Extras
    Qt5::Xml
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::Crash
    KF5::GlobalAccel
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::KIOWidgets
    KF5::KIONTLM
    KF5::I18n
    KF5::Service
    KF5::Solid
#     KF5::kcmutils
    KF5::WindowSystem
    ${X11_X11_LIB}
    ${XCB_LIBRARIES}
    pulse
    )

install(TARGETS qshell DESTINATION ${BIN_INSTALL_DIR} )
