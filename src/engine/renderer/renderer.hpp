#pragma once

#include <cstdint>
#include <vector>
#include <concepts>

#include "../types/idallocator.hpp"

#include "typedefs.hpp"
#include "../gpu/buffers/buffer.hpp"
#include "../gpu/shaderprogram/shader.hpp"

namespace eng {
    class ShaderProgram;
    class GLBuffer;
    class GLVao;
} // namespace eng

namespace eng {

    struct DrawCMD {
        DrawCMD();
        DrawCMD(uint32_t gl_mode);

        virtual void draw() = 0;
        virtual ~DrawCMD()  = default;

        uint32_t gl_mode;
    };
    struct DrawElementsCMD final : public DrawCMD {
        DrawElementsCMD(BufferID buffer);

        void draw() final;

        size_t count;
    };

    struct DrawElementsInstancedCMD final : public DrawCMD {
        DrawElementsInstancedCMD(BufferID buffer, uint32_t instances);

        void draw() final;

        size_t count{0};
        uint32_t instances{0};
    };

    struct DrawArraysInstancedCMD final : public DrawCMD {
        DrawArraysInstancedCMD(size_t vertex_count, size_t instance_count) : DrawCMD() {
            this->first          = 0;
            this->vertex_count   = vertex_count;
            this->instance_count = instance_count;
        }
        DrawArraysInstancedCMD(size_t vertex_count, size_t instance_count, int first, uint32_t gl_mode)
            : DrawCMD(gl_mode) {
            this->first          = first;
            this->vertex_count   = vertex_count;
            this->instance_count = instance_count;
        }

        void draw() final;

        int first;
        size_t vertex_count, instance_count;
    };
    struct DrawArraysInstancedBaseInstanceCMD final : public DrawCMD {
        DrawArraysInstancedBaseInstanceCMD(size_t vertex_count, size_t instance_count, uint32_t base_instance)
            : DrawCMD() {
            this->first          = 0;
            this->vertex_count   = vertex_count;
            this->instance_count = instance_count;
            this->base_instance  = base_instance;
        }
        DrawArraysInstancedBaseInstanceCMD(
            size_t vertex_count, size_t instance_count, int first, uint32_t base_instance, uint32_t gl_mode)
            : DrawCMD(gl_mode) {
            this->first          = first;
            this->vertex_count   = vertex_count;
            this->instance_count = instance_count;
            this->base_instance  = base_instance;
        }

        void draw() final;

        int first;
        size_t vertex_count, instance_count;
        uint32_t base_instance;
    };

    struct BufferBinder {
        virtual void bind()     = 0;
        virtual ~BufferBinder() = default;

        uint32_t gl_target;
        BufferID buffer;
    };
    struct BufferBasedBinder final : public BufferBinder {
        BufferBasedBinder(uint32_t gl_target, BufferID buffer, uint32_t base) {
            this->gl_target = gl_target;
            this->buffer    = buffer;
            this->base      = base;
        }

        void bind() final;

        uint32_t base{0u};
    };

    struct PipelineStage {
        SharedResource<ShaderProgram> program;
        SharedResource<eng::GLVao> vao;
        std::shared_ptr<DrawCMD> draw_cmd;
        std::vector<std::shared_ptr<BufferBinder>> bufferbinders;
        std::function<void()> on_stage_start, on_stage_end;
    };

    class Pipeline {
      public:
        void render();

        PipelineStage &create_stage() { return stages_.emplace_back(); }

      private:
        std::vector<PipelineStage> stages_;
    };

    class Renderer {

      public:
        // Renderer();
        //~Renderer();

        void render_frame();

      public:
        template <typename... ARGS>
        decltype(auto) create_pipeline(ARGS &&...args) requires std::constructible_from<Pipeline, ARGS...> {
            return pipelines_.emplace(std::forward<ARGS>(args)...);
        }
        template <typename... ARGS>
        decltype(auto) create_program(ARGS &&...args) requires std::constructible_from<ShaderProgram, ARGS...> {
            return programs_.emplace(std::forward<ARGS>(args)...);
        }
        template <typename... ARGS>
        decltype(auto) create_vao(ARGS &&...args) requires std::constructible_from<eng::GLVao, ARGS...> {
            return vaos_.emplace(std::forward<ARGS>(args)...);
        }
        template <typename... ARGS>
        decltype(auto) create_buffer(ARGS &&...args) requires std::constructible_from<eng::GLBuffer, ARGS...> {
            return buffers_.emplace(std::forward<ARGS>(args)...);
        }

        auto &get_program(ProgramID id) { return programs_.get(id); }
        auto &get_vao(VaoID id) { return vaos_.get(id); }
        auto &get_buffer(BufferID id) { return buffers_.get(id); }

      private:
        IDAllocator<Pipeline> pipelines_;
        IDAllocator<ShaderProgram> programs_;
        IDAllocator<GLVao> vaos_;
        IDAllocator<GLBuffer> buffers_;
    };

} // namespace eng