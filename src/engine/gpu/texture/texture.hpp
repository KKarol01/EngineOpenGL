#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

#include <engine/types/idresource.hpp>

namespace eng {

    enum class TextureType { Diffuse, Normal, Metallic, Roughness, Emissive };

    struct TextureSettings {
        // clang-format off
        explicit TextureSettings();
        explicit TextureSettings(uint32_t format, uint32_t wrap, uint32_t filter, uint32_t mip_count);
        explicit TextureSettings(uint32_t type, uint32_t format, uint32_t wrap, uint32_t filter, uint32_t mip_count);
        explicit TextureSettings(uint32_t type, uint32_t format, uint32_t wrap_s, uint32_t wrap_t, uint32_t wrap_r, uint32_t filter_min, uint32_t filter_mag, uint32_t mip_count=1);
        // clang-format on

        uint32_t type{0}, format{0};
        uint32_t wrap_s{0}, wrap_t{0}, wrap_r{0};
        uint32_t filter_min{0}, filter_mag{0};
        uint32_t mip_count{1};
    };

    struct TextureImageDataDescriptor {
        explicit TextureImageDataDescriptor(const std::string &path) : path{path} {}
        std::string path;
        int xoffset{0}, yoffset{0};
    };
    struct TextureImageData {
        explicit TextureImageData() = default;

        std::string path;
        uint32_t sizex{0}, sizey{0}, channels{0};
        std::shared_ptr<uint8_t> data;
    };

    class Texture : public IdResource<Texture> {
      public:
        explicit Texture() = default;
        explicit Texture(const TextureSettings &settings,
                         std::initializer_list<TextureImageDataDescriptor> data_descs,
                         bool also_store_data_on_cpu = false);

        void bind(uint32_t unit);
        void unbind();
        void make_resident();
        void make_non_resident();

        bool is_bound() const { return _is_bound; }
        bool is_resident() const { return _is_resident; }

        uint32_t handle() const { return _texture_handle; }
        uint64_t bindless_handle() const { return _texture_bindless_handle; }
        uint32_t bound_unit() const { return _bound_unit; }

        std::pair<uint32_t, uint32_t> get_size(uint32_t idx = 0u) const {
            return {_image_data.at(idx).sizex, _image_data.at(idx).sizey};
        }

      private:
        void _load(std::initializer_list<TextureImageDataDescriptor> data_descs,
                   bool also_store_data_on_cpu);
        uint8_t *_load_image(
            std::string_view path, int *sizex, int *sizey, int *channels, int req_channels = 0);

        TextureSettings _settings;
        std::vector<TextureImageData> _image_data;

        bool _is_bound{false}, _is_resident{false};
        uint32_t _bound_unit{0};
        uint32_t _texture_handle{0};
        uint64_t _texture_bindless_handle{0};
    };
} // namespace eng