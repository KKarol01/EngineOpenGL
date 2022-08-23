#pragma once

#include <memory>

class Window;
class Controller;
class ShaderManager;
class ECS;
class Renderer;
class GUI;

class Engine {
  public:
    Engine() = default;
    Engine(Window &&) noexcept;
    Engine &operator=(Window &&) noexcept;
    ~Engine();

    void update();

    inline Window *window() { return window_.get(); }
    inline Controller *controller() { return controller_.get(); }
    inline ShaderManager *shader_manager() { return shader_manager_.get(); }
    inline ECS *ecs() { return ecs_.get(); }
    inline double deltatime() { return dt; }

    static void initialise(Window &&w);
    static Engine &instance() { return _instance; }

    double time{0.f}, dt{0.f};
    std::unique_ptr<Window> window_;
    std::unique_ptr<Controller> controller_;
    std::unique_ptr<ShaderManager> shader_manager_;
    std::unique_ptr<ECS> ecs_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<GUI> gui_;

  private:
    static Engine _instance;
};