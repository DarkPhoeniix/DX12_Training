# =============================================================================
# RHI - Render Hardware Interface
# Backend-agnostic rendering abstraction with pluggable D3D12 / Vulkan backends
# =============================================================================

# --- Dependencies ------------------------------------------------------------
# Cross-platform dependencies
find_package(jsoncpp CONFIG REQUIRED)

# --- Target ------------------------------------------------------------------
# Collect all sources recursively. CONFIGURE_DEPENDS re-globs on rebuild so
# newly added files are picked up without re-running CMake by hand
file(GLOB_RECURSE RHI_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
)

add_library(RHI STATIC ${RHI_SOURCES})

# --- Output directories ------------------------------------------------------
foreach(config IN ITEMS DEBUG RELWITHDEBINFO RELEASE)
    set_property(TARGET RHI PROPERTY LIBRARY_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET RHI PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET RHI PROPERTY RUNTIME_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}")
endforeach()

# --- Includes / links / PCH --------------------------------------------------
target_include_directories(RHI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(RHI PRIVATE ${EXTERNAL_DIR} ${CMAKE_SOURCE_DIR}/Source)
target_link_libraries(RHI PRIVATE JsonCpp::JsonCpp GPUCrashTracker Utility)
target_precompile_headers(RHI PRIVATE "RHI_PCH.h")

# --- Backends ----------------------------------------------------------------
# D3D12 backend (Windows only)
if (WIN32 AND USE_D3D12)
    find_package(winpixevent CONFIG REQUIRED)
    find_package(D3D12MemoryAllocator CONFIG REQUIRED)
    find_package(directx-headers CONFIG REQUIRED)
    target_link_libraries(RHI PRIVATE
        Microsoft::WinPixEventRuntime
        Microsoft::DirectX-Headers
        GPUOpen::D3D12MemoryAllocator
    )
    target_compile_definitions(RHI PRIVATE USE_D3D12=1)
endif()

# Vulkan backend
if (USE_VULKAN)
    find_package(Vulkan REQUIRED)
    find_package(VulkanMemoryAllocator CONFIG REQUIRED)
    target_link_libraries(RHI PRIVATE
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
    )
    # VMA must see the same configuration in every translation unit that includes its header,
    # not just in the one providing the implementation. Entry points come from the vulkan.hpp
    # dynamic dispatcher, so the static ones are off
    target_compile_definitions(RHI PRIVATE
        USE_VULKAN=1
        VMA_STATIC_VULKAN_FUNCTIONS=0
        VMA_DYNAMIC_VULKAN_FUNCTIONS=1
    )
endif()

# --- Build options -----------------------------------------------------------
target_compile_definitions(RHI PUBLIC $<$<NOT:$<CONFIG:Release>>:ENABLE_GPU_EVENTS=1>)

if (MSVC)
    target_compile_options(RHI PRIVATE /W4)
endif()

# --- IDE source grouping -----------------------------------------------------
include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(RHI)
