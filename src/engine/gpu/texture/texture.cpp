#include "texture.hpp"

#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h>

namespace eng {
    Texture::Texture(const std::string &texture_path) {
        this->_texture_path  = texture_path;
        std::string base_dir = texture_path;
        auto image_data
            = stbi_load(base_dir.c_str(), (int *)&_sizex, (int *)&_sizey, (int *)&_channels, 0);

        if (image_data == nullptr) {
            fprintf(stderr, "Image not found at path: %s\n", texture_path.c_str());
            assert(false);
        }

        glCreateTextures(GL_TEXTURE_2D, 1, &_texture_handle);
        glTextureStorage2D(_texture_handle, 4, GL_RGB8, _sizex, _sizey);
        glTextureSubImage2D(_texture_handle,
                            0,
                            0,
                            0,
                            _sizex,
                            _sizey,
                            _channels == 3 ? GL_RGB : GL_RGBA,
                            GL_UNSIGNED_BYTE,
                            (void *)image_data);
        glGenerateTextureMipmap(_texture_handle);
        glTextureParameteri(_texture_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(_texture_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(_texture_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(_texture_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        _image_data = std::shared_ptr<uint8_t>{image_data, stbi_image_free};
    }

    void Texture::bind(uint32_t unit) {
        _bound_unit = unit;
        _is_bound   = true;
        glBindTextureUnit(unit, handle());
    }

    void Texture::unbind() {
        _is_bound = false;
        glBindTextureUnit(bound_unit(), handle());
    }

    void Texture::make_resident() {
        if (bindless_handle() == 0ull) {
            _texture_bindless_handle = glGetTextureHandleARB(handle());
        }

        _is_resident = true;
        glMakeTextureHandleResidentARB(bindless_handle());
    }

    void Texture::make_non_resident() {
        _is_resident = false;
        glMakeTextureHandleNonResidentARB(bindless_handle());
    }
} // namespace eng