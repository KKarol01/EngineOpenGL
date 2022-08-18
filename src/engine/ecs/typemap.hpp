#pragma once

#include <cstdint>
#include <memory>
#include <map>
#include <typeindex>
#include <string>
#include <stdexcept>

class TypeMap {
    using ID = std::uint32_t;
    ID gid{0u};
    std::map<std::type_index, ID> ids;

  public:
    using unique_ptr = std::unique_ptr<void, void (*)(const void *)>;

    template <typename T> void add_type() { ids[typeid(T)] = gid++; }
    template <typename T> ID get_type_id() {
        using namespace std::string_literals;

        auto &tid = typeid(T);

        if (ids.contains(tid) == false) throw std::runtime_error{"Type is not registered: "s + tid.name()};

        return ids.at(tid);
    }
    template <typename T> static auto unique_ptr_deleter() {
        return [](const void *data) { delete static_cast<const T *>(data); };
    }
    template <typename T> static unique_ptr get_unique_ptr(T *t) {
        return unique_ptr{t, unique_ptr_deleter<T>()};
    }
};
