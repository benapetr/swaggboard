cmake_minimum_required (VERSION 2.8.9)
PROJECT(swaggboard)
option(QT5_BUILD "Build sb using the Qt5 framework" TRUE)
SET(QT_USE_QTXML TRUE)
SET(QT_USE_MULTIMEDIA TRUE)
SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_AUTOMOC ON)
if(WIN32)
    if(MINGW)
	SET(CMAKE_CXX_FLAGS "-mwindows")
    endif()
endif()
#### Qt 4 and 5 ####
if(QT5_BUILD)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Multimedia REQUIRED)
    find_package(Qt5Xml REQUIRED)
    find_package(Qt5Widgets REQUIRED)
    set(QT_INCLUDES
        ${Qt5Gui_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5Network_INCLUDE_DIRS}
        ${Qt5Xml_INCLUDE_DIRS}
        ${Qt5Multimedia_INCLUDE_DIRS}
    )
    include_directories(${QT_INCLUDES})
else()
    find_package(Qt4 REQUIRED)
endif()

file(GLOB srcx
    "*.cpp"
)
file (GLOB resx
    "*.qrc"
)
file (GLOB uixx
    "*.ui"
)

SET(sb_SOURCES ${srcx})
SET(sb_FORMS ${uixx})
SET(sb_RESOURCES ${resx})
if(QT5_BUILD)
    QT5_WRAP_CPP(sb_HEADERS_MOC ${sb_HEADERS})
    QT5_WRAP_UI(sb_FORMS_HEADERS ${sb_FORMS})
    QT5_ADD_RESOURCES(sb_RESOURCES_RCC ${sb_RESOURCES})
else()
    QT4_WRAP_CPP(sb_HEADERS_MOC ${sb_HEADERS})
    QT4_WRAP_UI(sb_FORMS_HEADERS ${sb_FORMS})
    QT4_ADD_RESOURCES(sb_RESOURCES_RCC ${sb_RESOURCES})
endif()
if (NOT QT5_BUILD)
  INCLUDE(${QT_USE_FILE})
endif()
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS( -DQT_USE_QSTRINGBUILDER )
ADD_EXECUTABLE(swaggboard ${sb_SOURCES} ${sb_HEADERS_MOC} ${sb_FORMS_HEADERS} ${sb_RESOURCES_RCC})
TARGET_LINK_LIBRARIES(swaggboard ${QT_LIBRARIES})
if (WIN32)
    TARGET_LINK_LIBRARIES(swaggboard dbghelp)
endif()
if (QT5_BUILD)
    qt5_use_modules(swaggboard Core Multimedia Gui Widgets Xml)
	if (HUGGLE_EXT OR LIBCORE)
		qt5_use_modules(core Core Gui Widgets Xml)
	endif()
endif()
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
