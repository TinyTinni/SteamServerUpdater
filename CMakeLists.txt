cmake_minimum_required (VERSION 3.5.1)
project (SteamServerUpdater)

#set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
set (EXECUTABLE_NAME "SteamServerUpdater")

#include(cmake//SourceGrouper.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#set cmake prefix path. It provides a finder
#if (WIN32)
#    set (QT5_ROOT_PATH "" CACHE PATH "Path to Qt5 root directory (bin/include/lib)")
#    if (EXISTS "${QT5_ROOT_PATH}")
#        set (CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${QT5_ROOT_PATH}")
#        set (QT5_INSTALL_PATH_EXISTS TRUE)
#    else()
#        message(FATAL_ERROR "Could not find root directoy (bin/include/lib) of Qt5. Please specify.")
#    endif()
#endif(WIN32)
#set(CMAKE_AUTOMOC ON)
#find_package(Qt5 COMPONENTS Core Network)


if (NOT Boost_LIBRARY_DIR)
	message(FATAL_ERROR "Could not find Boost_LIBRARY_DIR. Please specify")
endif()
link_directories(${Boost_LIBRARY_DIR})
find_package(Boost REQUIRED)

include(SourceFiles.cmake)

set(SSU_Sources ${sources})

add_executable(${EXECUTABLE_NAME} ${SSU_Sources})

include_directories(${Boost_INCLUDE_DIR})
#include_directories(Qt5::Core)

if (WIN32)
    add_definitions("-D_WIN32_WINNT=0x0501;WIN32_LEAN_AND_MEAN")
endif()

#target_link_libraries(${EXECUTABLE_NAME} Qt5::Core Qt5::Network)

