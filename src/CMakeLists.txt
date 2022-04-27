cmake_minimum_required(VERSION 3.5)

project(cmake_issue_22531 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(UI_FILES
    mainwindow.ui
    widget1.ui
    widget2.ui
)

# qt_wrap_ui(UIC_FILES ${UI_FILES})

set(HEADER_FILES
    mainwindow.h
    widget1.h
    widget2.h
)

# qt_wrap_cpp(MOC_FILES ${HEADER_FILES})

set(SOURCE_FILES
    main.cpp
    mainwindow.cpp
    widget1.cpp
    widget2.cpp
)

add_executable(example
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${UI_FILES}
#    ${MOC_FILES}
#    ${UIC_FILES}
)

target_link_libraries(example PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(example PROPERTIES
    WIN32_EXECUTABLE TRUE
)
