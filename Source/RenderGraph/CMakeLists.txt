
find_package(directx-headers CONFIG REQUIRED)

add_library(RenderGraph STATIC ${RenderGraph_SOURCES})

file(GLOB_RECURSE RenderGraph_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
)

target_sources(RenderGraph PRIVATE ${RenderGraph_SOURCES})

foreach(config IN ITEMS DEBUG PROFILE RELEASE)
    set_property(TARGET RenderGraph PROPERTY LIBRARY_OUTPUT_DIRECTORY_${config} "${LIB_OUTPUT_DIR}")
    set_property(TARGET RenderGraph PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${config} "${LIB_OUTPUT_DIR}")
    set_property(TARGET RenderGraph PROPERTY RUNTIME_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}")
endforeach()

target_include_directories(RenderGraph PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(RenderGraph PRIVATE ${EXTERNAL_DIR} ${CMAKE_SOURCE_DIR}/Source)
target_compile_options(RenderGraph PRIVATE /MP)
target_link_libraries(RenderGraph PRIVATE Microsoft::DirectX-Headers Logger RHI Utility)
target_precompile_headers(RenderGraph PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/RenderGraphPCH.h")

include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(RenderGraph)
