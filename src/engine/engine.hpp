#pragma once

#include <memory>
#include <string_view>

#include <engine/window/window.hpp>
#include <engine/controller/controller.hpp>
#include <engine/gpu/shaderprogram/shader.hpp>
#include <engine/gpu/buffers/buffer.hpp>
#include <engine/gpu/buffers/ubo.hpp>
#include <engine/gpu/resource_manager/gpu_res_mgr.hpp>
#include <engine/renderer/renderer.hpp>
#include <engine/gui/gui.hpp>
#include <engine/camera/camera.hpp>

#ifndef _DEBUG
#define ENG_DEBUG(fmt, ...)
#else
#define ENG_DEBUG(fmt, ...) printf("[%s %s:%i] " fmt, __FILE__, __func__, __LINE__, __VA_ARGS__)
#endif

namespace eng {
    class Engine {
      public:
        Engine() = default;
        ~Engine();

        void start();

        Window *get_window() { return _window.get(); }
        Camera *get_camera() { return _camera.get(); }
        Controller *get_controller() { return _controller.get(); }
        GpuResMgr *get_gpu_res_mgr() { return _gpu_res_mgr.get(); }
        Renderer *get_renderer() { return _renderer.get(); }
        GUI *get_gui() { return _gui.get(); }

        static void initialise(std::string_view window_name, uint32_t size_x, uint32_t size_y);
        static Engine &instance() { return *_instance; }

        std::unique_ptr<Window> _window;
        std::unique_ptr<Camera> _camera;
        std::unique_ptr<Controller> _controller;
        std::unique_ptr<GpuResMgr> _gpu_res_mgr;
        std::unique_ptr<Renderer> _renderer;
        std::unique_ptr<GUI> _gui;

      private:
        void _update();

        inline static std::unique_ptr<Engine> _instance;
    };
} // namespace eng