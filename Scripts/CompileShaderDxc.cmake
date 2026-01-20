
function(CompileShader output_directory shader_path shader_type optimization_level debug_info)
    get_filename_component(shader_name ${shader_path} NAME_WE)
    message("Compiling shader: ${shader_name} as type: ${shader_type} with optimization: ${optimization_level}")

    add_custom_command(
        TARGET Renderer POST_BUILD
        COMMAND dxc.exe 
                /E main 
                /T ${shader_type}_6_6 
                /${optimization_level} 
                /Fo ${output_directory}/${shader_name}.cso ${shader_path}
                $<$<BOOL:${debug_info}>:/Zi> $<$<BOOL:${debug_info}>:/Qembed_debug>
        VERBATIM
    )
endfunction()
