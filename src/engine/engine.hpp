#pragma once

#include <memory>

#include "camera/camera.hpp"
#include "controller/keyboard/keyboard.hpp"
#include "gui/gui.hpp"
#include "renderer/renderer.hpp"
#include "scene/scene.hpp"
#include "signal/signal.hpp"
#include "ecs/ecs.hpp"
#include "wrappers/include_all.hpp"

class Engine {
    inline static std::unique_ptr<Window> window_;
    inline static std::unique_ptr<Controller> controller_;
    inline static std::unique_ptr<ShaderManager> shader_manager_;
    inline static std::unique_ptr<GUI> gui_;
    inline static std::unique_ptr<ECS> ecs_;

    inline static double time{0.f}, dt{0.f};
    inline static Scene *scene_;

  public:
    inline static std::unique_ptr<Renderer> renderer_;
    static void initialize(Window &&window);
    static void terminate();
    static void update();
    static void render_frame();

    static void set_scene(Scene *);

    inline static Window *window() { return window_.get(); }
    inline static Controller *controller() { return controller_.get(); }
    inline static ShaderManager *shader_manager() { return shader_manager_.get(); }
    inline static GUI *gui() { return gui_.get(); }
    inline static Scene *scene() { return scene_; }
    inline static ECS *ecs() { return ecs_.get(); }

    inline static double deltatime() { return dt; }
};