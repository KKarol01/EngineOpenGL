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

namespace eng {

    struct DrawCMD {
        virtual void draw() = 0;
        virtual ~DrawCMD()  = default;
    };
    struct DrawElementsCMD final : public DrawCMD {
        DrawElementsCMD(BufferID buffer);

        void draw() final;

        size_t count;
    };
    struct BufferBinder {
        virtual void bind() = 0;
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
        VaoID vao;
        ProgramID program;
        std::shared_ptr<DrawCMD> draw_cmd;
        std::vector<std::shared_ptr<BufferBinder>> bufferbinders;
    };

    class Pipeline {
      public:
        void render();

        std::vector<PipelineStage> stages;
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