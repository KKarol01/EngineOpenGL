#pragma once

#include "../types/types.hpp"
#include <compare>
#include <functional>
#include <algorithm>

template <typename T> class IDAllocator {
  public:
    typedef uint32_t ID;

    ID insert(const T &t) {
        auto &dt = storage.insert(DataWrapper{t});
        return dt.id;
    }
    ID insert(T &&t) {
        auto &dt = storage.insert(DataWrapper{std::move(t)});
        return dt.id;
    }
    template <typename... ARGS> ID emplace(ARGS &&...args) {
        auto &dt = storage.emplace(std::forward<ARGS>(args)...);
        return dt.id;
    }
    T &get(ID id) { return storage.find(id)->data; }
    const T &get(ID id) const { return storage.find(id)->data; }

    T &operator[](ID id) { return get(id); }
    const T &operator[](ID id) const { return get(id); }

    ID operator[](T &t) { return insert(std::move(t)); }
    ID operator[](T &&t) { return insert(std::move(t)); }

    void for_each(std::function<void(T &)> f) {
        std::for_each(storage.begin(), storage.end(), [&f](DataWrapper &dw) { f(dw.data); });
    }
    void for_each(std::function<void(const T &)> f) const {
        std::for_each(storage.begin(), storage.end(), [&f](const DataWrapper &dw) { f(dw.data); });
    }

  private:
    struct DataWrapper {
        DataWrapper() = default;
        DataWrapper(const T &t) : data{t} {}
        DataWrapper(T &&t) : data{std::move(t)} {}
        template <typename... ARGS> DataWrapper(ARGS &&...args) { data = T{std::forward<ARGS>(args)...}; }

        auto operator<=>(const DataWrapper &other) const { return id <=> other.id; }
        auto operator==(const DataWrapper &other) const { return id == other.id; }
        auto operator<=>(ID other) const { return id <=> other; }
        auto operator==(ID other) const { return id == other; }

        ID id{gid++};
        T data;
        static inline ID gid{0u};
    };

    eng::SortedVectorUnique<DataWrapper> storage;
};