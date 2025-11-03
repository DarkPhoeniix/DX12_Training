
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin/}) 

add_library(Utility STATIC ${Utility_SOURCES})

file(GLOB_RECURSE Utility_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
)

target_sources(Utility PRIVATE ${Utility_SOURCES})

target_include_directories(Utility PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(Utility)
