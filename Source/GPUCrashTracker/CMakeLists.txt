
add_library(GPUCrashTracker STATIC ${GPUCrashTracker_SOURCES})

file(GLOB_RECURSE GPUCrashTracker_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
)

target_sources(GPUCrashTracker PRIVATE ${GPUCrashTracker_SOURCES})

foreach(config IN ITEMS DEBUG PROFILE RELEASE)
    set_property(TARGET GPUCrashTracker PROPERTY LIBRARY_OUTPUT_DIRECTORY_${config} "${LIB_OUTPUT_DIR}")
    set_property(TARGET GPUCrashTracker PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${config} "${LIB_OUTPUT_DIR}")
    set_property(TARGET GPUCrashTracker PROPERTY RUNTIME_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}")
endforeach()

target_include_directories(GPUCrashTracker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(GPUCrashTracker PRIVATE "${CMAKE_SOURCE_DIR}/External" ${CMAKE_SOURCE_DIR}/Source)
target_link_libraries(GPUCrashTracker PRIVATE Aftermath)

if (USE_AFTERMATH)
    target_compile_definitions(GPUCrashTracker PRIVATE $<$<CONFIG:Release>:USE_AFTERMATH>)
endif()

include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(GPUCrashTracker)
