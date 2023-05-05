#include "framebuffer.hpp"

#include <cassert>
#include <chrono>

#include <engine/engine.hpp>
#include <engine/gpu/resource_manager/gpu_res_mgr.hpp>

#include <glad/glad.h>

eng::Framebuffer::Framebuffer(std::initializer_list<FramebufferAttachment> texture_attachments) {
    glCreateFramebuffers(1, &_handle);
    for (const auto &att : texture_attachments) { _attachments[att.target] = att; }
    _configure_attachments();
    _assert_completness();
}

eng::Framebuffer::Framebuffer(Framebuffer &&other) noexcept { *this = std::move(other); }

eng::Framebuffer &eng::Framebuffer::operator=(Framebuffer &&other) noexcept {
    id            = other.id;
    _handle       = other._handle;
    _attachments  = other._attachments;
    other._handle = 0u;
    return *this;
}

eng::Framebuffer::~Framebuffer() { glDeleteFramebuffers(1, &_handle); }

void eng::Framebuffer::bind() { glBindFramebuffer(GL_FRAMEBUFFER, handle()); }

void eng::Framebuffer::_configure_attachments() {
    std::vector<GLenum> color_attachments;

    for (const auto &[target, value] : _attachments) {
        glNamedFramebufferTexture(
            _handle,
            target,
            Engine::instance().get_gpu_res_mgr()->get_resource(value.texture)->handle(),
            value.level);

        if (target >= GL_COLOR_ATTACHMENT0 && target <= GL_COLOR_ATTACHMENT31) {
            color_attachments.push_back(target);
        }
    }

    if (color_attachments.size() > 0) {
        glNamedFramebufferDrawBuffers(_handle, color_attachments.size(), color_attachments.data());
    } else {
        glNamedFramebufferDrawBuffer(_handle, GL_NONE);
    }
}

void eng::Framebuffer::_assert_completness() {
    assert(glCheckNamedFramebufferStatus(_handle, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE
           && "The framebuffer is not complete.");
}
