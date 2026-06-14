
find_package(jsoncpp CONFIG REQUIRED)

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
add_library(Renderer STATIC ${Renderer_SOURCES})

file(GLOB_RECURSE Renderer_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
)

file(GLOB_RECURSE Renderer_ALL_SHADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.hlsl ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.hlsli)
file(GLOB_RECURSE Renderer_ALL_SRC_SHADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.hlsl)
file(GLOB_RECURSE Renderer_VS_SHADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*_vs.hlsl)
file(GLOB_RECURSE Renderer_GS_SHADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*_gs.hlsl)
file(GLOB_RECURSE Renderer_PS_SHADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*_ps.hlsl)
file(GLOB_RECURSE Renderer_CS_SHADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*_cs.hlsl)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    foreach(ShaderFile ${Renderer_ALL_SRC_SHADERS})
        foreach(config IN ITEMS Debug RelWithDebInfo Release)
            string(TOUPPER ${config} CONFIG_UPPER)
            set_source_files_properties(${ShaderFile} PROPERTIES
                VS_SHADER_OBJECT_FILE_NAME_DEBUG
                "${EXE_OUTPUT_DIR}/Shaders/${ShaderName}.cso"
            )
        endforeach()
    endforeach(ShaderFile)

    set_source_files_properties(${Renderer_ALL_SHADERS} PROPERTIES VS_SHADER_MODEL "6.6")
    set_source_files_properties(${Renderer_VS_SHADERS} PROPERTIES VS_SHADER_TYPE "Vertex")
    set_source_files_properties(${Renderer_GS_SHADERS} PROPERTIES VS_SHADER_TYPE "Geometry")
    set_source_files_properties(${Renderer_PS_SHADERS} PROPERTIES VS_SHADER_TYPE "Pixel")
    set_source_files_properties(${Renderer_CS_SHADERS} PROPERTIES VS_SHADER_TYPE "Compute")
endif()

file(GLOB_RECURSE Renderer_PIPELINES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/PipelineDescriptions/*.tech 
    ${CMAKE_CURRENT_SOURCE_DIR}/PipelineDescriptions/*.depth 
    ${CMAKE_CURRENT_SOURCE_DIR}/PipelineDescriptions/*.raster 
    ${CMAKE_CURRENT_SOURCE_DIR}/PipelineDescriptions/*.blend
)

target_sources(Renderer PRIVATE ${Renderer_SOURCES} ${Renderer_ALL_SHADERS} ${Renderer_PIPELINES})

foreach(config IN ITEMS DEBUG RELWITHDEBINFO RELEASE)
    set_property(TARGET Renderer PROPERTY LIBRARY_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET Renderer PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET Renderer PROPERTY RUNTIME_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}")
endforeach()

target_include_directories(Renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Renderer PRIVATE ${CMAKE_SOURCE_DIR}/Source)
target_compile_options(Renderer PRIVATE /MP)
target_link_libraries(Renderer PRIVATE JsonCpp::JsonCpp RenderGraph RHI Logger Utility ImageLoader)
target_precompile_headers(Renderer PRIVATE "RendererPCH.h")

if (ENABLE_CPU_PROFILING)
    target_compile_definitions(Renderer PUBLIC ENABLE_CPU_PROFILING)
endif()

if (ENABLE_GPU_PROFILING)
    target_compile_definitions(Renderer PUBLIC ENABLE_GPU_PROFILING ENABLE_GPU_EVENTS)
endif()

include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(Renderer)

add_custom_command(TARGET Renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${EXE_OUTPUT_DIR}/PipelineDescriptions/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/PipelineDescriptions" "${EXE_OUTPUT_DIR}/PipelineDescriptions"
    COMMENT "Copying ${SOURCE_FILE} to output directory"
)

add_custom_command(TARGET Renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${EXE_OUTPUT_DIR}/Shaders"
)

set(SHADER_DEBUG_INFO FALSE)
set(SHADER_DEBUG_INFO $<$<config:Debug,RelWithDebInfo>:TRUE>)

if (MSVC)
    set_property(SOURCE ${Renderer_CS_SHADERS} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")
    set_property(SOURCE ${Renderer_VS_SHADERS} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")
    set_property(SOURCE ${Renderer_GS_SHADERS} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")
    set_property(SOURCE ${Renderer_PS_SHADERS} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")
endif()

include(${CMAKE_SOURCE_DIR}/Scripts/CompileShaderDxc.cmake)
foreach(ShaderFile ${Renderer_VS_SHADERS})
    CompileShader("${EXE_OUTPUT_DIR}/Shaders" "${ShaderFile}" "vs" "O3" SHADER_DEBUG_INFO)
endforeach()

foreach(ShaderFile ${Renderer_GS_SHADERS})
    CompileShader("${EXE_OUTPUT_DIR}/Shaders" "${ShaderFile}" "gs" "O3" SHADER_DEBUG_INFO)
endforeach()

foreach(ShaderFile ${Renderer_PS_SHADERS})
    CompileShader("${EXE_OUTPUT_DIR}/Shaders" "${ShaderFile}" "ps" "O3" SHADER_DEBUG_INFO)
endforeach()

foreach(ShaderFile ${Renderer_CS_SHADERS})
    CompileShader("${EXE_OUTPUT_DIR}/Shaders" "${ShaderFile}" "cs" "O3" SHADER_DEBUG_INFO)
endforeach()
