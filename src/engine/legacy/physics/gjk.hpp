#pragma once

#include <vector>
#include <array>
#include <initializer_list>

#include <glm/glm.hpp>

#include "../model/importers.hpp"

//struct Simplex {
//    Simplex() = default;
//    explicit Simplex(std::vector<glm::vec3> vec) {
//        for (auto i = 0u; i < vec.size(); ++i) { verts[i] = *(vec.begin() + i); }
//        size_ = vec.size();
//    }
//
//    auto push_front(glm::vec3 p) {
//        verts = {p, verts[0], verts[1], verts[2]};
//        size_ = glm::min(size_ + 1u, 4u);
//    }
//    auto size() const { return size_; }
//
//    std::array<glm::vec3, 4> verts;
//    std::uint32_t size_ = 0;
//};
//
//class GJK {
//
//  public:
//    bool intersect(const Model &a,
//                   const Model &b,
//                   const glm::mat4 &amodel = glm::mat4{1.f},
//                   const glm::mat4 &bmodel = glm::mat4{1.f});
//
//  private:
//    glm::vec3 support_point(const Model &a, const Model &b, glm::vec3 d);
//    glm::vec3 furthest_point(glm::vec3 dir, const Model &a, bool use_a);
//
//    bool collide(Simplex &s, glm::vec3 &d);
//    bool line(Simplex &s, glm::vec3 &d);
//    bool triangle(Simplex &s, glm::vec3 &d);
//    bool tetrahedron(Simplex &s, glm::vec3 &d);
//
//    uint32_t tries = 0;
//    glm::mat4 amodel, bmodel;
//};