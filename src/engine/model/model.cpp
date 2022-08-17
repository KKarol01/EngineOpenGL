#include "model.hpp"

#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <stb_image.h>

#include <glad/glad.h>

extern glm::mat4 glm_cast(const aiMatrix4x4 &mat) { return glm::transpose(glm::make_mat4(&mat.a1)); }
extern glm::vec3 glm_cast(const aiVector3D &vec) { return glm::vec3{vec.x, vec.y, vec.z}; }
extern glm::quat glm_cast(const aiQuaternion &q) { return glm::quat{q.w, q.x, q.y, q.z}; }

void Model::draw(Shader &program) const {
    glUniformMatrix4fv(glGetUniformLocation(program.get_program(), "bone_mats"), skeleton.final_mats.size(), GL_FALSE, (float *)skeleton.final_mats.data());
    for (const auto &m : meshes) {
        glBindTextureUnit(0, m.material.texture_handle);
        glBindVertexArray(m.vao);
        glDrawElements(GL_TRIANGLES, m.indices.size(), GL_UNSIGNED_INT, 0);
    }
}

static void process_mesh_indices(const aiMesh *mesh, Mesh *user_mesh) {
    const auto nfaces = mesh->mNumFaces;
    user_mesh->indices.reserve(nfaces * 3);

    for (auto i = 0u; i < nfaces; ++i) {
        user_mesh->indices.push_back(mesh->mFaces[i].mIndices[0]);
        user_mesh->indices.push_back(mesh->mFaces[i].mIndices[1]);
        user_mesh->indices.push_back(mesh->mFaces[i].mIndices[2]);
    }
}
static void process_mesh_vertices(const aiMesh *mesh, Mesh *user_mesh) {
    const auto nvertices = mesh->mNumVertices;
    user_mesh->vertices.reserve(nvertices);

    for (auto i = 0u; i < nvertices; ++i) {
        const auto &vertex = mesh->mVertices[i];
        glm::vec3 pos      = glm::vec3{vertex.x, vertex.y, vertex.z};
        glm::vec3 normal{0.f};
        glm::vec2 uv{0.f};

        if (mesh->mNormals) { normal = glm::vec3{mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z}; }
        if (mesh->mTextureCoords && mesh->mTextureCoords[0]) { uv = glm::vec2{mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y}; }

        user_mesh->vertices.emplace_back(pos, normal, uv);
    }
}
static void process_mesh_bones(const aiMesh *mesh, Model *model, Mesh *user_mesh) {
    const auto nbones = mesh->mNumBones;

    for (auto i = 0u; i < nbones; ++i) {
        const auto bone = mesh->mBones[i];

        auto skbone = model->skeleton.get_bone(bone);
        if (!skbone) {
            Bone b;
            b.id       = model->skeleton.bones.size();
            b.name     = bone->mName.C_Str();
            b.bone_mat = glm_cast(bone->mOffsetMatrix);
            for (auto j = 0u; j < bone->mNumWeights; ++j) {
                const auto &weight = bone->mWeights[j];
                b.weights.emplace_back(weight.mVertexId, weight.mWeight);
            }
            model->skeleton.bones.insert({b.name, b});
            skbone = model->skeleton.get_bone(bone);
            assert(skbone);
        }
        user_mesh->bones.push_back(skbone);
        for (const auto &[id, w] : skbone->weights) {
            auto &vx = user_mesh->vertices[id];
            auto idx = 0u;
            for (auto j = 0u; j < 4u; ++j, ++idx) {
                if (vx.bone_weights[j] == 0.f) break;
            }

            if (idx > 3u) continue;

            vx.bone_ids[idx]     = skbone->id;
            vx.bone_weights[idx] = w;
        }
    }
}
static void process_mesh_material(const aiScene *scene_, const aiMesh *mesh, Model *model, Mesh *user_mesh) {
    if (mesh->mMaterialIndex >= scene_->mNumMaterials) return;

    const auto material = scene_->mMaterials[mesh->mMaterialIndex];
    aiString path;
    auto result = material->GetTexture(aiTextureType_DIFFUSE, 0, &path);

    if (result != aiReturn_SUCCESS) return;

    if (model->textures.contains(path.C_Str())) {
        user_mesh->material.texture_handle = model->textures.at(path.C_Str());
        return;
    }

    int x, y, ch;
    auto tex_data = stbi_load("3dmodels/stormtrooper/textures/Stormtrooper_D.png", &x, &y, &ch, 0);

    GLuint texture_handle;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture_handle);
    glTextureStorage2D(texture_handle, 8, GL_RGB8, x, y);
    glTextureSubImage2D(texture_handle, 0, 0, 0, x, y, ch == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
    glTextureParameteri(texture_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(texture_handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture_handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateTextureMipmap(texture_handle);
    stbi_image_free(tex_data);

    model->textures[path.C_Str()]      = texture_handle;
    user_mesh->material.texture_handle = texture_handle;
}
static void process_mesh_buffers(Mesh *user_mesh) {
    user_mesh->vao.insert_vbo(BUFFEROBJECT{0, user_mesh->vertices, 64, 0});
    user_mesh->vao.insert_ebo(user_mesh->indices.size(), BUFFEROBJECT{0, user_mesh->indices});

    user_mesh->vao.configure({
        ATTRCUSTORMFORMAT{0, 0, 3, GL_FLOAT, 4, 0},
        ATTRCUSTORMFORMAT{1, 0, 3, GL_FLOAT, 4, 12},
        ATTRCUSTORMFORMAT{2, 0, 2, GL_FLOAT, 4, 24},
        ATTRCUSTORMFORMAT{3, 0, 4, GL_UNSIGNED_INT, 4, 32, ATTRFORMATCOMMON::GL_FUNC::INT},
        ATTRCUSTORMFORMAT{4, 0, 4, GL_FLOAT, 4, 48},
    });
}
static void build_node_tree(const aiNode *node, Model &model, Node *n, Node *parent_node = nullptr) {
    n->name          = node->mName.C_Str();
    n->transform_mat = glm_cast(node->mTransformation);
    n->parent        = parent_node;
    n->children.resize(node->mNumChildren);
    for (auto i = 0u; i < node->mNumChildren; ++i) { build_node_tree(node->mChildren[i], model, &n->children[i], n); }
}
static void process_model_basic_data(Model &model, const aiScene *scene_) {
    model.skeleton.root_node = std::make_unique<Node>();
    build_node_tree(scene_->mRootNode, model, model.skeleton.root_node.get());
    model.skeleton.final_mats.resize(model.skeleton.bones.size());

    for (auto i = 0u; i < scene_->mNumAnimations; ++i) {
        const auto anim = scene_->mAnimations[i];
        Animation a;
        a.name           = anim->mName.C_Str();
        a.duration_ticks = anim->mDuration;
        a.ticks_per_sec  = anim->mTicksPerSecond ? anim->mTicksPerSecond : 30.f;

        for (auto j = 0u; j < anim->mNumChannels; ++j) {
            const auto channel = anim->mChannels[j];
            Channel ch;
            ch.node_name = channel->mNodeName.C_Str();

            const auto process_keys = [&ch]<typename KeyType>(auto *arr, auto length, std::vector<KeyType> &vec) {
                for (auto k = 0u; k < length; ++k) {
                    const auto &skey = arr[k];
                    KeyType animkey;
                    animkey.time  = skey.mTime;
                    animkey.value = glm_cast(skey.mValue);
                    vec.push_back(animkey);
                }
            };
            process_keys(channel->mScalingKeys, channel->mNumScalingKeys, ch.scaling_keys);
            process_keys(channel->mRotationKeys, channel->mNumRotationKeys, ch.rotation_keys);
            process_keys(channel->mPositionKeys, channel->mNumPositionKeys, ch.position_keys);

            a.channels.push_back(ch);
        }
        model.animations.push_back(a);
    }
}
static void skeleton_bind_pose(Model &m, Node *node, const glm::mat4 &parent_transform = glm::mat4{1.f}) {
    for (auto &[n, b] : m.skeleton.bones) {
        auto mat = b.bone_mat;
        auto n   = m.skeleton.find_node(b.name);
        while (n) {
            mat = n->transform_mat * mat;
            n   = n->parent;
        }
        m.skeleton.final_mats[b.id] = mat;
    }
}
static void process_skeleton_and_meshes(Model *model, const aiScene *scene_, const aiNode *node) {
    const auto nmeshes = node->mNumMeshes;

    for (auto i = 0u; i < nmeshes; ++i) {
        const auto assimpmesh = scene_->mMeshes[node->mMeshes[i]];
        const auto nbones     = assimpmesh->mNumBones;

        Mesh mesh;
        process_mesh_indices(assimpmesh, &mesh);
        process_mesh_vertices(assimpmesh, &mesh);
        process_mesh_bones(assimpmesh, model, &mesh);
        process_mesh_buffers(&mesh);
        process_mesh_material(scene_, assimpmesh, model, &mesh);

        model->meshes.push_back(mesh);
    }

    for (auto i = 0u; i < node->mNumChildren; ++i) { process_skeleton_and_meshes(model, scene_, node->mChildren[i]); }
}

Model Model::load_model(std::string_view path) {
    Model model;
    auto importer = Assimp::Importer{};
    auto scene_    = importer.ReadFile(path.data(), aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_PopulateArmatureData);
    if (!scene_) return {};

    process_skeleton_and_meshes(&model, scene_, scene_->mRootNode);
    process_model_basic_data(model, scene_);
    skeleton_bind_pose(model, model.skeleton.root_node.get());

    return model;
}
