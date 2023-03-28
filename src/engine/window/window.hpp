#pragma once

#include <string>
#include <string_view>

struct GLFWwindow;

namespace eng {
    class Window {
      public:
        struct WINDOW_HINTS {
            unsigned MAJOR_VERSION;
            unsigned MINOR_VERSION;
            unsigned PROFILE;
            unsigned FORWARD_COMPAT;
        };

      public:
        Window() noexcept = default;
        Window(std::string_view window_name, unsigned window_width = 640, unsigned window_height = 480);
        Window(Window &&w) noexcept;
        Window &operator=(Window &&w) noexcept;
        ~Window();

        void make_current() const;
        void toggle_vsync(int val = 1);
        void adjust_glviewport();

        void swap_buffers() const;
        void clear_framebuffer();
        inline void set_clear_flags(unsigned flags) { clear_buffer_flags = flags; }
        void close();

        bool should_close() const;

        inline auto glfwptr() const { return glfw_window; }
        inline auto title() const { return window_name; }
        inline void resize(int w, int h) {
            window_width  = w;
            window_height = h;
            adjust_glviewport();
        }
        inline auto width() const { return window_width; }
        inline auto height() const { return window_height; }
        inline auto aspect() const { return (float)(window_width) / (float)(window_height); }

      private:
        static void configure_glfw_and_hints(WINDOW_HINTS hints);

      private:
        static inline bool glfw_initialized{false};
        GLFWwindow *glfw_window{nullptr};
        std::string window_name;
        unsigned window_width{640}, window_height{480};
        unsigned clear_buffer_flags;
    };
} // namespace eng
