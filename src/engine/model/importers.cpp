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
            if (nmsh->HasNormals()) msh.stride += 3;
            if (nmsh->HasTangentsAndBitangents()) msh.stride += 6;
            if (nmsh->HasTextureCoords(0)) msh.stride += 2;

            msh.vertex_count  = nmsh->mNumVertices;
            msh.vertex_offset = m.vertices.size() / msh.stride;
            msh.index_offset  = m.indices.size();
            msh.index_count   = nmsh->mNumFaces * 3;

            for (auto j = 0u; j < nmsh->mNumVertices; ++j) {
                m.vertices.push_back(nmsh->mVertices[j].x);
                m.vertices.push_back(nmsh->mVertices[j].y);
                m.vertices.push_back(nmsh->mVertices[j].z);

                if (nmsh->HasNormals()) {
                    m.vertices.push_back(nmsh->mNormals[j].x);
                    m.vertices.push_back(nmsh->mNormals[j].y);
                    m.vertices.push_back(nmsh->mNormals[j].z);
                }

                if (nmsh->HasTangentsAndBitangents()) {
                    m.vertices.push_back(nmsh->mTangents[j].x);
                    m.vertices.push_back(nmsh->mTangents[j].y);
                    m.vertices.push_back(nmsh->mTangents[j].z);
                    m.vertices.push_back(nmsh->mBitangents[j].x);
                    m.vertices.push_back(nmsh->mBitangents[j].y);
                    m.vertices.push_back(nmsh->mBitangents[j].z);
                }

                if (nmsh->HasTextureCoords(0)) {
                    m.vertices.push_back(nmsh->mTextureCoords[0][j].x);
                    m.vertices.push_back(nmsh->mTextureCoords[0][j].y);
                }
            }

            for (auto j = 0u; j < nmsh->mNumFaces; ++j) {
                const auto nf = nmsh->mFaces[j];
                m.indices.push_back(nf.mIndices[0]);
                m.indices.push_back(nf.mIndices[1]);
                m.indices.push_back(nf.mIndices[2]);
            }

            auto nmat = scene->mMaterials[nmsh->mMaterialIndex];
            if (nmat) {

                const auto load_texture = [&](aiTextureType type, Model::Mesh::TEXTURE_IDX idx) {
                    if (nmat->GetTextureCount(type) == 0) return;
                    aiString aipath;
                    nmat->GetTexture(type, 0, &aipath);
                    auto txtpath = std::string{path.begin(), path.begin() + path.rfind("/") + 1};
                    txtpath.append(aipath.C_Str());

                    auto it      = std::find_if(m.textures.begin(),
                                           m.textures.end(),
                                           [&txtpath](auto &&e) { return e.path == txtpath; });
                    auto itfound = it != m.textures.end();

                    if (!itfound) {
                        Model::Texture txt{txtpath, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE};
                        txt.build2d();
                        it = m.textures.emplace(m.textures.end(), std::move(txt));
                    }

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

Model::Texture::Texture(std::string_view path, uint32_t filter_minmag, uint32_t wrap_str) {
    this->path = path.data();
    filter_min = filter_mag = filter_minmag;
    wrap_s = wrap_t = wrap_r = wrap_str;
}

void Model::Texture::build2d() {
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
