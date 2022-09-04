cmake_minimum_required(VERSION 3.4...3.18)
project(YYDS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-g -Wall")

#---------------------------------------------------------------------
#       [ Include、Lib  : Define ] ==> []
#---------------------------------------------------------------------

find_package(PkgConfig)
pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})

find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )

find_package(PaddleLite QUIET)
include_directories(${PADDLELITE_INCLUDE_DIR})
LINK_DIRECTORIES("/usr/local/lib/paddle_lite/")

#---------------------------------------------------------------------
#               [ bin ] ==> [ demo ]
#---------------------------------------------------------------------

# params
set(YYDS "yyds")
set(INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/_inc/common
    ${PROJECT_SOURCE_DIR}/_inc/core
    ${PROJECT_SOURCE_DIR}/_inc/road
    ${PROJECT_SOURCE_DIR}/_inc/utils
)

# include
include_directories(
    ${INCLUDE_DIRS}
)

# sub directory
add_subdirectory(${PROJECT_SOURCE_DIR}/_inc)

# auto find all source in _src
aux_source_directory(_src YYDS_SOURCES)

# to exe file
add_executable(
    ${YYDS}
    ${YYDS_SOURCES}
)

# add link some components
target_link_libraries(${YYDS} PRIVATE capture)
target_link_libraries(${YYDS} PRIVATE detection)
target_link_libraries(${YYDS} PRIVATE display)
target_link_libraries(${YYDS} PRIVATE imgprocess)
target_link_libraries(${YYDS} PRIVATE sideline)

# add link some libs
target_link_libraries(${YYDS} PRIVATE elements_libs)
target_link_libraries(${YYDS} PRIVATE logger_libs)
target_link_libraries(${YYDS} PRIVATE pthread)
target_link_libraries(${YYDS} PRIVATE serial)
target_link_libraries(${YYDS} PRIVATE paddle_full_api_shared)
target_link_libraries(${YYDS} PRIVATE ${OpenCV_LIBS})
target_link_libraries(${YYDS} PRIVATE ${PADDLELITE_LIBRARY})
