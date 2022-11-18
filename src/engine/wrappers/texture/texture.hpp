#pragma once

#include <string>
#include <string_view>
#include <cstdint>

struct FILTER_SETTINGS {
    uint32_t min, mag;

    constexpr FILTER_SETTINGS();
    constexpr FILTER_SETTINGS(uint32_t minmag);
    constexpr FILTER_SETTINGS(uint32_t min, uint32_t mag);
};
struct WRAP_SETTINGS {
    uint32_t s, t, r;

    constexpr WRAP_SETTINGS();
    constexpr WRAP_SETTINGS(uint32_t str);
    constexpr WRAP_SETTINGS(uint32_t s, uint32_t t);
    constexpr WRAP_SETTINGS(uint32_t s, uint32_t t, uint32_t r);
};

struct Texture {

    Texture() = default;
    Texture(FILTER_SETTINGS &&filter);
    Texture(FILTER_SETTINGS &&filter, WRAP_SETTINGS &&wrap);
    Texture(Texture &&) noexcept;
    Texture &operator=(Texture &&) noexcept;
    Texture(const Texture &) noexcept;
    Texture &operator=(const Texture &) noexcept;
    ~Texture();

    operator bool() const noexcept { return handle != 0u; }

    void build2d(std::string_view path, uint32_t format);
    void build2d_mips(std::string_view path, uint32_t dimensions);
    void build2d_ms(std::string_view path, uint32_t dimensions, uint32_t samples, bool fixedsamplelocations = true);

    void buildcube(std::string *paths, uint32_t TIME_FORMAT);

    FILTER_SETTINGS filter;
    WRAP_SETTINGS wrap;
    uint32_t handle{0u};
    uint32_t mipmaps{0u};
    uint32_t dimensions{2u};
};