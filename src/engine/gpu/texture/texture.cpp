#include "texture.hpp"

#include <glad/glad.h>

#include <array>
#include <vector>
#include <memory>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace eng {
    constexpr FILTER_SETTINGS::FILTER_SETTINGS() : FILTER_SETTINGS(GL_LINEAR) {}
    constexpr FILTER_SETTINGS::FILTER_SETTINGS(uint32_t minmag) : min{minmag}, mag{minmag} {}
    constexpr FILTER_SETTINGS::FILTER_SETTINGS(uint32_t min, uint32_t mag) : min{min}, mag{mag} {}
    constexpr WRAP_SETTINGS::WRAP_SETTINGS() : WRAP_SETTINGS(GL_CLAMP_TO_EDGE) {}
    constexpr WRAP_SETTINGS::WRAP_SETTINGS(uint32_t str) : s{str}, t{str}, r{str} {}
    constexpr WRAP_SETTINGS::WRAP_SETTINGS(uint32_t s, uint32_t t) : s{s}, t{t}, r{0u} {}
    constexpr WRAP_SETTINGS::WRAP_SETTINGS(uint32_t s, uint32_t t, uint32_t r) : s{s}, t{t}, r{r} {}

    GLTextureDescriptor::GLTextureDescriptor(GLTextureDescriptor &&other) noexcept {
        filter       = other.filter;
        wrap         = other.wrap;
        mipmaps      = other.mipmaps;
        dimensions   = other.dimensions;
        format       = other.format;
        width        = other.width;
        height       = other.height;
        handle       = other.handle;
        other.handle = 0u;
    }
    GLTextureDescriptor &GLTextureDescriptor::operator=(GLTextureDescriptor &&other) noexcept {
        *this = std::move(other);
        return *this;
    }
    GLTextureDescriptor::~GLTextureDescriptor() { glDeleteTextures(1, &handle); }

    GLTexture::GLTexture(const FILTER_SETTINGS &filter) { descriptor.filter = filter; }

    GLTexture::GLTexture(const FILTER_SETTINGS &filter, const WRAP_SETTINGS &wrap) {
        descriptor.filter = filter;
        descriptor.wrap   = wrap;
    }

    void GLTexture::build2d(uint32_t levels, uint32_t format, uint32_t width, uint32_t height) {
        auto &handle       = descriptor.handle;
        const auto &filter = descriptor.filter;
        const auto &wrap   = descriptor.wrap;

        assert((handle == 0u, "Don't call building functions more than once on the same object."));

        glCreateTextures(GL_TEXTURE_2D, 1, &handle);
        glTextureStorage2D(handle, levels, format, width, height);
        glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, filter.min);
        glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, filter.mag);
        glTextureParameteri(handle, GL_TEXTURE_WRAP_S, wrap.s);
        glTextureParameteri(handle, GL_TEXTURE_WRAP_T, wrap.t);
    }

    void GLTexture::build2d_mips(std::string_view path, uint32_t dimensions) { assert(false); }

    void
    GLTexture::build2d_ms(std::string_view path, uint32_t dimensions, uint32_t samples, bool fixedsamplelocations) {
        assert(false);
    }

    void GLTexture::buildcube(std::string *path, uint32_t TIME_FORMAT) {
        /* glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &handle);
         glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, filter.min);
         glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, filter.mag);
         glTextureParameteri(handle, GL_TEXTURE_WRAP_S, wrap.s);
         glTextureParameteri(handle, GL_TEXTURE_WRAP_T, wrap.t);
         glTextureParameteri(handle, GL_TEXTURE_WRAP_R, wrap.r);

         glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
         for (int i = 0; i < 6; ++i) {
             int x, y, ch;
             auto data = stbi_load(path[i].data(), &x, &y, &ch, 0);

             glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                          0,
                          TIME_FORMAT,
                          x,
                          y,
                          0,
                          ch == 3 ? GL_RGB : GL_RGBA,
                          GL_UNSIGNED_BYTE,
                          data);
             stbi_image_free(data);
         }*/
    }

} // namespace eng