#include "texture.hpp"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

constexpr FILTER_SETTINGS::FILTER_SETTINGS() : FILTER_SETTINGS(GL_LINEAR) {}
constexpr FILTER_SETTINGS::FILTER_SETTINGS(uint32_t minmag) : min{minmag}, mag{minmag} {}
constexpr FILTER_SETTINGS::FILTER_SETTINGS(uint32_t min, uint32_t mag) : min{min}, mag{mag} {}
constexpr WRAP_SETTINGS::WRAP_SETTINGS() : WRAP_SETTINGS(GL_REPEAT) {}
constexpr WRAP_SETTINGS::WRAP_SETTINGS(uint32_t str) : s{str}, t{str}, r{str} {}
constexpr WRAP_SETTINGS::WRAP_SETTINGS(uint32_t s, uint32_t t) : s{s}, t{t}, r{0u} {}
constexpr WRAP_SETTINGS::WRAP_SETTINGS(uint32_t s, uint32_t t, uint32_t r) : s{s}, t{t}, r{r} {}

Texture::Texture(FILTER_SETTINGS &&filter) : filter{std::move(filter)} {}
Texture::Texture(FILTER_SETTINGS &&filter, WRAP_SETTINGS &&wrap) : filter{std::move(filter)}, wrap{std::move(wrap)} {}
Texture::Texture(Texture &&other) noexcept { *this = std::move(other); }
Texture &Texture::operator=(Texture &&other) noexcept {
    handle       = other.handle;
    mipmaps      = other.mipmaps;
    filter       = other.filter;
    wrap         = other.wrap;
    other.handle = 0u;
    return *this;
}
Texture::Texture(const Texture &other) noexcept { *this = other; }
Texture &Texture::operator=(const Texture &other) noexcept {
    handle  = other.handle;
    mipmaps = other.mipmaps;
    filter  = other.filter;
    wrap    = other.wrap;
    return *this;
}

Texture::~Texture() { 
    printf("Deleting handle: %u\n", handle);
    glDeleteTextures(1, &handle); }

void Texture::build2d(std::string_view path, uint32_t TIME_FORMAT) {
    int x, y, ch;
    auto data = stbi_load(path.data(), &x, &y, &ch, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    glTextureStorage2D(handle, 0, TIME_FORMAT, x, y);
    glTextureSubImage2D(handle, 0, 0, 0, x, y, ch == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, filter.min);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, filter.mag);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, wrap.s);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, wrap.t);

    stbi_image_free(data);
}

void Texture::build2d_mips(std::string_view path, uint32_t dimensions) { assert(false); }

void Texture::build2d_ms(std::string_view path, uint32_t dimensions, uint32_t samples, bool fixedsamplelocations) { assert(false); }

void Texture::buildcube(std::string *path, uint32_t TIME_FORMAT) {
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &handle);
    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, filter.min);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, filter.mag);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, wrap.s);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, wrap.t);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_R, wrap.r);

    glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
    for (int i = 0; i < 6; ++i) {
        int x, y, ch;
        auto data = stbi_load(path[i].data(), &x, &y, &ch, 0);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, TIME_FORMAT, x, y, 0, ch == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
}
