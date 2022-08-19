add_executable(opengl_engine "main.cpp" "3rdparty/glad.c"  "engine/wrappers/buffer/buffer.cpp" "engine/wrappers/shader/shader.cpp" "engine/wrappers/shader/shader_template.cpp" "engine/wrappers/shader/shader_manager.cpp" "engine/wrappers/texture/texture.cpp" "engine/wrappers/window/window.cpp" "engine/engine.cpp" "engine/camera/camera.cpp" "engine/gui/gui.cpp" "engine/controller/controller.cpp" "engine/controller/keyboard/keyboard.cpp"  "engine/ecs/ecs.cpp" "engine/renderer/renderer.cpp" "engine/renderer/systems.cpp" )
set_property(TARGET opengl_engine PROPERTY CXX_STANDARD 20)

target_link_directories(opengl_engine PRIVATE "3rdparty/lib")
target_include_directories(opengl_engine PRIVATE "3rdparty/include")
target_link_libraries(opengl_engine PRIVATE "glfw3dll" "assimp-vc143-mtd")

target_compile_definitions(opengl_engine PRIVATE GL_VER_MAJ=4 GL_VER_MIN=6 GL_FORWARD_COMPAT=GLFW_OPENGL_FORWARD_COMPAT GL_PROFILE=GLFW_OPENGL_CORE_PROFILE PUBLIC USE_DEFAULT_GL_INIT_HINTS)

add_subdirectory("3rdparty")
add_subdirectory("assets")