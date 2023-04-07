#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include "../../renderer/typedefs.hpp"

namespace eng {
    struct FILTER_SETTINGS {
        uint32_t min, mag;

        constexpr explicit FILTER_SETTINGS();
        constexpr explicit FILTER_SETTINGS(uint32_t minmag);
        constexpr explicit FILTER_SETTINGS(uint32_t min, uint32_t mag);
    };
    struct WRAP_SETTINGS {
        uint32_t s, t, r;

        constexpr explicit WRAP_SETTINGS();
        constexpr explicit WRAP_SETTINGS(uint32_t str);
        constexpr explicit WRAP_SETTINGS(uint32_t s, uint32_t t);
        constexpr explicit WRAP_SETTINGS(uint32_t s, uint32_t t, uint32_t r);
    };

    struct GLTextureDescriptor {
        GLTextureDescriptor() = default;
        GLTextureDescriptor(GLTextureDescriptor &&other) noexcept;
        GLTextureDescriptor &operator=(GLTextureDescriptor &&other) noexcept;
        ~GLTextureDescriptor();

        bool operator==(const GLTextureDescriptor &other) const { return handle == other.handle; }

        FILTER_SETTINGS filter;
        WRAP_SETTINGS wrap;
        TextureID handle{0u};
        uint32_t mipmaps{0u}, dimensions{2u}, format{0u};
        uint32_t width{0u}, height{0u};
    };

    struct GLTexture {
        GLTexture() = default;
        explicit GLTexture(const FILTER_SETTINGS &filter);
        GLTexture(const FILTER_SETTINGS &filter, const WRAP_SETTINGS &wrap);

        void build2d(uint32_t levels, uint32_t format, uint32_t width, uint32_t height);
        void build2d_mips(std::string_view path, uint32_t dimensions);
        void build2d_ms(std::string_view path, uint32_t dimensions, uint32_t samples, bool fixedsamplelocations = true);
        void buildcube(std::string *paths, uint32_t TIME_FORMAT);
        // void buildattachment(uint32_t int_format, size_t width, size_t height);

        GLTextureDescriptor descriptor;
    };
} // namespace eng
