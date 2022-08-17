target_sources(opengl_engine PRIVATE 
"3rdparty/include/imgui/imgui.cpp"
"3rdparty/include/imgui/imgui_draw.cpp"
"3rdparty/include/imgui/imgui_tables.cpp"
"3rdparty/include/imgui/imgui_widgets.cpp"
"3rdparty/include/imgui/imgui_impl_opengl3.cpp"
"3rdparty/include/imgui/imgui_impl_glfw.cpp"
"3rdparty/include/imgui/implot.cpp"
"3rdparty/include/imgui/implot_items.cpp"
)


file(GLOB shared_libs "shared/*.dll")

foreach(lib IN LISTS shared_libs)
	get_filename_component(lib_filename ${lib} NAME)
	configure_file(${lib} ${CMAKE_BINARY_DIR}/src/${lib_filename} COPYONLY)
endforeach()