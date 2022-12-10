#pragma once

#include <map>
#include <vector>
#include <string>
#include <string_view>
#include <cstdint>
#include <memory>

#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include "../wrappers/shader/shader.hpp"
#include "../wrappers/buffer/buffer.hpp"
#include "../renderer/renderer.hpp"
//
//struct Vertex {
//    glm::vec3 pos{0.f};
//    glm::vec3 normal{0.f};
//    glm::vec2 tc{0.f};
//    glm::uvec4 bone_ids{0u};
//    glm::vec4 bone_weights{0.f};
//
//    Vertex(const glm::vec3 &pos, const glm::vec3 &normal, const glm::vec2 &tc) noexcept
//        : pos{pos}, normal{normal}, tc{tc} {}
//};
//struct Node {
//    std::string name;
//    std::vector<Node> children;
//    Node *parent{nullptr};
//    glm::mat4 transform_mat;
//};
//struct Bone {
//    uint32_t id;
//    std::string name;
//    glm::mat4 bone_mat{1.f};
//    std::vector<std::pair<uint32_t, float>> weights;
//};
//struct Mesh {
//    VAO vao;
//    Material material;
//    std::vector<Vertex> vertices;
//    std::vector<uint32_t> indices;
//    std::vector<const Bone *> bones;
//};
//struct Skeleton {
//    std::unique_ptr<Node> root_node;
//    std::map<std::string, Bone> bones;
//    std::vector<glm::mat4> final_mats;
//
//    bool contains_aibone(const aiString &name) const { return bones.contains(name.C_Str()); }
//    bool contains_aibone(const aiBone *bone) const { return bones.contains(bone->mName.C_Str()); }
//
//    Bone *get_bone(const std::string &name) {
//        if (bones.contains(name)) return &bones.at(name);
//        return nullptr;
//    }
//    Bone *get_bone(const aiBone *bone) {
//        if (contains_aibone(bone)) return &bones.at(bone->mName.C_Str());
//        return nullptr;
//    }
//    Bone* get_bone(const Node* n) {
//        if(bones.contains(n->name)) return &bones.at(n->name);
//        return nullptr;
//    }
//    Node *find_node(const std::string &name ) {
//        return _find_node_impl(name, root_node.get());
//    }
//
//  private:
//    Node *_find_node_impl(const std::string &name, Node *n) {
//        if (n->name == name) return n;
//        if (n->children.size() == 0) return nullptr;
//
//        for (auto &c : n->children) {
//            auto res = _find_node_impl(name, &c);
//            if (res) return res;
//        }
//        return nullptr;
//    }
//};
//
//template <typename T> struct AnimKey {
//    double time;
//    T value;
//};
//using AnimRotKey = AnimKey<glm::quat>;
//using AnimPosKey = AnimKey<glm::vec3>;
//using AnimSclKey = AnimKey<glm::vec3>;
//struct Channel {
//    std::string node_name;
//    std::vector<AnimRotKey> rotation_keys;
//    std::vector<AnimSclKey> scaling_keys;
//    std::vector<AnimPosKey> position_keys;
//};
//struct Animation {
//    std::string name;
//    double duration_ticks;
//    double ticks_per_sec;
//    std::vector<Channel> channels;
//};
//
//struct Model {
//    std::string name;
//    Skeleton skeleton;
//    std::vector<Animation> animations;
//    std::vector<Mesh> meshes;
//    std::map<std::string, uint32_t> textures;
//
//    void draw(Shader &) const;
//    static Model load_model(std::string_view path);
//};