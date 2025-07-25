file(GLOB_RECURSE model_files CONFIGURE_DEPENDS "models/*")
file(
  GLOB_RECURSE
  shader_files
  CONFIGURE_DEPENDS
  "shaders/*.vert"
  "shaders/*.frag"
  "shaders/*.rgen"
  "shaders/*.rint"
  "shaders/*.rchit"
  "shaders/*.rmiss")
file(GLOB_RECURSE shader_extra_files CONFIGURE_DEPENDS "shaders/*.glsl")
file(GLOB_RECURSE texture_files CONFIGURE_DEPENDS "textures/*")

foreach(shader ${shader_files})
  get_filename_component(file_name ${shader} NAME)
  get_filename_component(full_path ${shader} ABSOLUTE)
  set(output_dir ${CMAKE_CURRENT_BINARY_DIR}/shaders)
  set(output_file ${output_dir}/${file_name}.spv)
  set(compiled_shaders ${compiled_shaders} ${output_file})
  set(compiled_shaders
      ${compiled_shaders}
      PARENT_SCOPE)
  set_source_files_properties(${shader} PROPERTIES HEADER_FILE_ONLY TRUE)
  add_custom_command(
    OUTPUT ${output_file}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}"
    COMMAND Vulkan::glslangValidator --target-env vulkan1.3 -v ${full_path} -o
            ${output_file}
    DEPENDS ${full_path})
endforeach()

macro(copy_assets asset_files dir_name copied_files)
  foreach(asset ${${asset_files}})
    file(RELATIVE_PATH relative_path ${CMAKE_CURRENT_SOURCE_DIR}/ ${asset})
    get_filename_component(full_path ${asset} ABSOLUTE)
    set(output_dir ${CMAKE_CURRENT_BINARY_DIR}/)
    set(output_file ${output_dir}/${relative_path})
    set(${copied_files} ${${copied_files}} ${output_file})
    set(${copied_files}
        ${${copied_files}}
        PARENT_SCOPE)
    set_source_files_properties(${asset} PROPERTIES HEADER_FILE_ONLY TRUE)
    if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
      add_custom_command(
        OUTPUT ${output_file}
        COMMAND xcopy \"${full_path}\" \"${output_file}*\" /Y /Q /F
        DEPENDS ${full_path})
    else()
      add_custom_command(
        OUTPUT ${output_file}
        COMMAND mkdir --parents ${output_dir} && cp --force --link
                \"${full_path}\" \"${output_file}\"
        DEPENDS ${full_path})
    endif()
  endforeach()
endmacro()

copy_assets(model_files models copied_models)
copy_assets(texture_files textures copied_textures)

source_group("Models" FILES ${model_files})
source_group("Shaders" FILES ${shader_files} ${shader_extra_files})
source_group("Textures" FILES ${texture_files})

add_custom_target(
  Assets
  DEPENDS ${copied_models} ${compiled_shaders} ${copied_textures}
  SOURCES ${model_files} ${shader_files} ${shader_extra_files} ${texture_files})
