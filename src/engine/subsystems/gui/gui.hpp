#pragma once

#include <vector>
#include <map>
#include <functional>
#include <stdexcept>
#include <cstring>
#include <memory>

#include "ecs_comps.hpp"

struct ImGuiIO;
struct RenderGraphGUI;


class GUI {
  public:
    GUI();
    ~GUI();

    uint32_t add_draw(std::function<void()> &&f) {
        ui_draws.emplace(ui_draws_count, std::move(f));
        return ui_draws_count++;
    }

    void remove_draw(uint32_t id) {
        if (!ui_draws.contains(id)) {
            char err[128];
            sprintf_s(err, "Could not found ui draw function of specified id: %u.", id);
            throw std::runtime_error{err};
        }

        ui_draws.erase(id);
    }

    void draw();

  private:
    std::map<uint32_t, std::function<void()>> ui_draws;
    uint32_t ui_draws_count{0u};
    ComponentGUI comp_gui;
    ImGuiIO *io{nullptr};

    std::unique_ptr<RenderGraphGUI> render_graph;
};