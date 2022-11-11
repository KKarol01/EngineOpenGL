#include "importers.hpp"

#include <filesystem>
#include <ranges>
#include <iostream>

#include <glad/glad.h>
#include <stb_image.h>

AbstractModelImporter::AbstractModelImporter(const std::string &dir_name, unsigned import_flags) {
    auto path_str = models_base_folder + dir_name;
    if (path_str.ends_with('/') == false) path_str.append("/");

    if (std::filesystem::is_directory(path_str) == false) {
        std::ostringstream ss;
        ss << "Path to the model does not exist. The path was: '" << path_str << "'.";

        throw std::runtime_error{ss.str()};
    }

    model_path = path_str;
    std::filesystem::directory_iterator dir_it{path_str};

    auto scene_files = dir_it | std::views::filter([](auto &&entry) {
                           return entry.is_regular_file() && entry.path().string().ends_with(".gltf");
                       })
                       | std::views::transform([](auto &&entry) { return entry.path().string(); })
                       | std::views::take(1);

    scene = model_importer.ReadFile(*scene_files.begin(), import_flags);

    if (scene == nullptr) {
        throw std::runtime_error{(std::ostringstream{} << "the scene '" << path_str << "' could not be loaded.").str()};
    }

    std::cout << scene->mRootNode->mName.C_Str();
}

Model AbstractModelImporter::import_model() {
    Model model;
    for (auto i = 0u; i < scene->mNumMeshes; ++i) { model.meshes.emplace_back(process_mesh(scene->mMeshes[i])); }

    return model;
}

Mesh PBRModelImporter::process_mesh(const aiMesh *mesh) {
    Mesh m;
    m.name = mesh->mName.C_Str();

    const auto insert_n_zeroes = []<typename Vec>(int n, Vec &vector) {
        typename Vec::value_type zero{0.f};
        for (int i = 0; i < n; ++i) vector.push_back(zero);
    };
    const auto insert_xyz = [](int num_comps, const auto &wrapper, auto &vec) {
        if (num_comps >= 1) vec.push_back(wrapper.x);
        if (num_comps >= 2) vec.push_back(wrapper.y);
        if (num_comps >= 3) vec.push_back(wrapper.z);
    };

    for (auto i = 0u; i < mesh->mNumFaces; ++i) {
        auto face = mesh->mFaces[i];

        m.indices.push_back(face.mIndices[0]);
        m.indices.push_back(face.mIndices[1]);
        m.indices.push_back(face.mIndices[2]);
    }

    for (auto i = 0u; i < mesh->mNumVertices; ++i) {
        auto vertex = mesh->mVertices[i];

        insert_xyz(3, vertex, m.vertices);

        if (mesh->HasNormals()) {
            insert_xyz(3, mesh->mNormals[i], m.vertices);
        } else {
            insert_n_zeroes(3, m.vertices);
        }

        if (mesh->HasTangentsAndBitangents()) {
            insert_xyz(3, mesh->mTangents[i], m.vertices);
            insert_xyz(3, mesh->mBitangents[i], m.vertices);
        } else {
            insert_n_zeroes(6, m.vertices);
        }

        if (mesh->HasTextureCoords(0)) {
            insert_xyz(2, mesh->mTextureCoords[0][i], m.vertices);
        } else {
            insert_n_zeroes(2, m.vertices);
        }
    }

    if (mesh->mMaterialIndex < scene->mNumMaterials) {
        const auto load_texture = [this](aiMaterial *material, auto txt_type, auto idx = 0u) -> TextureDesc {
            aiString local_path;
            material->GetTexture(txt_type, idx, &local_path);

            std::string real_path = model_path + local_path.C_Str();

            int w, h, ch;
            auto data = stbi_load(real_path.c_str(), &w, &h, &ch, 0);

            if (data == nullptr) {
                std::ostringstream ss;
                ss << "could not load model's texture '" << real_path << "'.";
                throw std::runtime_error{ss.str()};
            }

            TextureDesc t;
            t.width        = w;
            t.height       = h;
            t.num_channels = ch;

            glCreateTextures(GL_TEXTURE_2D, 1, &t.texture);
            glTextureStorage2D(t.texture, 1, GL_RGBA8, w, h);
            glTextureSubImage2D(t.texture, 0, 0, 0, w, h, ch == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
            glTextureParameteri(t.texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(t.texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(t.texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(t.texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

            t.bindless_handle = glGetTextureHandleARB(t.texture);
            glMakeTextureHandleResidentARB(t.bindless_handle);

            stbi_image_free(data);
            return t;
        };
        auto mat = scene->mMaterials[mesh->mMaterialIndex];

        if (!mat) return m;

        Material material;
        material.name = mat->GetName().C_Str();

        if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            material.arr[(int)TEX::DIFFUSE] = load_texture(mat, aiTextureType_DIFFUSE, 0);
        }
        if (mat->GetTextureCount(aiTextureType_NORMALS) > 0) {
            material.arr[(int)TEX::NORMAL] = load_texture(mat, aiTextureType_NORMALS, 0);
        }
        if (mat->GetTextureCount(aiTextureType_METALNESS) > 0) {
            material.arr[(int)TEX::METALLIC] = load_texture(mat, aiTextureType_METALNESS, 0);
        }
        if (mat->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0) {
            material.arr[(int)TEX::ROUGHNESS] = load_texture(mat, aiTextureType_DIFFUSE_ROUGHNESS, 0);
        }

        m.material = new Material{material};
    }

    return m;
}
