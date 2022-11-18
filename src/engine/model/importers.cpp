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

    std::function<void(const aiScene*, aiNode*)> parse_node = [&](const aiScene *scene, aiNode *node) {
        for (auto i = 0u; i < node->mNumMeshes; ++i) {
            const auto nmsh = scene->mMeshes[node->mMeshes[i]];
            assert(("asdf", nmsh->HasTextureCoords(0)));
            Model::Mesh msh;
            msh.vertex_offset = m.vertices.size()/8 ;
            msh.vertex_count  = nmsh->mNumVertices ;
            msh.index_offset  = m.indices.size();
            msh.index_count   = nmsh->mNumFaces * 3;

            for (auto j = 0u; j < nmsh->mNumVertices; ++j) {
                m.vertices.push_back(nmsh->mVertices[j].x);
                m.vertices.push_back(nmsh->mVertices[j].y);
                m.vertices.push_back(nmsh->mVertices[j].z);
                m.vertices.push_back(nmsh->mNormals[j].x);
                m.vertices.push_back(nmsh->mNormals[j].y);
                m.vertices.push_back(nmsh->mNormals[j].z);
                m.vertices.push_back(nmsh->mTextureCoords[0][j].x);
                m.vertices.push_back(nmsh->mTextureCoords[0][j].y);
            }

            for (auto j = 0u; j < nmsh->mNumFaces; ++j) {
                const auto nf = nmsh->mFaces[j];
                m.indices.push_back(nf.mIndices[0] + msh.vertex_offset);
                m.indices.push_back(nf.mIndices[1] + msh.vertex_offset);
                m.indices.push_back(nf.mIndices[2] + msh.vertex_offset);
            }

            m.meshes.push_back(std::move(msh));
        }

        for(auto i=0u; i<node->mNumChildren; ++i) {
            parse_node(scene, node->mChildren[i]);
        }
    };
    parse_node(scene, scene->mRootNode);

    return m;
}
