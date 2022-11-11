#pragma once

#include <vector>
#include "typedefs.hpp"
#include "buffer_binder.hpp"
#include "../wrappers/buffer/buffer.hpp"

struct DrawElementsIndirectCommand {
    uint32_t count;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int baseVertex;
    uint32_t baseInstance;
};

class DrawCommand {
  public:
    virtual void draw()                                                        = 0;
    virtual void add_geometry(uint32_t model_id, const ModelBufferRecord &mbr) = 0;
    std::vector<BufferBinder *> binders;

    // TODO: virtual void remove_geometry() = 0;
};

class TEST_DRAW : public DrawCommand {
  public:
    void draw() override;
    void add_geometry(uint32_t model_id, const ModelBufferRecord &mbr) override;
};
class MultiDrawElementsIndirectCommand : public DrawCommand {
  public:
    void draw() override;
    void add_geometry(uint32_t model_id, const ModelBufferRecord &mbr) override;

  private:
    std::vector<std::uint32_t> model_idx_in_buffer;
    GLBuffer commands_buffer{BUFF_FLAGS};
    size_t size{0u};

    static const std::uint32_t BUFF_FLAGS;
    static const std::uint32_t MAP_FLAGS;
};
