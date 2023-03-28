//#pragma once
//
//#include <assimp/scene.h>
//#include "model.hpp"
//
//class LoopAnimation {
//    float start_time, current_time;
//    Model *model;
//
//  public:
//    LoopAnimation(Model *model);
//    void update();
//    float looped_time;
//
//  private:
//    void animate_from_the_root(Animation *anim, Node *node, const glm::mat4 parent_transform = glm::mat4{1.f});
//
//    void update_meshes_final_mats();
//
//    Channel *find_bone_animation(Animation *anim, Bone &b);
//    Bone *find_bone_by_node(Node *node);
//
//    uint32_t get_rot_keyframe(const Channel *ch, float anim_time);
//    uint32_t get_scl_keyframe(const Channel *ch, float anim_time);
//    uint32_t get_pos_keyframe(const Channel *ch, float anim_time);
//
//    glm::quat interpolate_rotation(const Channel *anim, uint32_t t0, uint32_t t1, float anim_time);
//    glm::vec3 interpolate_position(const Channel *anim, uint32_t t0, uint32_t t1, float anim_time);
//    glm::vec3 interpolate_scale(const Channel *anim, uint32_t t0, uint32_t t1, float anim_time);
//};