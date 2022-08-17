#include "renderer.hpp"
#include "../engine.hpp"

#include <glad/glad.h>

void Renderer::render() {
    for (auto &m : materials) {
        m.shader->use();
        for (auto &d : m.object_datas) {
            d->vao.bind();
            m.shader->feed_uniforms(d->uniforms);
            for (auto &[id, t] : d->textures) glBindTextureUnit(id, t.handle);
            d->vao.draw();
        }
    }
}

void Renderer::register_material(std::string_view name, std::string_view sh_name) {
    materials.emplace_back(name.data(), sh_name.data());
}

void Renderer::add_to_draw_list(std::string_view mat_name, RenderData *data) {

    auto mat = std::find_if(
        materials.begin(), materials.end(), [&mat_name](const auto &mat) { return mat.name == mat_name; });
    if (mat == materials.end()) throw std::runtime_error{"Material of that name does not exists."};

    mat->object_datas.push_back(data);
}

Material::Material(const std::string &name, const std::string &sh_name)
    : name{name}, shader{Engine::shader_manager()->get_shader(sh_name)} {}
