cmake_minimum_required (VERSION 3.5.1)
project (SteamServerUpdater)

#set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
set (EXECUTABLE_NAME "SteamServerUpdater")

include(cmake//SourceGrouper.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#set cmake prefix path. It provides a finder
if (WIN32)
    set (QT5_ROOT_PATH "" CACHE PATH "Path to Qt5 root directory (bin/include/lib)")
    if (EXISTS "${QT5_ROOT_PATH}")
        set (CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${QT5_ROOT_PATH}")
        set (QT5_INSTALL_PATH_EXISTS TRUE)
    else()
        message(FATAL_ERROR "Could not find root directoy (bin/include/lib) of Qt5. Please specify.")
    endif()
endif(WIN32)

find_package(Qt5Core)

include(SteamServerUpdater//SourceFiles.cmake)

set(SSU_Sources ${sources})

add_executable(${EXECUTABLE_NAME} ${SSU_Sources})

target_link_libraries(${EXECUTABLE_NAME} Qt5::Core)

