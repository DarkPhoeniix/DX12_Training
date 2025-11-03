
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin/}) 

find_package(spdlog CONFIG REQUIRED)

add_library(Logger STATIC ${Logger_SOURCES})

file(GLOB_RECURSE Logger_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
)

target_sources(Logger PRIVATE ${Logger_SOURCES})

target_include_directories(Logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Logger PRIVATE spdlog::spdlog)

include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(Logger)
