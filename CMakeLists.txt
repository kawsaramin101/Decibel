cmake_minimum_required(VERSION 3.16)
project(Decibel VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Sql Multimedia Widgets)
qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appDecibel
    main.cpp
    appcontroller.cpp
    appcontroller.h
    database.cpp
    database.h
    models.h
    resources.qrc
)

qt_add_qml_module(appDecibel
    URI Decibel
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/BottomBox.qml
        qml/LeftBox.qml
        qml/RightBox.qml
        qml/TopBox.qml
        qml/PlayingIndicator.qml
)

set_target_properties(appDecibel PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appDecibel
    PRIVATE
        Qt6::Quick
        Qt6::Sql
        Qt6::Multimedia
        Qt6::Widgets
)

install(TARGETS appDecibel DESTINATION bin)

# CPack / .deb config
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "decibel")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_CONTACT "you@example.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple music player app for desktop")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6quick6 (>= 6.0), libqt6sql6 (>= 6.0), libqt6multimedia6 (>= 6.0), libqt6widgets6 (>= 6.0)")
set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/kawsaramin101/Decibel")

install(FILES decibel.desktop
    DESTINATION share/applications)
install(FILES decibel.svg
    DESTINATION share/icons/hicolor/256x256/apps)

include(CPack)