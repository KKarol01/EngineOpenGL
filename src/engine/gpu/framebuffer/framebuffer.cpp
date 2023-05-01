#include "framebuffer.hpp"

#include <glad/glad.h>

//eng::GLFboDescriptor::GLFboDescriptor() { glCreateFramebuffers(1, &handle); }
//
//eng::GLFboDescriptor::GLFboDescriptor(GLFboDescriptor &&other) noexcept {
//    handle      = other.handle;
//    attachments = other.attachments;
//
//    other.handle = 0u;
//}
//
//eng::GLFboDescriptor &eng::GLFboDescriptor::operator=(GLFboDescriptor &&other) noexcept {
//    *this = std::move(other);
//    return *this;
//}
//
//eng::GLFboDescriptor::~GLFboDescriptor() { glDeleteFramebuffers(1, &handle); }
//
//void eng::GLFbo::add_attachment() {
////glFramebufferTexture2D()
//}
