#include "animation.hpp"

#include <iostream>
#include <functional>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

extern glm::mat4 glm_cast(const aiMatrix4x4 &mat);

LoopAnimation::LoopAnimation(Model *model) : start_time{(float)glfwGetTime()}, current_time{start_time}, model{model} {
    assert(model);
}

void LoopAnimation::update() {
    current_time = (float)glfwGetTime();
    animate_from_the_root(&model->animations[0], model->skeleton.root_node.get());
    update_meshes_final_mats();
}

void LoopAnimation::animate_from_the_root(Animation *anim, Node *node, const glm::mat4 parent_transform) {
    float timedelta       = current_time - start_time;
    float ticks_in_sec    = anim->ticks_per_sec;
    float anime_time      = anim->duration_ticks;
    float anim_time_ticks = fmod(current_time * ticks_in_sec, anime_time);
    looped_time = fmod(current_time, anime_time / ticks_in_sec);

    auto &skeleton        = model->skeleton;

    auto bone           = skeleton.get_bone(node->name);
    auto anim_node      = bone ? find_bone_animation(anim, *bone) : nullptr;
    auto node_transform = node->transform_mat;

    if (anim_node) {
        auto rot_idx = get_rot_keyframe(anim_node, anim_time_ticks);
        auto scl_idx = get_scl_keyframe(anim_node, anim_time_ticks);
        auto pos_idx = get_pos_keyframe(anim_node, anim_time_ticks);

        auto rot = interpolate_rotation(anim_node, rot_idx, rot_idx + 1, anim_time_ticks);
        auto scl = interpolate_scale(anim_node, scl_idx, scl_idx + 1, anim_time_ticks);
        auto pos = interpolate_position(anim_node, pos_idx, pos_idx + 1, anim_time_ticks);

        auto transform = glm::translate(glm::mat4{1.f}, pos) * glm::mat4_cast(rot) * glm::scale(glm::mat4{1.f}, scl);

        node_transform = transform;
    }

    auto global_transform = parent_transform * node_transform;
    if (bone) { model->skeleton.final_mats[bone->id] = global_transform * bone->bone_mat; }

    for (auto &n : node->children) { animate_from_the_root(anim, &n, global_transform); }
}

void LoopAnimation::update_meshes_final_mats() {
    return;
    for (auto &[n, b] : model->skeleton.bones) {
        auto mat = b.bone_mat;
        auto n   = model->skeleton.find_node(b.name);
        while (n) {
            mat = n->transform_mat * mat;
            n   = n->parent;
        }
        model->skeleton.final_mats[b.id] = mat;
    }
}

Channel *LoopAnimation::find_bone_animation(Animation *anim, Bone &b) {
    for (auto &ch : model->animations[0].channels) {
        if (ch.node_name == b.name) return &ch;
    }

    return nullptr;
}

Bone *LoopAnimation::find_bone_by_node(Node *node) { return model->skeleton.get_bone(node); }

uint32_t LoopAnimation::get_rot_keyframe(const Channel *ch, float anim_time) {
    auto i = 0u;
    for (; i < ch->rotation_keys.size() - 1; ++i) {
        if (ch->rotation_keys[i + 1].time > anim_time) return i;
    }
    return i;
}

uint32_t LoopAnimation::get_scl_keyframe(const Channel *ch, float anim_time) {
    auto i = 0u;
    for (; i < ch->scaling_keys.size() - 1; ++i) {
        if (ch->scaling_keys[i + 1].time > anim_time) return i;
    }
    return i;
}

uint32_t LoopAnimation::get_pos_keyframe(const Channel *ch, float anim_time) {
    auto i = 0u;
    for (; i < ch->position_keys.size() - 1; ++i) {
        if (ch->position_keys[i + 1].time > anim_time) return i;
    }
    return i;
}

glm::quat LoopAnimation::interpolate_rotation(const Channel *anim, uint32_t t0, uint32_t t1, float anim_time) {
    assert(t1 < anim->rotation_keys.size());

    const auto &a    = anim->rotation_keys[t0];
    const auto &b    = anim->rotation_keys[t1];
    float anim_delta = b.time - a.time;

    float transition_progress = (anim_time - a.time) / anim_delta;
    transition_progress       = glm::clamp(transition_progress, 0.f, 1.f);

    return glm::slerp(a.value, b.value, transition_progress);
}

glm::vec3 LoopAnimation::interpolate_position(const Channel *anim, uint32_t t0, uint32_t t1, float anim_time) {
    assert(t1 < anim->position_keys.size());

    const auto &a    = anim->position_keys[t0];
    const auto &b    = anim->position_keys[t1];
    float anim_delta = b.time - a.time;

    float transition_progress = (anim_time - a.time) / anim_delta;
    transition_progress       = glm::clamp(transition_progress, 0.f, 1.f);

    return glm::mix(a.value, b.value, transition_progress);
}

glm::vec3 LoopAnimation::interpolate_scale(const Channel *anim, uint32_t t0, uint32_t t1, float anim_time) {
    assert(t1 < anim->scaling_keys.size());

    const auto &a    = anim->scaling_keys[t0];
    const auto &b    = anim->scaling_keys[t1];
    float anim_delta = b.time - a.time;

    float transition_progress = (anim_time - a.time) / anim_delta;
    transition_progress       = glm::clamp(transition_progress, 0.f, 1.f);

    return glm::mix(a.value, b.value, transition_progress);
}
