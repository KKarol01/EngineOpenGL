#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include <engine/types/idresource.hpp>

namespace eng {

    enum class TextureType { Diffuse, Normal, Metallic, Roughness, Emissive };

    class Texture : public IdResource<Texture> {
      public:
        explicit Texture() = default;
        explicit Texture(const std::string &texture_path);

        void bind(uint32_t unit);
        void unbind();
        void make_resident();
        void make_non_resident();

        bool is_bound() const { return _is_bound; }
        bool is_resident() const { return _is_resident; }

        uint32_t handle() const { return _texture_handle; }
        uint64_t bindless_handle() const { return _texture_bindless_handle; }
        uint32_t bound_unit() const { return _bound_unit; }

        std::pair<uint32_t, uint32_t> get_size() const { return {_sizex, _sizey}; }

      private:
        bool _is_bound{false}, _is_resident{false};
        uint32_t _bound_unit{0};
        uint32_t _sizex{0}, _sizey{0}, _channels{0};
        uint32_t _texture_handle{0};
        uint64_t _texture_bindless_handle{0};
        std::string _texture_path;
        std::shared_ptr<uint8_t> _image_data;
    };
} // namespace eng