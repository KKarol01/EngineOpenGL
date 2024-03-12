#pragma once

#include <compare>
#include <functional>
#include <algorithm>

#include "sorted_vec.hpp"

namespace eng {

    template <typename T> class IDAllocator;
    template <typename T> class IDData {
      public:
        T &operator->();
        T &operator*();

        typename IDAllocator<T>::ID id;

      private:
        IDData(IDAllocator<T> *parent, typename IDAllocator<T>::ID id) : _storage{parent}, id{id} {}

        IDAllocator<T> *_storage{nullptr};
        friend class IDAllocator<T>;
    };

    template <typename T> class IDAllocator {
      public:
        typedef uint32_t ID;

        IDData<T> insert(const T &t) {
            auto &dt = storage.insert(DataWrapper{t});
            return {this, dt.id};
        }
        IDData<T> insert(T &&t) {
            auto &dt = storage.insert(DataWrapper{std::move(t)});
            return {this, dt.id};
        }
        template <typename... ARGS> IDData<T> emplace(ARGS &&...args) {
            auto &dt = storage.emplace(std::forward<ARGS>(args)...);
            return {this, dt.id};
        }

        T &get(ID id) { return storage.find(id)->data; }
        const T &get(ID id) const { return storage.find(id)->data; }

        T& find(auto val, auto cb) {
            return storage.find(val, cb);
        }

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
            static inline ID gid{1u};
        };

        eng::SortedVectorUnique<DataWrapper> storage;
    };

    template <typename T> inline T &IDData<T>::operator->() { return _storage->get(id); }
    template <typename T> inline T &IDData<T>::operator*() { return _storage->get(id); }
} // namespace eng
