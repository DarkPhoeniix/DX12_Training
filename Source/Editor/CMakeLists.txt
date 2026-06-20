# =============================================================================
# Editor - the engine application (Equinox Engine executable)
# =============================================================================

# --- Dependencies ------------------------------------------------------------
find_package(imgui CONFIG REQUIRED)

set(CMAKE_AUTORCC ON)

if(MSVC)
    add_compile_options(/MP /utf-8)
    add_compile_definitions(
        UNICODE
        _UNICODE
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        "SOLUTION_DIR=R\"(${CMAKE_SOURCE_DIR})\""
        "SOLUTION_DIR_W=LR\"(${CMAKE_SOURCE_DIR})\""
    )
endif()

# --- Sources -----------------------------------------------------------------
file(GLOB_RECURSE Editor_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/*.rc
)

# --- Target ------------------------------------------------------------------
add_executable(Editor WIN32 ${Editor_SOURCES})
set_target_properties(Editor PROPERTIES OUTPUT_NAME "Equinox Engine")

# --- Output directories ------------------------------------------------------
foreach(config IN ITEMS DEBUG RELWITHDEBINFO RELEASE)
    set_property(TARGET Editor PROPERTY LIBRARY_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET Editor PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET Editor PROPERTY RUNTIME_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}")
endforeach()

# --- Includes / links / PCH --------------------------------------------------
target_include_directories(Editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Editor PRIVATE ${EXTERNAL_DIR} ${EXTERNAL_DIR}/ImGui ${CMAKE_SOURCE_DIR}/Source)
target_compile_options(Editor PRIVATE /MP)

target_link_libraries(Editor PRIVATE
    # Engine modules
    RenderGraph
    Renderer
    RHI
    Logger
    UI
    Utility
    # Third-party (only what Editor's own sources include directly)
    imgui::imgui
)

if (WIN32)
    # Windows system libraries (backend-agnostic)
    target_link_libraries(Editor PRIVATE
        dxguid.lib
        dinput8.lib
        pathcch.lib
    )
endif()

if (USE_D3D12)
    target_link_libraries(Editor PRIVATE
        d3d12.lib
        dxgi.lib
        d3dcompiler.lib
    )
endif()

target_precompile_headers(Editor PRIVATE "EditorPCH.h")

# --- IDE source grouping -----------------------------------------------------
include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(Editor)

# --- Asset deployment --------------------------------------------------------
add_custom_command(TARGET Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${EXE_OUTPUT_DIR}/Assets/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_SOURCE_DIR}/Saved/Assets" "${EXE_OUTPUT_DIR}/Assets"
    COMMENT "Copying assets to output directory"
)
