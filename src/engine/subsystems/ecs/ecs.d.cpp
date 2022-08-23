#include "ecs.d.hpp"

ECS_T auto ECS::get_comp_id() { return comps_map.get_type_id<T>(); }

ECS_T void ECS::register_component() { comps_map.add_type<T>(); }

ECS_T const T &ECS::add_component(EntityID id) {
    auto cid = comps_map.get_type_id<T>();
    entities.at(id).insert(cid);
    return *static_cast<T *>(components[cid].emplace(id, TypeMap::get_unique_ptr(new T{})).pdata.get());
}

ECS_T const T &ECS::add_component(EntityID id, T *data) {
    auto cid = comps_map.get_type_id<T>();
    entities.at(id).insert(cid);

    const auto ptr = static_cast<const T *>(components[cid].emplace(id, TypeMap::get_unique_ptr(data)).pdata.get());

    update_entity(id);

    return *ptr;
}

ECS_T const T &ECS::get_component(EntityID id) {
    auto cid = comps_map.get_type_id<T>();
    auto it  = components.at(cid).find(id, [](auto &&e, auto &&v) { return e.id < v; });
    return *static_cast<T *>(it->pdata.get());
}

ECS_T void ECS::modify_component(EntityID id, std::function<void(T &)> &&func) {
    auto cid = comps_map.get_type_id<T>();
    auto it  = components.at(cid).find(id, [](auto &&e, auto &&v) { return e.id < v; });
    func(*static_cast<T *>(it->pdata.get()));

    update_entity(id);
}

ECS_T SystemID ECS::add_system() {
    sys_map.add_type<T>();
    auto id = sys_map.get_type_id<T>();
    auto it = systems.insert({id, new T{}});
    for (auto &[e, comps] : entities) it.first->second->update_entity(e);
    return id;
}

ECS_T T *ECS::get_system(SystemID id) { return static_cast<T *>(systems.find(id)->second); }