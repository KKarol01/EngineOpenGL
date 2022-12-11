#include "importers.hpp"

#include <filesystem>
#include <ranges>
#include <iostream>
#include <cassert>
#include <functional>

#include <glad/glad.h>
#include <stb_image.h>

Model ModelImporter::import_model(std::string_view path, uint32_t ai_flags) {
    Assimp::Importer importer;
    const auto scene = importer.ReadFile(path.data(), ai_flags);

    Model model;

    std::function<void(const aiScene *, aiNode *)> parse_node = [&](const aiScene *scene, aiNode *node) {
        for (auto i = 0u; i < node->mNumMeshes; ++i) {
            const auto assimp_mesh = scene->mMeshes[node->mMeshes[i]];
            Mesh mesh;
            if (assimp_mesh->mName.length > 0) mesh.name = assimp_mesh->mName.C_Str();
            mesh.vertex_offset = model.vertices.size();
            mesh.index_offset  = model.indices.size();
            mesh.normal_offset = model.normals.size();

            for (auto j = 0u; j < assimp_mesh->mNumVertices; ++j) {
                auto av = assimp_mesh->mVertices[j];
                mesh.vertices.push_back(model.vertices.size());
                model.vertices.emplace_back(av.x, av.y, av.z);

                if (assimp_mesh->HasNormals()) {
                    av = assimp_mesh->mNormals[j];
                    mesh.normals.push_back(model.normals.size());
                    model.normals.emplace_back(av.x, av.y, av.z);
                }

                if (assimp_mesh->HasTangentsAndBitangents()) {
                    av = assimp_mesh->mTangents[j];
                    // m.tangents.emplace_back(av.x, av.y, av.z);
                    av = assimp_mesh->mBitangents[j];
                    // m.bitangents.emplace_back(av.x, av.y, av.z);
                    // mesh.(bi)tangents.pushback(size)
                }

                if (assimp_mesh->HasTextureCoords(0)) {
                    av = assimp_mesh->mTextureCoords[0][j];
                    model.texture_coordinates.emplace_back(av.x, av.y);
                    // push textures to mesh
                }
            }

            for (auto j = 0u; j < assimp_mesh->mNumFaces; ++j) {
                const auto nf = assimp_mesh->mFaces[j];
                mesh.indices.push_back(model.indices.size());
                model.indices.push_back(nf.mIndices[0]);
                mesh.indices.push_back(model.indices.size());
                model.indices.push_back(nf.mIndices[1]);
                mesh.indices.push_back(model.indices.size());
                model.indices.push_back(nf.mIndices[2]);
            }

            /*auto nmat = scene->mMaterials[assimp_mesh->mMaterialIndex];
            if (nmat) {

                const auto load_texture = [&](aiTextureType type, Mesh::TEXTURE_IDX idx) {
                    if (nmat->GetTextureCount(type) == 0) return;
                    aiString aipath;
                    nmat->GetTexture(type, 0, &aipath);
                    auto txtpath = std::string{path.begin(), path.begin() + path.rfind("/") + 1};
                    txtpath.append(aipath.C_Str());

                    auto it      = std::find_if(model.textures.begin(), model.textures.end(), [&txtpath](auto &&e) {
                        return e.path == txtpath;
                    });
                    auto itfound = it != model.textures.end();

                    if (!itfound) {
                        ModelTexture txt{txtpath, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE};
                        txt.build2d();
                        it = model.textures.emplace(model.textures.end(), std::move(txt));
                    }

                    mesh.present_textures |= (uint32_t)std::pow(2., (double)idx);
                    mesh.textures[idx] = std::distance(model.textures.begin(), it);
                };

                load_texture(aiTextureType_DIFFUSE, mesh.DIFFUSE);
                load_texture(aiTextureType_NORMALS, mesh.NORMAL);
                load_texture(aiTextureType_METALNESS, mesh.METALNESS);
                load_texture(aiTextureType_DIFFUSE_ROUGHNESS, mesh.ROUGHNESS);
                load_texture(aiTextureType_EMISSIVE, mesh.EMISSIVE);
            }*/

            model.meshes.push_back(std::move(mesh));
        }

        for (auto i = 0u; i < node->mNumChildren; ++i) { parse_node(scene, node->mChildren[i]); }
    };

    if (scene) parse_node(scene, scene->mRootNode);

    for (auto &mesh : model.meshes) {
        for (auto i = 0u; i < mesh.vertices.size(); ++i) {
            auto v = model.vertices[mesh.vertices[i]];

            model.aabb.min.x = glm::min(model.aabb.min.x, v.x);
            model.aabb.min.y = glm::min(model.aabb.min.y, v.y);
            model.aabb.min.z = glm::min(model.aabb.min.z, v.z);
            model.aabb.max.x = glm::max(model.aabb.max.x, v.x);
            model.aabb.max.y = glm::max(model.aabb.max.y, v.y);
            model.aabb.max.z = glm::max(model.aabb.max.z, v.z);
        }
    }

    return model;
}

ModelTexture::ModelTexture(std::string_view path, uint32_t filter_minmag, uint32_t wrap_str) {
    this->path = path.data();
    filter_min = filter_mag = filter_minmag;
    wrap_s = wrap_t = wrap_r = wrap_str;
}

void ModelTexture::build2d() {
    auto data = stbi_load(path.data(), &width, &height, &channels, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    glTextureStorage2D(handle, 4, GL_RGBA8, width, height);
    glTextureSubImage2D(handle, 0, 0, 0, width, height, channels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, filter_min);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, filter_mag);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, wrap_s);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, wrap_t);
    glGenerateTextureMipmap(handle);

#ifdef USE_BINDLESS_TEXTURES
    bindless_handle = glGetTextureHandleARB(handle);
    glMakeTextureHandleResidentARB(bindless_handle);
#endif // USE_BINDLESS_TEXTURES

    stbi_image_free(data);
}
