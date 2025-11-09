
find_package(imgui CONFIG REQUIRED)
find_package(directx-headers CONFIG REQUIRED)
find_package(winpixevent CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)

add_executable(Editor WIN32 ${Editor_SOURCES})

file(GLOB_RECURSE Editor_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
)

target_sources(Editor PRIVATE ${Editor_SOURCES})

foreach(config IN ITEMS DEBUG PROFILE RELEASE)
    set_property(TARGET Editor PROPERTY LIBRARY_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET Editor PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET Editor PROPERTY RUNTIME_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}")
endforeach()

set_target_properties(Editor PROPERTIES OUTPUT_NAME "Equinox Engine")
target_include_directories(Editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Editor PRIVATE ${EXTERNAL_DIR} ${EXTERNAL_DIR}/ImGui ${CMAKE_SOURCE_DIR}/Source)
target_compile_options(Editor PRIVATE /MP)
target_link_libraries(Editor PRIVATE 
    Microsoft::DirectX-Headers 
    Microsoft::WinPixEventRuntime 
    JsonCpp::JsonCpp
    imgui::imgui
    RenderGraph 
    Renderer 
    RHI 
    Logger 
    Utility 
    d3d12.lib 
    dxgi.lib 
    dxguid.lib
    dinput8.lib 
    pathcch.lib 
    d3dcompiler.lib)
target_precompile_headers(Editor PRIVATE "EditorPCH.h")

include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(Editor)
