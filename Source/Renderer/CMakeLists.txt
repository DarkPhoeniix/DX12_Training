
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin/}) 

find_package(directx-headers CONFIG REQUIRED)
find_package(winpixevent CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)

add_library(Renderer STATIC ${Renderer_SOURCES})

file(GLOB_RECURSE Renderer_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
)

target_sources(Renderer PRIVATE ${Renderer_SOURCES})

target_include_directories(Renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Renderer PRIVATE ${CMAKE_SOURCE_DIR}/Source)
target_compile_options(Renderer PRIVATE /MP)
target_link_libraries(Renderer PRIVATE Microsoft::DirectX-Headers Microsoft::WinPixEventRuntime JsonCpp::JsonCpp RenderGraph RHI Logger Utility)
target_precompile_headers(Renderer PRIVATE "RendererPCH.h")

include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(Renderer)
