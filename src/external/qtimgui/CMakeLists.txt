CMAKE_MINIMUM_REQUIRED(VERSION 3.8.1)
PROJECT(qtimgui)

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_AUTOUIC ON)
FIND_PACKAGE(Qt5 COMPONENTS Core Quick Gui Widgets Svg REQUIRED)

# imgui library: imgui is build by default, but you can
# provide your own version by setting QTIMGUI_BUILD_IMGUI to OFF
OPTION(QTIMGUI_BUILD_IMGUI "Use imgui provided as qtimgui submodule" ON)
IF(QTIMGUI_BUILD_IMGUI)
    ADD_LIBRARY(imgui
      STATIC
      imgui/imconfig.h
      imgui/imgui_demo.cpp
      imgui/imgui_draw.cpp
      imgui/imgui_internal.h
      imgui/imgui_widgets.cpp
      imgui/imgui.cpp
    )
    TARGET_INCLUDE_DIRECTORIES(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
ENDIF(QTIMGUI_BUILD_IMGUI)

SET(qt_imgui_sources ImGuiRenderer.h ImGuiRenderer.cpp QtImGui.h QtImGui.cpp)

# qt_imgui_quick: library with a qt renderer for Qml / QtQuick applications
ADD_LIBRARY(qt_imgui_quick STATIC ${qt_imgui_sources})
TARGET_INCLUDE_DIRECTORIES(qt_imgui_quick PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(
    qt_imgui_quick 
    PUBLIC
    imgui
    Qt5::Core Qt5::Quick Qt5::Svg
    )

IF(ANDROID)
    TARGET_LINK_LIBRARIES(qt_imgui_quick PUBLIC log dl GLESv2 z)
ENDIF()

# qt_imgui_widget: library with a qt renderer for classic Qt Widget applications
ADD_LIBRARY(qt_imgui_widgets STATIC ${qt_imgui_sources})
TARGET_INCLUDE_DIRECTORIES(qt_imgui_widgets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(
    qt_imgui_widgets
    PUBLIC
    imgui
    Qt5::Core Qt5::Widgets Qt5::Svg
    )
IF(ANDROID)
    TARGET_LINK_LIBRARIES(qt_imgui_widgets PUBLIC log dl GLESv2 z)
ENDIF()
TARGET_COMPILE_DEFINITIONS(qt_imgui_widgets PUBLIC QT_WIDGETS_LIB)

OPTION(BUILD_IMGUI_QT_DEMO_WINDOW "Include demo-window Project" OFF)
IF(NOT BUILD_IMGUI_QT_DEMO_WINDOW)
  MESSAGE(STATUS "Skipping demo-window Project")
ELSE()
	ADD_SUBDIRECTORY(demo-window)
ENDIF(NOT BUILD_IMGUI_QT_DEMO_WINDOW)

OPTION(BUILD_IMGUI_QT_DEMO_WIDGET "Include demo-widget Project" OFF)
IF(NOT BUILD_IMGUI_QT_DEMO_WIDGET)
  MESSAGE(STATUS "Skipping demo-widget Project")
ELSE()
	ADD_SUBDIRECTORY(demo-widget)
ENDIF(NOT BUILD_IMGUI_QT_DEMO_WIDGET)