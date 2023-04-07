#pragma once

#include <compare>
#include <functional>
#include <algorithm>

#include "sorted_vec.hpp"

namespace eng {
    template <typename T> class IDAllocator;

    template <typename T> struct SharedResource {

      public:
        SharedResource() = default;

        SharedResource(const SharedResource &dp) noexcept { *this = dp; }
        SharedResource(SharedResource &&dp) noexcept { *this = std::move(dp); }
        SharedResource &operator=(const SharedResource &dp) noexcept {
            printf("Incrementing: %i\n", ++gg);
            id    = dp.id;
            this_ = dp.this_;
            this_->on_proxy_add(this);
            return *this;
        }
        SharedResource &operator=(SharedResource &&dp) noexcept {
            id       = dp.id;
            this_    = dp.this_;
            dp.this_ = nullptr;
            return *this;
        }

        ~SharedResource() {
            if(this_==nullptr){return;}
            printf("Decrementing: %i\n", --gg);
            if(gg < 0) {
                int x = 1;
            }
            if (this_ != nullptr) { 
                this_->on_proxy_delete(this); 
            }
        }

      private:
        typedef uint32_t ID;

        SharedResource(ID id, IDAllocator<T> *this_) : id{id}, this_{this_} {}

      public:
        T *operator->() { return &this_->get(id); }
        const T *operator->() const { return &this_->get(id); }
        operator ID() const { return id; }

        ID id{0u};
        static inline int gg{0u};
      private:
        IDAllocator<T> *this_{nullptr};

        friend class IDAllocator<T>;
    };

    template <typename T> class IDAllocator {
      public:
        typedef uint32_t ID;

        SharedResource<T> insert(const T &t) {
            auto &dt = storage.insert(DataWrapper{t});
            return SharedResource{dt.id, this};
        }
        SharedResource<T> insert(T &&t) {
            auto &dt = storage.insert(DataWrapper{std::move(t)});
            return SharedResource{dt.id, this};
        }
        template <typename... ARGS> SharedResource<T> emplace(ARGS &&...args) {
            auto &dt = storage.emplace(std::forward<ARGS>(args)...);
            return SharedResource{dt.id, this};
        }

        T &get(ID id) { return storage.find(id)->data; }
        const T &get(ID id) const { return storage.find(id)->data; }

        void for_each(std::function<void(T &)> f) {
            std::for_each(storage.begin(), storage.end(), [&f](DataWrapper &dw) { f(dw.data); });
        }
        void for_each(std::function<void(const T &)> f) const {
            std::for_each(storage.begin(), storage.end(), [&f](const DataWrapper &dw) { f(dw.data); });
        }

      private:
        void on_proxy_add(SharedResource<T> *p) { ++storage.find(p->id)->ref_count; }
        void on_proxy_delete(SharedResource<T> *p) {
            auto pdata = storage.find(p->id);
            --pdata->ref_count;

            if (pdata->ref_count == 0u) { printf("Deleting\n"); }
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
            uint32_t ref_count{1u};
            T data;
            static inline ID gid{1u};
        };

        eng::SortedVectorUnique<DataWrapper> storage;

        friend struct SharedResource<T>;
    };
} // namespace eng
