
find_package(jsoncpp CONFIG REQUIRED)
find_package(directx-headers CONFIG REQUIRED)

add_library(RHI STATIC ${RHI_SOURCES})

file(GLOB_RECURSE RHI_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
)

target_sources(RHI PRIVATE ${RHI_SOURCES})

foreach(config IN ITEMS DEBUG RELWITHDEBINFO RELEASE)
    set_property(TARGET RHI PROPERTY LIBRARY_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET RHI PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET RHI PROPERTY RUNTIME_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}")
endforeach()

target_include_directories(RHI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(RHI PRIVATE ${EXTERNAL_DIR} ${CMAKE_SOURCE_DIR}/Source)
target_link_libraries(RHI PRIVATE Microsoft::DirectX-Headers JsonCpp::JsonCpp GPUCrashTracker Utility)
target_precompile_headers(RHI PRIVATE "RHI_PCH.h")

if (USE_D3D12)
    target_compile_definitions(RHI PRIVATE USE_D3D12=1)
endif()

if (ENABLE_GPU_PROFILING EQUAL TRUE)
    target_compile_definitions(RHI PUBLIC ENABLE_GPU_EVENTS=1)
endif()

if (MSVC)
    target_compile_options(RHI PRIVATE /W4)
endif()

include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(RHI)
