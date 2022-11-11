#include "draw_commands.hpp"

#include "../renderer/pipeline.hpp"

#include <glad/glad.h>
#include <memory>
#include <cassert>

void MultiDrawElementsIndirectCommand::draw() {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, RenderingPipeline::vvv);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commands_buffer.descriptor.handle);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, size, 0);
}

void MultiDrawElementsIndirectCommand::add_geometry(uint32_t model_id, const ModelBufferRecord &mbr) {

    if (auto it = std::find(model_idx_in_buffer.begin(), model_idx_in_buffer.end(), model_id);
        it != model_idx_in_buffer.end()) {

        auto idx      = std::distance(model_idx_in_buffer.begin(), it);
        auto n        = model_idx_in_buffer.size() - idx;
        auto buff_ptr = (DrawElementsIndirectCommand *)glMapNamedBufferRange(commands_buffer.descriptor.handle,
                                                                             idx * sizeof(DrawElementsIndirectCommand),
                                                                             n * sizeof(DrawElementsIndirectCommand),
                                                                             MAP_FLAGS);
        buff_ptr->instanceCount++;
        glUnmapNamedBuffer(commands_buffer.descriptor.handle);

        return;
    }

    size_t temp_offset = 0;
    size_t temp_idx    = 0;
    std::vector<DrawElementsIndirectCommand> commands;
    for (auto &m : mbr.meshes) {
        DrawElementsIndirectCommand cmd;
        cmd.instanceCount = 1;
        cmd.count         = m.ind_count;
        cmd.baseInstance  = 0;
        cmd.baseVertex    = mbr.offset + temp_offset;
        cmd.firstIndex    = temp_idx;
        commands.push_back(cmd);

        temp_offset += m.vert_count;
        temp_idx += m.ind_count;
    }

    model_idx_in_buffer.emplace_back(model_id);
    commands_buffer.push_data(commands.data(), commands.size() * sizeof(DrawElementsIndirectCommand));
    size += commands.size();
}

const std::uint32_t MultiDrawElementsIndirectCommand::BUFF_FLAGS
    = GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

const std::uint32_t MultiDrawElementsIndirectCommand::MAP_FLAGS
    = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

void TEST_DRAW::draw() { glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0); }

void TEST_DRAW::add_geometry(uint32_t model_id, const ModelBufferRecord &mbr) {}
