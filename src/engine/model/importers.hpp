#pragma once

#include <string>
#include <array>
#include <unordered_map>
#include <string_view>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>

#include "../renderer/typedefs.hpp"

struct Model {
    using texture_path_t = std::string;

    struct Mesh {
        enum TEXTURE_IDX { DIFFUSE, NORMAL, METALNESS, ROUGHNESS, EMISSIVE };
        static inline constexpr uint32_t MAX_TEXTURES = 5u;

        size_t vertex_offset, index_offset;
        size_t vertex_count, index_count;
        size_t stride;
        uint32_t present_textures = 0;
        std::array<size_t, MAX_TEXTURES> textures;
    };
    uint32_t id{gid++};
    std::vector<Mesh> meshes;
    std::vector<float> vertices;
    std::vector<unsigned> indices;
    std::vector<texture_path_t> textures;

  private:
    static inline uint32_t gid{0u};
};

class ModelImporter {
  public:
    Model import_model(std::string_view path, uint32_t ai_flags);
};
