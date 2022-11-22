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

    Model m;

    std::function<void(const aiScene *, aiNode *)> parse_node = [&](const aiScene *scene, aiNode *node) {
        for (auto i = 0u; i < node->mNumMeshes; ++i) {
            const auto nmsh = scene->mMeshes[node->mMeshes[i]];
            Model::Mesh msh;

            msh.stride = 3;
            for (auto j = 0u; j < nmsh->mNumVertices; ++j) {
                m.vertices.push_back(nmsh->mVertices[j].x);
                m.vertices.push_back(nmsh->mVertices[j].y);
                m.vertices.push_back(nmsh->mVertices[j].z);

                if (nmsh->HasNormals()) {
                    msh.stride += 3;
                    m.vertices.push_back(nmsh->mNormals[j].x);
                    m.vertices.push_back(nmsh->mNormals[j].y);
                    m.vertices.push_back(nmsh->mNormals[j].z);
                }

                if (nmsh->HasTangentsAndBitangents()) {
                    msh.stride += 6;
                    m.vertices.push_back(nmsh->mTangents[j].x);
                    m.vertices.push_back(nmsh->mTangents[j].y);
                    m.vertices.push_back(nmsh->mTangents[j].z);
                    m.vertices.push_back(nmsh->mBitangents[j].x);
                    m.vertices.push_back(nmsh->mBitangents[j].y);
                    m.vertices.push_back(nmsh->mBitangents[j].z);
                }

                if (nmsh->HasTextureCoords(0)) {
                    msh.stride += 2;
                    m.vertices.push_back(nmsh->mTextureCoords[0][j].x);
                    m.vertices.push_back(nmsh->mTextureCoords[0][j].y);
                }
            }

            msh.vertex_count  = nmsh->mNumVertices;
            msh.vertex_offset = m.vertices.size() / msh.stride;
            msh.index_offset  = m.indices.size();
            msh.index_count   = nmsh->mNumFaces * 3;

            for (auto j = 0u; j < nmsh->mNumFaces; ++j) {
                const auto nf = nmsh->mFaces[j];
                m.indices.push_back(nf.mIndices[0] + msh.vertex_offset);
                m.indices.push_back(nf.mIndices[1] + msh.vertex_offset);
                m.indices.push_back(nf.mIndices[2] + msh.vertex_offset);
            }

            auto nmat = scene->mMaterials[nmsh->mMaterialIndex];
            if (nmat) {

                const auto load_texture = [&](aiTextureType type, Model::Mesh::TEXTURE_IDX idx) {
                    if (nmat->GetTextureCount(type) == 0) return;
                    aiString path;
                    nmat->GetTexture(aiTextureType_DIFFUSE, 0, &path);

                    auto it      = std::find(m.textures.begin(), m.textures.end(), path.C_Str());
                    auto itfound = it != m.textures.end();

                    if (!itfound) { it = m.textures.insert(m.textures.end(), path.C_Str()); }

                    msh.present_textures |= (uint32_t)std::pow(2., (double)idx);
                    msh.textures[idx] = std::distance(m.textures.begin(), it);
                };

                load_texture(aiTextureType_DIFFUSE, msh.DIFFUSE);
                load_texture(aiTextureType_NORMALS, msh.NORMAL);
                load_texture(aiTextureType_METALNESS, msh.METALNESS);
                load_texture(aiTextureType_DIFFUSE_ROUGHNESS, msh.ROUGHNESS);
                load_texture(aiTextureType_EMISSIVE, msh.EMISSIVE);
            }

            m.meshes.push_back(std::move(msh));
        }

        for (auto i = 0u; i < node->mNumChildren; ++i) { parse_node(scene, node->mChildren[i]); }
    };

    if (scene) parse_node(scene, scene->mRootNode);

    return m;
}
