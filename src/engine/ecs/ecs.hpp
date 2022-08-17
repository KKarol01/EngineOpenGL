#pragma once

#include <cstdint>
#include <set>
#include <map>
#include <algorithm>
#include <utility>
#include <vector>

#include "typemap.hpp"
#include "sorted_vec.hpp"

typedef std::uint32_t ComponentID;
typedef std::uint32_t SystemID;
typedef std::uint32_t EntityID;

#define ECS_T template <typename T>

class ECS {
    struct Entity {
        static inline EntityID gid{0u};
        EntityID id{gid++};
        std::set<ComponentID> components;

        bool operator<(const Entity &other) const { return id < other.id; }
    };
    struct EntityCompWrapper {
        EntityID id;
        TypeMap::unique_ptr ptr;

        bool operator<(const EntityCompWrapper &other) const { return id < other.id; }
    };

    TypeMap comp_map, sys_map;

    std::map<ComponentID, SortedVectorUnique<EntityCompWrapper>> comps;
    std::map<EntityID, Entity> entities;

  public:
    ECS_T void register_component() { comp_map.add_type<T>(); }

    EntityID add_entity() {
        Entity e{};
        entities[e.id] = e;
        return e.id;
    }
    ECS_T T *add_component(EntityID eid) {
        auto e  = entities.at(eid);
        auto id = comp_map.get_type_id<T>();

        e.components.insert(id);
        return static_cast<T*>(comps[id].emplace(eid, new T{}).ptr.get());
    }
    ECS_T T *get_component(EntityID eid) {
        return static_cast<T *>(
            comps.at(comp_map.get_type_id<T>()).find(eid, [](auto &&e, auto &&v) { return e.id < v; })->ptr.get());
    }
    ECS_T void add_system() {}

  private:
};