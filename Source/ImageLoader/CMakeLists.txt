
add_library(ImageLoader STATIC ${IMAGE_LOADER_SOURCES})

file(GLOB_RECURSE IMAGE_LOADER_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
)

target_sources(ImageLoader PRIVATE ${IMAGE_LOADER_SOURCES})

foreach(config IN ITEMS DEBUG RELWITHDEBINFO RELEASE)
    set_property(TARGET ImageLoader PROPERTY LIBRARY_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET ImageLoader PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET ImageLoader PROPERTY RUNTIME_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}")
endforeach()

target_include_directories(ImageLoader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ImageLoader PRIVATE ${EXTERNAL_DIR} ${CMAKE_SOURCE_DIR}/Source)

if (MSVC)
    target_compile_options(ImageLoader PRIVATE /W4)
endif()

include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(ImageLoader)
