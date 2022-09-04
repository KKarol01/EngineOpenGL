#pragma once

#include <cstdint>
#include <set>
#include <map>
#include <functional>
#include <utility>
#include <vector>

#include "typemap.hpp"
#include "../../types/types.hpp"

namespace eng {
    #define ECS_T template <typename T>

    typedef std::uint32_t ComponentID;
    typedef std::uint32_t SystemID;
    typedef std::uint32_t EntityID;
    typedef std::set<ComponentID> Entity;

    class SystemBase;

    class ECS {
        struct EntityComponentWrapper {
            EntityID id;
            TypeMap::unique_ptr pdata;

            bool operator<(const EntityComponentWrapper &other) const { return id < other.id; }
            bool operator==(const EntityComponentWrapper &other) const { return id == other.id; }
        };

      public:
        EntityID create_entity();
        OptionalReference<Entity> get_entity(EntityID eid);
        inline auto get_entities() {
            std::vector<std::pair<EntityID, Entity>> ents;
            for (auto &[k, v] : entities) ents.push_back({k, v});

            return ents;
        }
        ECS_T inline auto get_comp_id();
        ECS_T inline void register_component();
        ECS_T inline const T &add_component(EntityID id);
        ECS_T inline const T &add_component(EntityID id, T *data);
        ECS_T inline const T &get_component(EntityID id);
        ECS_T inline void modify_component(EntityID id, std::function<void(T &)> &&func);

        ECS_T SystemID add_system();
        ECS_T T *get_system(SystemID id);

        void update_entity(EntityID id);
        void update();

        ~ECS();

      private:
        TypeMap comps_map, sys_map;
        std::map<EntityID, std::set<ComponentID>> entities;
        std::map<ComponentID, SortedVectorUnique<EntityComponentWrapper>> components;
        std::unordered_map<SystemID, SystemBase *> systems;
    };

    class SystemBase {
      protected:
        std::set<ComponentID> family;
        template <typename... FAMILY> void set_component_family(ECS *e) { family = {e->get_comp_id<FAMILY>()...}; }
        bool entity_compatible(const Entity &e) const {
            return std::includes(e.begin(), e.end(), family.begin(), family.end());
        }

      public:
        virtual void update_entity(EntityID id) = 0;
        virtual void update()                   = 0;

        virtual ~SystemBase() = default;
    };
} // namespace eng
