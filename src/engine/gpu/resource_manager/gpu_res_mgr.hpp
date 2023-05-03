#pragma once

#include <concepts>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

#include <engine/types/sorted_vec.hpp>
#include <engine/types/idresource.hpp>
#include <engine/gpu/buffers/buffer.hpp>
#include <engine/gpu/texture/texture.hpp>

namespace eng {
    template <typename Resource>
    concept GpuResource = std::derived_from<Resource, IdResource<Resource>>
                          and std::default_initializable<Resource>;

    class GpuResMgr {
      public:
        template <GpuResource Resource> Resource &create_resource(Resource &&rsc) {
            return *static_cast<Resource *>(
                _get_storage<Resource>().insert(new Resource{std::move(rsc)}));
        }

        template <typename Resource> Resource &get_resource(Handle<Resource>);

      private:
        using _sort_func_t
            = decltype([](const IdWrapper *a, const IdWrapper *b) { return a->id < b->id; });
        using _storage_t = SortedVector<IdWrapper *, _sort_func_t>;

        std::unordered_map<std::type_index, _storage_t> _containers;

        template <typename Resource> _storage_t &_get_storage() {
            auto ti = std::type_index{typeid(Resource)};

            if (_containers.contains(ti) == false) { _containers[ti]; }

            return _containers.at(ti);
        }
    };

    template <typename Resource> Resource &GpuResMgr::get_resource(Handle<Resource> handle) {
        auto &storage = _get_storage<Resource>();
        auto p_data   = storage.try_find(handle);

        assert(p_data != nullptr && "Bad handle");

        return *static_cast<Resource *>(p_data);
    }

} // namespace eng