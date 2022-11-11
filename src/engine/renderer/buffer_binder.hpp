#pragma once

#include <cstdint>

struct BufferBinder {
    virtual void bind()     = 0;
    virtual ~BufferBinder() = default;
};
struct TargetBaseBufferBinder : public BufferBinder {
    TargetBaseBufferBinder(uint32_t target, uint32_t buffer, uint32_t binding)
        : target{target}, buffer{buffer}, binding{binding} {}

    void bind() override;

  private:
    std::uint32_t target, buffer, binding;
};
