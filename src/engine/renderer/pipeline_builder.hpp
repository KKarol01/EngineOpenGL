#pragma once

#include <cassert>

#include "typedefs.hpp"
#include "../engine.hpp"
#include "./renderer.hpp"
#include "pipeline.hpp"

class PipelineBuilder {
  public:
    explicit PipelineBuilder(PipelineID pid) : pid{pid} {
        pipeline = &eng::Engine::instance().renderer_->get_pipeline(pid);
    }

    void begin_new_phase() {
        if (schedule_phase) { assert(("Previous phase is empty!" && schedule_phase->size() != 0)); }

        schedule_phase     = &pipeline_schedule.phases.emplace_back();
        at_least_one_phase = true;
    }
    void add_stage(Shader* phandle, VaoID vid, DRAW_CMD cmd, bool notify) {
        assert(("Did you forget to call \"BeginNewPhase()\"?" && schedule_phase));

        auto &stage = schedule_phase->emplace_back();
        stage.pid   = phandle;
        stage.vid   = vid;

        switch (cmd) {
        case DRAW_CMD::MULTI_DRAW_ELEMENTS_INDIRECT:
            stage.cmd = std::make_shared<MultiDrawElementsIndirectCommand>();
            break;
        case DRAW_CMD::TEST:
            stage.cmd = std::make_shared<TEST_DRAW>();
        default: assert(("Incorrect command type", false));
        }

        if (notify) {
            pipeline->on_geometry_add.connect(
                [&stage](uint32_t mid, const ModelBufferRecord &mbr) { stage.cmd->add_geometry(mid, mbr); });
        }

        /*stage.draw_command->binders.push_back(
            new TargetBaseBufferBinder{GL_SHADER_STORAGE_BUFFER, pipeline->textures_ssbo, 0});*/

        at_least_one_stage = true;
    }
    void configure_vao(const std::function<void(GLVao &)> &config_callback) {
        config_callback(eng::Engine::instance().renderer_->get_vao(pipeline->vao));
        configured_vao = true;
    }

    void build() {
        assert(("pipeline built incorrectly", at_least_one_phase && at_least_one_stage && configured_vao));
        pipeline->set_schedule(pipeline_schedule);
    }

  private:
    Schedule pipeline_schedule;
    ScheduleStages *schedule_phase{nullptr};

    PipelineID pid;
    RenderingPipeline *pipeline{nullptr};

    bool at_least_one_phase{false}, at_least_one_stage{false}, configured_vao{false};
};