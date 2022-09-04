# some components
add_library(capture    core/capture.cpp   )
add_library(detection  core/detection.cpp )
add_library(display    core/display.cpp   )
add_library(imgprocess road/imgprocess.cpp)
add_library(sideline   road/_sideline.cpp )

include_directories(utils/logger/inc)
include_directories(road/elements)

# logger libs source
aux_source_directory(utils/logger/src LOGGER_LIBS)
set(LOGGER_LIBS ${LOGGER_LIBS})

# elements libs source
aux_source_directory(road/elements ELEMENTS_LIBS)
set(ELEMENTS_LIBS ${ELEMENTS_LIBS})

# compile to static lib
add_library(logger_libs ${LOGGER_LIBS})
add_library(elements_libs ${ELEMENTS_LIBS})
