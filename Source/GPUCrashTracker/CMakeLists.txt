
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin/}) 

add_library(GPUCrashTracker STATIC ${GPUCrashTracker_SOURCES})

file(GLOB_RECURSE GPUCrashTracker_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
)

target_sources(GPUCrashTracker PRIVATE ${GPUCrashTracker_SOURCES})

if (USE_AFTERMATH)
    add_definitions(-DUSE_AFTERMATH)
endif()

target_include_directories(GPUCrashTracker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(GPUCrashTracker PRIVATE "${CMAKE_SOURCE_DIR}/External" ${CMAKE_SOURCE_DIR}/Source)
target_link_libraries(GPUCrashTracker PRIVATE Aftermath)

include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(GPUCrashTracker)
