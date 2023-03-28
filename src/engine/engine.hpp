#pragma once

#include <memory>
#include <string_view>

#include "window/window.hpp"
#include "controller/controller.hpp"
#include "gpu/shaderprogram/shader_manager.hpp"
#include "renderer/renderer.hpp"
#include "gui/gui.hpp"
#include "camera/camera.hpp"

namespace eng {
    class Engine {
      public:
        Engine() = default;
        ~Engine();

        void update();

        inline Window *window() { return window_.get(); }
        inline Controller *controller() { return controller_.get(); }
        inline ShaderManager *shader_manager() { return shader_manager_.get(); }
        // inline ECS *ecs() { return ecs_.get(); }
        inline double deltatime() { return dt; }

        static void initialise(std::string_view window_name, uint32_t size_x, uint32_t size_y);
        static Engine &instance() { return *_instance; }

        double time{0.f}, dt{0.f};
        std::unique_ptr<Window> window_;
        std::unique_ptr<Controller> controller_;
        std::unique_ptr<ShaderManager> shader_manager_;
        //        std::unique_ptr<eng::ECS> ecs_;
        std::unique_ptr<eng::Renderer> renderer_;
        std::unique_ptr<GUI> gui_;
        Camera *cam;

      private:
        inline static std::unique_ptr<Engine> _instance;
    };
} // namespace eng