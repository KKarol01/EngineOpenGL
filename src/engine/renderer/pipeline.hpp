#pragma once

#include "typedefs.hpp"
#include "schedule.hpp"
#include "../signal/signal.hpp"

#include <memory>
#include <optional>
#include <functional>
#include <set>

class RenderingPipeline {
  public:
    RenderingPipeline();

    void set_schedule(const Schedule &schedule) { this->schedule = schedule; }

    void allocate_model(const Model &m);
    void create_instance(uint32_t mid);
    void render();

    VaoID vao;
    BufferID geometry_vertices, geometry_indices, textures_ssbo;
    Signal<uint32_t, const ModelBufferRecord &> on_geometry_add;
    inline static uint32_t vvv;

  private:
    std::vector<ModelBufferRecord>::iterator get_record(uint32_t mid) {
        return std::find_if(records.begin(), records.end(), [mid](auto &&e) { return e.id == mid; });
    }
    std::vector<ModelBufferRecord> records;

    Schedule schedule;
    std::set<std::string> imported_models;
};