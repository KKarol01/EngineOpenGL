#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

typedef std::uint32_t StageID;
typedef std::uint32_t BufferID;
typedef std::uint32_t VaoID;
typedef std::uint32_t PipelineID;
typedef std::uint32_t ProgramID;

enum class TEX { DIFFUSE, NORMAL, METALLIC, ROUGHNESS, AO };
enum class DRAW_CMD { MULTI_DRAW_ELEMENTS_INDIRECT, TEST };
struct TextureDesc {
    uint32_t width{0u}, height{0u}, num_channels{0u};
    uint32_t texture{0u};
    std::uint64_t bindless_handle{0ull};
};

struct Material {
    std::string name;
    std::array<TextureDesc, 5> arr{};
};

struct Mesh {
    std::string name;
    Material *material{nullptr};
    std::vector<float> vertices;
    std::vector<unsigned> indices;
};

struct Model {

    uint32_t id{gmodel_id++};
    std::vector<Mesh> meshes;

    inline static uint32_t gmodel_id{0u};
};

struct ModelBufferRecord {
    struct MeshRecord {
        size_t ind_count{0u}, vert_count{0u}, vertices_size_bytes{0u};
        std::unordered_map<uint32_t, TextureDesc> textures;
    };

    ModelBufferRecord(const Model &m, uint32_t offset) : offset{offset} {
        id = m.id;

        for (const auto &mesh : m.meshes) {
            vertices_size_bytes += mesh.vertices.size() * sizeof(decltype(mesh.vertices)::value_type);

            MeshRecord record;
            record.vert_count          = mesh.vertices.size() / 14;
            record.ind_count           = mesh.indices.size();
            record.vertices_size_bytes = mesh.vertices.size() * sizeof(decltype(mesh.vertices)::value_type);
            for (auto i = 0u; i < mesh.material->arr.size(); ++i) { record.textures[i] = mesh.material->arr[i]; }

            meshes.emplace_back(std::move(record));
        }
    }

    uint32_t id;
    size_t offset{0}, vertices_size_bytes{0};
    std::vector<MeshRecord> meshes;
};