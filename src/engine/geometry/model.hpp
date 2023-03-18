#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

typedef std::uint32_t TextureID;
typedef std::uint32_t ModelID;
typedef std::uint32_t MeshID;
typedef std::uint32_t MaterialID;

struct Texture;
struct Material;
struct Mesh;
struct Model;
struct Vertex;

enum class TextureType {
    Diffuse,
    Normal,
    Metallic,
    Roughness,
    Emission,
    DiffuseMetallic,
    DiffuseRoughness,
    NormalMetallic,
    NormalRoughness,
    MetallicRoughness
};

struct Vertex {
    glm::vec3 position{0.f}, normal{0.f};
    glm::uvec2 uv{0.f};
};

struct Transform {
    auto position() const { return t; }
    auto rotation() const { return r; }
    auto scale() const { return s; }
    auto &position() { return t; }
    auto &rotation() { return r; }
    auto &scale() { return s; }

    auto to_mat4() const {
        constexpr glm::mat4 I{1.f};

        const auto T = glm::translate(I, t);
        const auto R = glm::eulerAngleXYZ(r.x, r.y, r.z);
        const auto S = glm::scale(I, s);

        return T * R * S;
    }

  private:
    glm::vec3 t{0.f}, r{0.f}, s{1.f};
};

struct Mesh {
    MeshID id;
    Material material;
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
};

struct Model {
    Transform transform;
    std::vector<Mesh> meshes;
};