cmake_minimum_required(VERSION 2.8.11)
project(WebCamCap)

set(CMAKE_CXX_FLAGS "-m64 -pipe -pedantic -Wall -Wextra -O2 -Wall -W -D_REENTRANT ")

#c++11 support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

ADD_DEFINITIONS(${QT_DEFINITIONS})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(HW_GUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Gui)
subdirs(Gui)

include_directories(QT_INCLUDE_DIR ${HW_GUI_DIR} ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

FIND_PACKAGE(Qt5Core)
FIND_PACKAGE(Qt5Widgets)
FIND_PACKAGE(Qt5OpenGL)
FIND_PACKAGE(Qt5Network)
FIND_PACKAGE(Qt5Gui)
FIND_PACKAGE(Qt5Concurrent)
FIND_PACKAGE(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

file(GLOB_RECURSE SRC
    "*.h"
    "*.cpp"
)

QT5_WRAP_UI(UIS
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/addcamera.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/addproject.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/animplayer.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/camwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/mainwindow.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/structureeditor.ui
)

QT5_WRAP_CPP(CPPS

    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/addcamera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/addproject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/animplayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/camwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/mainwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/structureeditor.h
)

ADD_EXECUTABLE(${PROJECT_NAME} main.cpp ${SRC} ${UIS})

qt5_use_modules(${PROJECT_NAME} Widgets Core Gui OpenGL Concurrent Network)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
