#include "gjk.hpp"

#include <iostream>

bool GJK::intersect(const Model &a, const Model &b, const glm::mat4 &amodel, const glm::mat4 &bmodel) {
    this->amodel = amodel;
    this->bmodel = bmodel;
    tries        = 0;

    Simplex simplex;
    glm::vec3 dir{1.f, 0.f, 0.f};

    auto supp = support_point(a, b, dir);
    simplex.push_front(supp);
    dir = -supp;

    for (;;) {
        supp = support_point(a, b, dir);
        dir  = glm::normalize(dir);
        if (glm::dot(supp, dir) <= 0.f) return false;

        simplex.push_front(supp);

        if (collide(simplex, dir)) return true;
    }

    return false;
}

glm::vec3 GJK::support_point(const Model &a, const Model &b, glm::vec3 d) {
    return furthest_point(d, a, true) - furthest_point(-d, b, false);
}

glm::vec3 GJK::furthest_point(glm::vec3 dir, const Model &a, bool use_a) {
    glm::vec3 max_p{0.f};
    float max_d = -FLT_MAX;

    for (auto &m : a.meshes) {
        for (auto i = 0u; i < m.vertex_count; ++i) {
            auto v     = m.get_vertex(&a, i).position;
            auto model = amodel;
            if (!use_a) model = bmodel;
            auto _v = (model)*glm::vec4{v.x, v.y, v.z, 1.f};
            v       = glm::vec3{_v.x, _v.y, _v.z};

            float dist = glm::dot(v, dir);
            if (dist > max_d) {
                max_d = dist;
                max_p = v;
            }
        }
    }

    return max_p;
}

bool GJK::collide(Simplex &s, glm::vec3 &d) {
     tries++;
    if (tries > 100) return false;

    switch (s.size()) {
    case 2: return line(s, d);
    case 3: return triangle(s, d);
    case 4: return tetrahedron(s, d);
    default: assert("Incorrect simplex size.");
    }

    return false;
}

bool GJK::line(Simplex &s, glm::vec3 &d) {
    auto a = s.verts[0];
    auto b = s.verts[1];

    auto ab = b - a;
    auto ao = -a;

    if (glm::dot(ab, ao) > 0.f) {
        d = glm::cross(glm::cross(ab, ao), ab);
    } else {
        s = Simplex{{a}};
    }

    return false;
}

bool GJK::triangle(Simplex &s, glm::vec3 &d) {
    auto a = s.verts[0];
    auto b = s.verts[1];
    auto c = s.verts[2];

    auto ab = b - a;
    auto ac = c - a;
    auto ao = -a;

    auto abxac = glm::cross(ab, ac);

    if (glm::dot(glm::cross(abxac, ac), ao) > 0.f) {
        if (glm::dot(ac, ao) > 0.f) {
            s = Simplex{{a, c}};
            d = glm::cross(glm::cross(ac, ao), ac);
        }

        else {
            s = Simplex{{a, b}};
            return line(s, d);
        }
    }

    else {
        if (glm::dot(glm::cross(ab, abxac), ao) > 0.f) {
            s = Simplex{{a, b}};
            return line(s, d);
        }

        else {
            if (glm::dot(abxac, ao) > 0.f) {
                d = abxac;
            }

            else {
                s = Simplex{{a, c, b}};
                d = -abxac;
            }
        }
    }

    return false;
}

bool GJK::tetrahedron(Simplex &s, glm::vec3 &dir) {
    auto a = s.verts[0];
    auto b = s.verts[1];
    auto c = s.verts[2];
    auto d = s.verts[3];

    auto ab = b - a;
    auto ac = c - a;
    auto ad = d - a;
    auto ao = -a;

    auto abxac = glm::cross(ab, ac);
    auto acxad = glm::cross(ac, ad);
    auto adxab = glm::cross(ad, ab);

    if (glm::dot(abxac, ao) > 0.f) { return triangle(s = Simplex{{a, b, c}}, dir); }
    if (glm::dot(acxad, ao) > 0.f) { return triangle(s = Simplex{{a, c, d}}, dir); }
    if (glm::dot(adxab, ao) > 0.f) { return triangle(s = Simplex{{a, d, b}}, dir); }

    return true;
}
