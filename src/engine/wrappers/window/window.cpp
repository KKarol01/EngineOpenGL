#include "window.hpp"

#include "../../engine/engine.hpp"

#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void Window::configure_glfw_and_hints(WINDOW_HINTS hints) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, hints.MAJOR_VERSION);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, hints.MINOR_VERSION);
    glfwWindowHint(GLFW_OPENGL_PROFILE, hints.PROFILE);
    glfwWindowHint(GLFW_OPENGL_COMPAT_PROFILE, hints.FORWARD_COMPAT);
    glfwWindowHint(GLFW_SAMPLES, 4);

    glfw_initialized = true;
};

Window::Window(std::string_view window_name, unsigned window_width, unsigned window_height)
    : window_name{window_name}, window_width{window_width}, window_height{window_height}, clear_buffer_flags{
                                                                                              GL_COLOR_BUFFER_BIT
                                                                                              | GL_DEPTH_BUFFER_BIT} {
    if (!glfw_initialized) {
#ifdef USE_DEFAULT_GL_INIT_HINTS
        configure_glfw_and_hints({.MAJOR_VERSION  = GL_VER_MAJ,
                                  .MINOR_VERSION  = GL_VER_MIN,
                                  .PROFILE        = GL_PROFILE,
                                  .FORWARD_COMPAT = GL_FORWARD_COMPAT});

#elif
        throw std::runtime_error{"Default GLFW hints disabled, please, call configure_glfw_and_hints method yourself."};
#endif
    }

    glfw_window = glfwCreateWindow(window_width, window_height, window_name.data(), 0, 0);
    make_current();
    toggle_vsync();

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { throw std::runtime_error{"Could not initialize glad."}; }

    glfwSetFramebufferSizeCallback(
        glfw_window,
        (GLFWframebuffersizefun)[](auto a, auto b, auto c) { eng::Engine::instance().window()->resize(b, c); });

    glViewport(0, 0, window_width, window_height);
}

Window::Window(Window &&w) noexcept { *this = std::move(w); }

Window &Window::operator=(Window &&w) noexcept {
    glfw_window        = w.glfw_window;
    window_width       = w.window_width;
    window_height      = w.window_height;
    window_name        = w.window_name;
    clear_buffer_flags = w.clear_buffer_flags;
    glfw_initialized   = w.glfw_initialized;

    w.glfw_window      = nullptr;
    w.glfw_initialized = false;

    return *this;
}

Window::~Window() {
    glfwDestroyWindow(glfw_window);
    glfw_window = nullptr;
}

void Window::make_current() const { glfwMakeContextCurrent(glfw_window); }

void Window::toggle_vsync(int val) { glfwSwapInterval(val); }

void Window::adjust_glviewport() { glViewport(0, 0, window_width, window_height); }

void Window::swap_buffers() const { glfwSwapBuffers(glfw_window); }

void Window::clear_framebuffer() { glClear(clear_buffer_flags); }

void Window::close() { glfwSetWindowShouldClose(glfw_window, GLFW_TRUE); }

bool Window::should_close() const { return !!glfwWindowShouldClose(glfw_window); }
