cmake_minimum_required(VERSION 3.16)

project(streamlights VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick)

include(FetchContent)

FetchContent_Declare(
    TwitchQt
    GIT_REPOSITORY "https://github.com/g-fb/TwitchQt"
    GIT_TAG "master"
    )
FetchContent_MakeAvailable(TwitchQt)

qt_standard_project_setup()

qt_add_executable(appstreamlights
    main.cpp
)

qt_add_qml_module(appstreamlights
    URI streamlights
    VERSION 1.0
    QML_FILES
        Main.qml
        SOURCES wizcontroller.h wizcontroller.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appstreamlights PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appstreamlights
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

add_dependencies(appstreamlights TwitchQt)

target_link_libraries(appstreamlights
    PRIVATE Qt6::Quick TwitchQt
)

target_include_directories(appstreamlights PRIVATE ${TwitchQt_BINARY_DIR}/src)

include(GNUInstallDirs)
install(TARGETS appstreamlights
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
