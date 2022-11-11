#pragma once

#include <string>
#include <array>
#include <unordered_map>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>

#include "../renderer/typedefs.hpp"

class AbstractModelImporter {
  public:
    AbstractModelImporter(const std::string &dir_name, unsigned import_flags);
    Model import_model();

  protected:
    virtual Mesh process_mesh(const aiMesh *) = 0;

  public:
    inline static std::string models_base_folder{"3dmodels/"};

  protected:
    std::string model_path;
    const aiScene *scene;
    Assimp::Importer model_importer;
};

class PBRModelImporter final : public AbstractModelImporter {
  public:
    PBRModelImporter(const std::string &dir_name, unsigned import_flags = 0)
        : AbstractModelImporter(dir_name, import_flags) {}

  protected:
    Mesh process_mesh(const aiMesh *mesh) override;
};