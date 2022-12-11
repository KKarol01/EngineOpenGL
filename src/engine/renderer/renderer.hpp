#pragma once

/*
*
* dla ognia:
* vao, shader
* narusyj ogien do 3 tekstur
*
*
        pp {
        buffer, drawing_cmd

                s{
                        fboid, bindings, vao, program
                }
        }
*/

#include "../wrappers/shader/shader.hpp"
#include "../wrappers/buffer/buffer.hpp"
#include "../allocators/idallocator.hpp"
#include "../types/types.hpp"
#include "../model/importers.hpp"

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

    struct ModelPipelineAdapter {
        enum ATTRIBUTES { ATTR_POSITION, ATTR_NORMAL, ATTR_TEXCOORDS };

        ModelPipelineAdapter() = default;
        ModelPipelineAdapter(std::initializer_list<ATTRIBUTES> vbo_layout);

        bool uses_attribute(ATTRIBUTES attr) const {
            return std::find(layout.begin(), layout.end(), attr) != layout.end();
        }
        std::vector<float> convert(const Model &model);

        size_t stride{0};
        std::vector<ATTRIBUTES> layout;
    };

    struct PipelineStage {
        VaoID vao;
        ProgramID program;
        std::shared_ptr<DrawCMD> draw_cmd;
        std::vector<std::shared_ptr<BufferBinder>> bufferbinders;
    };

    class Pipeline {
      public:
        Pipeline();
        explicit Pipeline(ModelPipelineAdapter adapter);
        void render();
        void add_model(const Model &m);

        ModelPipelineAdapter adapter;
        Signal<const Model &> on_model_add;
        std::vector<PipelineStage> stages;
        BufferID vbo{0}, ebo{0};
    };

    class Renderer {

      public:
        void render();

        IDAllocator<Pipeline> pipelines;
        IDAllocator<GLVao> vaos;
        IDAllocator<GLBuffer> buffers;
        IDAllocator<ShaderProgram> programs;
    };

} // namespace eng