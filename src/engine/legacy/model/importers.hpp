#pragma once

#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <string_view>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>

#include <glm/glm.hpp>

#define USE_BINDLESS_TEXTURES

struct ModelTexture {
    ModelTexture(std::string_view path, uint32_t filter_minmag, uint32_t wrap_str);

    void build2d();

    std::string path;
    uint32_t handle{0};
    int width{0}, height{0}, depth{0}, channels{0};
    uint32_t wrap_s{0}, wrap_t{0}, wrap_r{0};
    uint32_t filter_min{0}, filter_mag{0};
#ifdef USE_BINDLESS_TEXTURES
    size_t bindless_handle{0};
#endif // USE_BINDLESS_TEXTURES
};

struct Material {};

struct Vertex {
    glm::vec3 position;
};

struct AABB {
    glm::vec3 min{FLT_MAX}, max{-FLT_MAX};
};

struct Mesh {
    std::string name{"undefined"};
    size_t vertex_offset{0u}, normal_offset{0u}, index_offset{0u};
    std::vector<size_t> vertices, indices, normals;
    size_t stride;
};

struct Model {
    using texture_path_t = std::string;

    uint32_t id{gid++};
    std::vector<Mesh> meshes;
    std::vector<glm::vec3> vertices, normals;
    std::vector<glm::vec2> texture_coordinates;
    std::vector<unsigned> indices;
    std::vector<ModelTexture> textures;
    AABB aabb;

  private:
    static inline uint32_t gid{0u};
};

class ModelImporter {
  public:
    Model import_model(std::string_view path, uint32_t ai_flags);
};
