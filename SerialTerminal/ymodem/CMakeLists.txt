project(YModem)

set(SOURCES
    ymodem.cpp
    ymodem.h
)

add_library(YModem ${SOURCES})


# include target
#target_include_directories(YModem PUBLIC &{CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(YModem Qt6::Core)
