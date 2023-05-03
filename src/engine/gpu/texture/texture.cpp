#include "texture.hpp"

#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h>

namespace eng {
    Texture::Texture(const TextureSettings &settings,
                     const TextureImageDataDescriptor &data_descs,
                     bool also_store_data_on_cpu)
        : _settings{settings} {
        _load(data_descs, also_store_data_on_cpu);
    }

    void Texture::bind(uint32_t unit) {
        _bound_unit = unit;
        _is_bound   = true;
        glBindTextureUnit(unit, handle());
    }

    void Texture::unbind() {
        _is_bound = false;
        glBindTextureUnit(bound_unit(), 0);
    }

    void Texture::make_resident() {
        if (bindless_handle() == 0ull) {
            _bindless_handle = glGetTextureHandleARB(handle());
        }

        _is_resident = true;
        glMakeTextureHandleResidentARB(bindless_handle());
    }

    void Texture::make_non_resident() {
        _is_resident = false;
        glMakeTextureHandleNonResidentARB(bindless_handle());
    }

    void Texture::_load(const TextureImageDataDescriptor &data_desc, bool also_store_data_on_cpu) {
        glCreateTextures(_settings.type, 1, &_handle);

        switch (_settings.type) {
        case GL_TEXTURE_2D: {
            const auto &desc = data_desc;
            auto &img_data   = _image_data;
            img_data.path    = desc.path;

            // clang-format off
            auto pixels = (uint8_t*)0;
            if(img_data.path.empty()==false) {
			    pixels = _load_image(desc.path, (int *)&img_data.sizex, (int *)&img_data.sizey, (int *)&img_data.channels, 0);
            }else {
				img_data.channels =3;
				img_data.sizex = data_desc.xoffset;
				img_data.sizey = data_desc.yoffset;
            }
            glTextureStorage2D(_handle, _settings.mip_count, _settings.format, img_data.sizex, img_data.sizey);
            glTextureSubImage2D(_handle, 0, desc.xoffset,desc.yoffset, img_data.sizex, img_data.sizey, img_data.channels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            // clang-format on

            if (also_store_data_on_cpu) {
                img_data.data = std::shared_ptr<uint8_t>(pixels, stbi_image_free);
            } else if (pixels != nullptr) {
                stbi_image_free(pixels);
            }
        } break;
        deafult:
            assert(false && "Unrecognized texture type");
        }

        glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateTextureMipmap(_handle);
    }
    uint8_t *Texture::_load_image(
        std::string_view path, int *sizex, int *sizey, int *channels, int req_channels) {
        auto pixels = stbi_load(path.data(), sizex, sizey, channels, req_channels);

        if (pixels == nullptr) {
            fprintf(stderr, "Image not found at path: %s\n", path.data());
            assert(false);
        }

        return pixels;
    }

    // clang-format off

    TextureSettings::TextureSettings() : TextureSettings(GL_TEXTURE_2D, GL_RGB8, GL_CLAMP_TO_EDGE, GL_LINEAR, 1) {}
    TextureSettings::TextureSettings(uint32_t format, uint32_t wrap, uint32_t filter, uint32_t mip_count) : TextureSettings(GL_TEXTURE_2D, format, wrap, filter, mip_count) {}
    TextureSettings::TextureSettings(uint32_t type, uint32_t format, uint32_t wrap, uint32_t filter, uint32_t mip_count) : TextureSettings(type, format, wrap, wrap, wrap, filter, filter, mip_count) {}
    TextureSettings::TextureSettings(uint32_t type, uint32_t format, uint32_t wrap_s, uint32_t wrap_t, uint32_t wrap_r, uint32_t filter_min, uint32_t filter_mag, uint32_t mip_count) : type{type}, format{format}, wrap_s{wrap_s}, wrap_t{wrap_t}, wrap_r{wrap_r}, filter_min{filter_min}, filter_mag{filter_mag}, mip_count{mip_count} {}

    // clang-format on
} // namespace eng