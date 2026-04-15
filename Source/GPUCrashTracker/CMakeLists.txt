
if (USE_D3D12)
    add_library(GPUCrashTracker STATIC ${GPUCrashTracker_COMMON_SOURCES} ${GPUCrashTracker_D3D12_SOURCES})
else()
    add_library(GPUCrashTracker STATIC ${GPUCrashTracker_COMMON_SOURCES})
endif()


file(GLOB_RECURSE GPUCrashTracker_COMMON_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/ICommandListCrashContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ICommandListCrashContext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/IGPUCrashTracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IGPUCrashTracker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/NullCrashTracker/NullCommandListCrashContext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/NullCrashTracker/NullCrashTracker.h
)

file(GLOB_RECURSE GPUCrashTracker_D3D12_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/D3D12CrashTracker/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/D3D12CrashTracker/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/D3D12CrashTracker/*.inl
)

if (USE_D3D12)
    target_sources(GPUCrashTracker PRIVATE ${GPUCrashTracker_COMMON_SOURCES} ${GPUCrashTracker_D3D12_SOURCES})
else()
    target_sources(GPUCrashTracker PRIVATE ${GPUCrashTracker_COMMON_SOURCES})
endif()

foreach(config IN ITEMS DEBUG RELWITHDEBINFO RELEASE)
    set_property(TARGET GPUCrashTracker PROPERTY LIBRARY_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET GPUCrashTracker PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET GPUCrashTracker PROPERTY RUNTIME_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}")
endforeach()

target_include_directories(GPUCrashTracker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(GPUCrashTracker PRIVATE "${CMAKE_SOURCE_DIR}/External" ${CMAKE_SOURCE_DIR}/Source)

if (ENABLE_NSIGHT_AFTERMATH)
    target_compile_definitions(GPUCrashTracker PRIVATE USE_NSIGHT_AFTERMATH)
    target_link_libraries(GPUCrashTracker PRIVATE Aftermath)

    add_custom_command(TARGET GPUCrashTracker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${AFTERMATH_DIR}/lib/x64/GFSDK_Aftermath_Lib.x64.dll"
        "${EXE_OUTPUT_DIR}"
    )
endif()

include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(GPUCrashTracker)
