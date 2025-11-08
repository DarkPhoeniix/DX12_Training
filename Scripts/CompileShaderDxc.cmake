
function(CompileShaders output_directory shader_path shader_type optimization_level)
    get_filename_component(shader_name ${shader_path} NAME_WE)
    message("Compiling shader: ${shader_name} as type: ${shader_type} with optimization: ${optimization_level}")
    
    message("dxc.exe -E main -T ${shader_type}_6_6 -${optimization_level} -Fo ${output_directory}/${shader_name}.cso ${shader_path}")
    add_custom_command(
        TARGET Renderer
        COMMAND "dxc.exe /E main /T ${shader_type}_6_6 -${optimization_level} /Fo ${output_directory}/${shader_name}.cso ${shader_path}"
        VERBATIM
    )
endfunction()