#pragma once

#include <map>
#include <set>
#include <vector>
#include <functional>
#include <string_view>
#include <tuple>

#include "../wrappers/include_all.hpp"
#include "../scene/scene.hpp"
#include "../wrappers/shader/shader.hpp"

struct RenderData {
    VAO vao;
    std::vector<SHADERDATA> uniforms;
    std::vector<std::pair<uint32_t, Texture>> textures;
};

struct Material {
    std::string name;
    Shader *shader{nullptr};
    std::vector<RenderData *> object_datas;
    // additional setting like face culling or depth masks

    Material(const std::string &name, const std::string &sh_name);
};

class Renderer {
    std::vector<Material> materials;

    void render();

  public:
    void register_material(std::string_view name, std::string_view sh_name);
    void add_to_draw_list(std::string_view mat_name, RenderData *data);

    friend class Engine;
};