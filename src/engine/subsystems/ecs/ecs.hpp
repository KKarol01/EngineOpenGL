#pragma once

#include <cstdint>
#include <set>
#include <map>
#include <algorithm>
#include <utility>
#include <vector>
#include <bitset>

#include "typemap.hpp"
#include "sorted_vec.hpp"

typedef std::uint32_t ComponentID;
typedef std::uint32_t SystemID;
typedef std::uint32_t EntityID;

class SystemBase;

#define ECS_T template <typename T>

class ECS {
    struct EntityComponentWrapper {
        EntityID id;
        TypeMap::unique_ptr pdata;

        bool operator<(const EntityComponentWrapper &other) const { return id < other.id; }
        bool operator==(const EntityComponentWrapper &other) const { return id == other.id; }
    };

    std::map<EntityID, std::set<ComponentID>> entities;
    std::map<ComponentID, SortedVectorUnique<EntityComponentWrapper>> components;
    std::unordered_map<SystemID, SystemBase *> systems;

    TypeMap comps_map, sys_map;

  public:
    EntityID create_entity() {
        static EntityID gid{0u};
        entities[gid];
        return gid++;
    }
    ECS_T auto get_comp_id() { return comps_map.get_type_id<T>(); }
    auto &get_entity(EntityID eid) { return entities.at(eid); }

    ECS_T void register_component() { comps_map.add_type<T>(); }
    ECS_T const T &add_component(EntityID id) {
        auto cid = comps_map.get_type_id<T>();
        entities.at(id).insert(cid);
        return *static_cast<T *>(components[cid].emplace(id, TypeMap::get_unique_ptr(new T{})).pdata.get());
    }
    ECS_T const T &add_component(EntityID id, T *data) {
        auto cid = comps_map.get_type_id<T>();
        entities.at(id).insert(cid);

        const auto ptr = static_cast<const T *>(components[cid].emplace(id, TypeMap::get_unique_ptr(data)).pdata.get());

        update_entity(id);

        return *ptr;
    }
    ECS_T const T &get_component(EntityID id) {
        auto cid = comps_map.get_type_id<T>();
        auto it  = components.at(cid).find(id, [](auto &&e, auto &&v) { return e.id < v; });
        return *static_cast<T *>(it->pdata.get());
    }
    ECS_T void modify_component(EntityID id, std::function<void(T &)> &&func) {
        auto cid = comps_map.get_type_id<T>();
        auto it  = components.at(cid).find(id, [](auto &&e, auto &&v) { return e.id < v; });
        func(*static_cast<T *>(it->pdata.get()));

        update_entity(id);
    }
    void update_entity(EntityID id);
    void update();

    ECS_T SystemID add_system() {
        sys_map.add_type<T>();
        auto id = sys_map.get_type_id<T>();
        auto it = systems.insert({id, new T{}});
        for (auto &[e, comps] : entities) it.first->second->update_entity(e);
        return id;
    }
    ECS_T T *get_system(SystemID id) { return static_cast<T *>(systems.find(id)->second); }

    ~ECS();
};

class SystemBase {
  protected:
    std::set<ComponentID> family;
    template <typename... FAMILY> void set_component_family(ECS *e) { family = {e->get_comp_id<FAMILY>()...}; }
    bool entity_compatible(auto &&e) { return std::includes(e.begin(), e.end(), family.begin(), family.end()); }

  public:
    virtual void update_entity(EntityID id) = 0;
    virtual void update()                   = 0;

    virtual ~SystemBase() = default;
};