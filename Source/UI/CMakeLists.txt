
find_package(imgui CONFIG REQUIRED)

add_library(UI STATIC ${UI_SOURCES})

file(GLOB_RECURSE UI_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
)

target_sources(UI PRIVATE ${UI_SOURCES})

foreach(config IN ITEMS DEBUG RELWITHDEBINFO RELEASE)
    set_property(TARGET UI PROPERTY LIBRARY_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET UI PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET UI PROPERTY RUNTIME_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}")
endforeach()

target_include_directories(UI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(UI PRIVATE ${EXTERNAL_DIR} ${EXTERNAL_DIR}/ImGui ${CMAKE_SOURCE_DIR}/Source)
target_link_libraries(UI PRIVATE RHI Renderer Logger Utility imgui::imgui)

if (USE_D3D12)
    target_compile_definitions(UI PRIVATE USE_D3D12=1)
endif()

if (ENABLE_GPU_PROFILING EQUAL TRUE)
    target_compile_definitions(UI PUBLIC ENABLE_GPU_EVENTS=1)
endif()

if (MSVC)
    target_compile_options(UI PRIVATE /W4)
endif()

include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(UI)
