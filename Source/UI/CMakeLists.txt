# =============================================================================
# UI - UI layer over Dear ImGui
# =============================================================================

# --- Dependencies ------------------------------------------------------------
find_package(imgui CONFIG REQUIRED)

# --- Sources -----------------------------------------------------------------
file(GLOB_RECURSE UI_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
)

# --- Target ------------------------------------------------------------------
add_library(UI STATIC ${UI_SOURCES})

# --- Output directories ------------------------------------------------------
foreach(config IN ITEMS DEBUG RELWITHDEBINFO RELEASE)
    set_property(TARGET UI PROPERTY LIBRARY_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET UI PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET UI PROPERTY RUNTIME_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}")
endforeach()

# --- Includes / links --------------------------------------------------------
target_include_directories(UI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(UI PRIVATE ${EXTERNAL_DIR} ${EXTERNAL_DIR}/ImGui ${CMAKE_SOURCE_DIR}/Source)
target_link_libraries(UI PRIVATE RHI Renderer Logger Utility imgui::imgui)

# --- Build options -----------------------------------------------------------
if (USE_D3D12)
    target_compile_definitions(UI PRIVATE USE_D3D12=1)
endif()

target_compile_definitions(UI PUBLIC $<$<NOT:$<CONFIG:Release>>:ENABLE_GPU_EVENTS=1>)

if (MSVC)
    target_compile_options(UI PRIVATE /W4)
endif()

# --- IDE source grouping -----------------------------------------------------
include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(UI)
