
# The dxc.exe shipped with the Windows SDK is built without the SPIR-V backend, so the
# compiler is taken from the Vulkan SDK instead of whatever comes first on PATH
find_package(Vulkan REQUIRED COMPONENTS dxc dxc_exe)

# Register-to-binding mapping expected by rhi::vulkan::VulkanPipelineState:
#   set 0, binding 0     - b0, frame constants
#   set 0, binding 1..13 - s0..s12, static samplers
#   set 1, binding 0     - ResourceDescriptorHeap, emulated through descriptor indexing
set(SPIRV_BINDING_FLAGS
    -fvk-b-shift 0 0
    -fvk-s-shift 1 0
    -fvk-bind-resource-heap 0 1
)

function(CompileShaderSpirv output_directory shader_path shader_type optimization_level debug_info)
    get_filename_component(shader_name ${shader_path} NAME_WE)
    message("Compiling shader: ${shader_name} as type: ${shader_type} to SPIR-V with optimization: ${optimization_level}")

    add_custom_command(
        TARGET Renderer POST_BUILD
        COMMAND ${Vulkan_dxc_EXECUTABLE}
                -spirv
                -fspv-target-env=vulkan1.3
                -fvk-use-dx-layout
                ${SPIRV_BINDING_FLAGS}
                -E main
                -T ${shader_type}_6_6
                -${optimization_level}
                -Fo ${output_directory}/${shader_name}.spv ${shader_path}
                $<$<BOOL:${debug_info}>:-Zi>
        VERBATIM
    )
endfunction()
