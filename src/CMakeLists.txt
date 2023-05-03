add_executable(opengl_engine 
"main.cpp" 
"3rdparty/glad.c"
"3rdparty/include/glm/detail/glm.cpp"
"3rdparty/include/imgui/imgui.cpp"
"3rdparty/include/imgui/imgui_demo.cpp"
"3rdparty/include/imgui/imgui_draw.cpp"
"3rdparty/include/imgui/imgui_impl_glfw.cpp"
"3rdparty/include/imgui/imgui_impl_opengl3.cpp"
"3rdparty/include/imgui/imgui_stdlib.cpp"
"3rdparty/include/imgui/imgui_tables.cpp"
"3rdparty/include/imgui/imgui_widgets.cpp"
"3rdparty/include/imgui/implot.cpp"
"3rdparty/include/imgui/implot_demo.cpp"
"3rdparty/include/imgui/implot_items.cpp"
"engine/camera/camera.cpp"
"engine/controller/controller.cpp"
"engine/controller/keyboard/keyboard.cpp"
"engine/engine.cpp"
"engine/gpu/buffers/buffer.cpp"
"engine/gpu/buffers/ubo.cpp"
"engine/gpu/shaderprogram/shader.cpp"
"engine/gpu/shaderprogram/shader_manager.cpp"
"engine/gpu/shaderprogram/shader_template.cpp"
"engine/gpu/texture/texture.cpp"
"engine/gui/gui.cpp"
"engine/gui/render_graph.cpp"
"engine/renderer/renderer.cpp"
"engine/window/window.cpp"
"engine/gpu/framebuffer/framebuffer.cpp"  
"engine/scene/scene.cpp" "engine/gpu/resource_manager/gpu_res_mgr.cpp")

set_property(TARGET opengl_engine PROPERTY CXX_STANDARD 20)

target_link_directories(opengl_engine PRIVATE "3rdparty/lib")
target_include_directories(opengl_engine PRIVATE "3rdparty/include" ".")
target_link_libraries(opengl_engine PRIVATE "glfw3dll" "assimp-vc143-mtd")

target_compile_definitions(opengl_engine PRIVATE GL_VER_MAJ=4 GL_VER_MIN=6 GL_FORWARD_COMPAT=GLFW_OPENGL_FORWARD_COMPAT GL_PROFILE=GLFW_OPENGL_CORE_PROFILE PUBLIC USE_DEFAULT_GL_INIT_HINTS)
add_subdirectory("3rdparty")
add_subdirectory("assets")